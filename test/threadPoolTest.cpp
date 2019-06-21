#include "ThreadPool.hpp"

#include <atomic>
#include <gtest/gtest.h>
#include <gmock/gmock.h>


class CallableMock {
public:

    MOCK_METHOD0(run, void());

};

std::atomic<bool> start{false};

class Waiter {
public:
    void run() { while(!start); }
};

TEST(ThreadPoolTest, smoke)
{
    threadUtils::ThreadPool threadPool(4);
    ASSERT_TRUE(threadPool.Init());

    CallableMock callable;
    const size_t N = 200;
    EXPECT_CALL(callable, run()).Times(N);

    for (size_t i = 0; i < N; ++i) {
        EXPECT_TRUE(threadPool.Enqueue(callable));
    }


    threadPool.StopIfNoTasks();

    ASSERT_FALSE(threadPool.Enqueue(callable));
}

TEST(ThreadPoolTest, ordering)
{
    using ::testing::InSequence;
    Waiter waiter;

    threadUtils::ThreadPool threadPool(1);
    ASSERT_TRUE(threadPool.Init());

    const size_t lowCallableSize = 6;
    const size_t normalCallableSize = 4;
    const size_t highCallableSize = 6;
    CallableMock lowCallable[lowCallableSize];
    CallableMock normalCallable[normalCallableSize];
    CallableMock highCallable[highCallableSize];

    {
        InSequence dummy;
        EXPECT_CALL(highCallable[0], run());
        EXPECT_CALL(highCallable[1], run());
        EXPECT_CALL(highCallable[2], run());
        EXPECT_CALL(normalCallable[0], run());
        EXPECT_CALL(highCallable[3], run());
        EXPECT_CALL(highCallable[4], run());
        EXPECT_CALL(highCallable[5], run());
        EXPECT_CALL(normalCallable[1], run());
        EXPECT_CALL(normalCallable[2], run());
        EXPECT_CALL(normalCallable[3], run());
        EXPECT_CALL(lowCallable[0], run());
        EXPECT_CALL(lowCallable[1], run());
        EXPECT_CALL(lowCallable[2], run());
        EXPECT_CALL(lowCallable[3], run());
        EXPECT_CALL(lowCallable[4], run());
        EXPECT_CALL(lowCallable[5], run());
    }


    threadPool.Enqueue(waiter);
    sleep(5);
    for (size_t i = 0; i < lowCallableSize; ++i) {
        EXPECT_TRUE(threadPool.Enqueue(lowCallable[i], threadUtils::ThreadPool::low));
    }

    for (size_t i = 0; i < normalCallableSize; ++i) {
        EXPECT_TRUE(threadPool.Enqueue(normalCallable[i], threadUtils::ThreadPool::normal));
    }

    for (size_t i = 0; i < highCallableSize; ++i) {
        EXPECT_TRUE(threadPool.Enqueue(highCallable[i], threadUtils::ThreadPool::high));
    }

    start = true;

    threadPool.StopIfNoTasks();
}
