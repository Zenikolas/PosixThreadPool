#include "ThreadPool.hpp"

namespace threadUtils
{

ThreadPool::ThreadPool(size_t nth) : m_stop(false)
{
    m_workers.reserve(nth);

    for (size_t i = 0; i < nth; ++i)
    {
        m_workers.emplace_back([this]()
                               { threadFunc(); });
    }
}

void ThreadPool::threadFunc()
{
    while (true)
    {
        std::function<void()> task;
        std::unique_lock<std::mutex> lc(m_mut);
        m_cv.wait(lc, [this]()
        { return !m_tasks.empty() || m_stop.load(std::memory_order_acquire); });

        if (m_stop && m_tasks.empty())
        {
            return;
        }

        task = std::move(m_tasks.front());
        m_tasks.pop();

        if (task)
        {
            task();
        }
    }
}

ThreadPool::~ThreadPool()
{
    m_stop.store(true, std::memory_order_release);
    m_cv.notify_all();

    for (auto &w : m_workers)
    {
        w.join();
    }
}

}