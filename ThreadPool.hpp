#ifndef TEST_FUNCTION_THREADPOOL_HPP
#define TEST_FUNCTION_THREADPOOL_HPP

#include <pthread.h>
#include <vector>
#include <queue>

namespace threadUtils {

void *startThread(void *context);

class ThreadPool {
public:
    enum Priority {
        low,
        normal,
        high
    };

    ThreadPool(size_t);

    bool Init();

    template<class T>
    bool Enqueue(T& task, Priority priority = normal);

    void StopIfNoTasks();

    void Stop();

    ~ThreadPool();

private:
    friend void *startThread(void *context);

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
    void enqueuePrioritizedTask(T& task, Priority priority);

    bool isTaskWaiting() const {
        return !m_lowTasks.empty() || !m_normalTasks.empty() ||
               !m_highTasks.empty();
    }

    size_t m_workersCount;
    unsigned long int m_highTaskResolved;
    std::queue<TaskBase *> m_highTasks;
    std::queue<TaskBase *> m_normalTasks;
    std::queue<TaskBase *> m_lowTasks;
    std::vector<pthread_t> m_workers;

    mutable pthread_mutex_t m_mut;
    pthread_cond_t m_cv;

    volatile bool m_stop;
};

template<class T>
bool ThreadPool::Enqueue(T& task, Priority priority) {
    pthread_mutex_lock(&m_mut);
    if (m_stop) {
        pthread_mutex_unlock(&m_mut);
        return false;
    }

    enqueuePrioritizedTask(task, priority);

    pthread_mutex_unlock(&m_mut);

    pthread_cond_signal(&m_cv);

    return true;
}

template<class T>
void ThreadPool::enqueuePrioritizedTask(T& task,
                                        threadUtils::ThreadPool::Priority priority) {
    if (priority == low) {
        m_lowTasks.push(new TaskT<T>(task));
        // todo: consider using nothrow version of new
    } else if (priority == normal) {
        m_normalTasks.push(new TaskT<T>(task));
    } else {
        m_highTasks.push(new TaskT<T>(task));
    }
}

}

#endif //TEST_FUNCTION_THREADPOOL_HPP
