#include <iostream>
#include <assert.h>
#include <algorithm>
#include <stdio.h>
#include "ThreadPool.hpp"

namespace threadUtils {

void *startThread(void *context) {
    ThreadPool *poolPtr = static_cast<ThreadPool *> (context);
    poolPtr->threadFunc();
    return NULL;
}

ThreadPool::ThreadPool(size_t numTasks, size_t queueTaskSize) : m_workersCount(numTasks),
                                                                m_queueMaxSize(queueTaskSize),
                                                                m_highTaskResolved(0),
                                                                m_stop(false) {
    if (pthread_mutex_init(&m_mut, NULL)) {
        throw std::runtime_error("Failed to init pthread_mutex");
    }

    if (pthread_cond_init(&m_cv, NULL)) {
        throw std::runtime_error("Failed to init pthread_cond");
    }

    m_workers.reserve(m_workersCount);

    for (size_t i = 0; i < m_workersCount; ++i) {
        pthread_t tid = 0;
        int err = pthread_create(&tid, NULL, startThread, this);
        if (err) {
            char buffer[64];
            sprintf(buffer, "Failed to created thread, error code : %d", err);
            throw std::runtime_error(buffer);
        }

        m_workers.push_back(tid);
    }
}

void ThreadPool::StopIfNoTasks() {
    while (isTaskWaiting()) {
        pthread_yield();
    }

    Stop();

}

void ThreadPool::Stop() {
    pthread_mutex_lock(&m_mut);
    if (m_stop) {
        pthread_mutex_unlock(&m_mut);
        return;
    }
    m_stop = true;
    pthread_cond_broadcast(&m_cv);
    pthread_mutex_unlock(&m_mut);

    for (size_t i = 0; i < m_workers.size(); ++i) {
        void *ret = NULL;
        pthread_join(m_workers[i], &ret);
    }

    deleteTasks(m_lowTasks);
    deleteTasks(m_normalTasks);
    deleteTasks(m_highTasks);
}

void ThreadPool::threadFunc() {
    while (true) {
        pthread_mutex_lock(&m_mut);
        while (!isTaskWaiting() && !m_stop) {
            pthread_cond_wait(&m_cv, &m_mut);
        }

        if (m_stop) {
            pthread_mutex_unlock(&m_mut);
            return;
        }

        TaskBase *taskPtr = NULL;
        if (m_highTasks.empty() && m_normalTasks.empty()) {
            taskPtr = m_lowTasks.front();
            m_lowTasks.pop();
        }

        if (!taskPtr && !m_highTasks.empty()) {
            if (m_normalTasks.empty() || m_highTaskResolved < 3) {
                taskPtr = m_highTasks.front();
                m_highTasks.pop();
                ++m_highTaskResolved;
            }
        }

        if (!taskPtr && !m_normalTasks.empty()) {
            taskPtr = m_normalTasks.front();
            m_normalTasks.pop();
            if (m_highTaskResolved < 3) {
                m_highTaskResolved = 0;
            } else {
                m_highTaskResolved -= 3;
            }
        }

        pthread_mutex_unlock(&m_mut);
        assert(taskPtr);

        taskPtr->run();

        delete taskPtr;
    }
}

void ThreadPool::deleteTasks(std::queue<TaskBase *> &queue) {
    while (!queue.empty()) {
        TaskBase *ptr = queue.front();
        queue.pop();
        delete ptr;
    }
}

ThreadPool::~ThreadPool() {
    Stop();
}

}