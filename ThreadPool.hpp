#ifndef TEST_FUNCTION_THREADPOOL_HPP
#define TEST_FUNCTION_THREADPOOL_HPP

#include <vector>
#include <functional>
#include <future>
#include <queue>
#include <memory>

namespace threadUtils
{

class ThreadPool
{
public:
    ThreadPool(size_t);

    template<class F, class ... Args>
    std::future<typename std::result_of<F(Args...)>::type> addTask(F&&, Args&& ...);

    ~ThreadPool();
private:
    void threadFunc();

    std::queue<std::function<void()>> m_tasks;
    std::vector<std::thread> m_workers;

    std::mutex m_mut;
    std::condition_variable m_cv;

    std::atomic<bool> m_stop;
};

template<class F, class ... Args>
std::future<typename std::result_of<F(Args...)>::type> ThreadPool::addTask(F&& f, Args&&... args)
{
    using retType = typename std::result_of<F(Args ...)>::type;
    auto task = std::make_shared< std::packaged_task<retType()> >([f=f, args...] () { return f(std::move(args)...); });
    auto ret = task->get_future();

    {
        std::unique_lock<std::mutex> lc(m_mut);
        if (m_stop) {
            throw std::runtime_error("Trying to add new task to stopped ThreadPool");
        }

        m_tasks.emplace([task] () { (*task)(); });
    }

    m_cv.notify_one();

    return ret;
}

}

#endif //TEST_FUNCTION_THREADPOOL_HPP
