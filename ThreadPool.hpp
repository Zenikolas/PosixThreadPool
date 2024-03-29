#ifndef TEST_FUNCTION_THREADPOOL_HPP
#define TEST_FUNCTION_THREADPOOL_HPP

#include <pthread.h>
#include <vector>
#include <queue>
#include <iostream>

namespace threadUtils {

void *startThread(void *context);

class ThreadPool {
public:
    static const size_t MAX_TASK_SIZE = 1024;

    enum Priority {
        low,
        normal,
        high
    };

    ThreadPool(size_t numTasks, size_t queueTaskSize = MAX_TASK_SIZE);

    template<class T>
    bool Enqueue(T& task, Priority priority = normal);

    template<class T>
    bool EnqueueBlocking(T& task, Priority priority = normal);

    void Stop();

    void StopBlocking();

    ~ThreadPool();


private:
    ThreadPool(const ThreadPool& rhs);

    ThreadPool& operator=(const ThreadPool& rhs);

    friend void *startThread(void *context);

    void stopImpl();

    void joinWorkers();

    void stopUnlockJoin();

    void initConds();

    class TaskBase {
    public:
        virtual ~TaskBase() {}

        virtual void run() = 0;
    };

    template<class T>
    class TaskT : public TaskBase {
        T& m_task;
    public:
        TaskT(T& task) : m_task(task) {}

        virtual void run() { m_task.run(); }
    };

    void threadFunc();

    void deleteTasks(std::queue<TaskBase *>& queue);

    template<class T>
    bool enqueuePrioritizedTask(T& task, Priority priority);

    template<class T>
    bool enqueuePrioritizedTaskBlocking(T& task, Priority priority);

    bool isTaskWaiting() const {
        return !m_lowTasks.empty() || !m_normalTasks.empty() ||
               !m_highTasks.empty();
    }

    const size_t m_workersCount;
    const size_t m_queueMaxSize;
    unsigned long int m_highTaskResolved;
    std::queue<TaskBase *> m_highTasks;
    std::queue<TaskBase *> m_normalTasks;
    std::queue<TaskBase *> m_lowTasks;
    std::vector<pthread_t> m_workers;

    mutable pthread_mutex_t m_mut;
    pthread_cond_t m_cvNewTask;
    pthread_cond_t m_cvAllReady;
    pthread_cond_t m_cvLowEnrolled;
    pthread_cond_t m_cvNormalEnrolLed;
    pthread_cond_t m_cvHighEnrolled;

    bool m_stop;
};

template<class T>
bool ThreadPool::Enqueue(T& task, Priority priority) {
    pthread_mutex_lock(&m_mut);
    if (m_stop) {
        std::cerr << "Trying to add a new task after stop call" << std::endl;
        pthread_mutex_unlock(&m_mut);
        return false;
    }

    if (!enqueuePrioritizedTask(task, priority)) {
        pthread_mutex_unlock(&m_mut);
        return false;
    }

    pthread_cond_signal(&m_cvNewTask);
    pthread_mutex_unlock(&m_mut);

    return true;
}

template<class T>
bool ThreadPool::EnqueueBlocking(T& task, Priority priority) {
    pthread_mutex_lock(&m_mut);
    if (m_stop) {
        std::cerr << "Trying to add a new task after stop call" << std::endl;
        pthread_mutex_unlock(&m_mut);
        return false;
    }

    if (!enqueuePrioritizedTaskBlocking(task, priority)) {
        pthread_mutex_unlock(&m_mut);
        return false;
    }

    pthread_cond_signal(&m_cvNewTask);
    pthread_mutex_unlock(&m_mut);

    return true;
}

template<class T>
bool ThreadPool::enqueuePrioritizedTask(T& task,
                                        threadUtils::ThreadPool::Priority priority) {
    TaskBase *ptr = new(std::nothrow) TaskT<T>(task);
    if (!ptr) {
        std::cerr << "Failed to allocate memory for new task" << std::endl;
        return false;
    }

    if (priority == low) {
        if (m_queueMaxSize <= m_lowTasks.size()) {
            std::cerr << "Trying to exceed max size of low priority tasks in queue"
                      << std::endl;
            return false;
        }
        m_lowTasks.push(ptr);
    } else if (priority == normal) {
        if (m_queueMaxSize <= m_normalTasks.size()) {
            std::cerr << "Trying to exceed max size of normal priority tasks in queue"
                      << std::endl;
            return false;
        }
        m_normalTasks.push(ptr);
    } else {
        if (m_queueMaxSize <= m_highTasks.size()) {
            std::cerr << "Trying to exceed max size of high priority tasks in queue"
                      << std::endl;
            return false;
        }
        m_highTasks.push(ptr);
    }

    return true;
}

template<class T>
bool ThreadPool::enqueuePrioritizedTaskBlocking(T& task,
                                                threadUtils::ThreadPool::Priority priority) {
    TaskBase *ptr = new(std::nothrow) TaskT<T>(task);
    if (!ptr) {
        std::cerr << "Failed to allocate memory for new task" << std::endl;
        return false;
    }

    if (priority == low) {
        while (m_queueMaxSize <= m_lowTasks.size() && !m_stop) {
            pthread_cond_wait(&m_cvLowEnrolled, &m_mut);
        }
        if (!m_stop) {
            m_lowTasks.push(ptr);
        }
    } else if (priority == normal) {
        while (m_queueMaxSize <= m_normalTasks.size() && !m_stop) {
            pthread_cond_wait(&m_cvNormalEnrolLed, &m_mut);
        }
        if (!m_stop) {
            m_normalTasks.push(ptr);
        }
    } else {
        while (m_queueMaxSize <= m_highTasks.size() && !m_stop) {
            pthread_cond_wait(&m_cvHighEnrolled, &m_mut);
        }
        if (!m_stop) {
            m_highTasks.push(ptr);
        }
    }

    return true;
}

}

#endif //TEST_FUNCTION_THREADPOOL_HPP
