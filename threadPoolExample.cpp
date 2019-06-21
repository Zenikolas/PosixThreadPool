#include <iostream>
#include "ThreadPool.hpp"

class CalculateFibonacci {
public:
    CalculateFibonacci(size_t n) : m_n(n), m_result(0) {}

    void run()
    {
        size_t a = 0, b = 1;
        if( m_n == 0) {
            m_result = 0;
            return;
        } else if (m_n == 1) {
            m_result = 1;
            return;
        }

        for (size_t i = 2; i <= m_n; i++) {
            m_result = a + b;
            a = b;
            b = m_result;
        }
    }

    size_t get() const { return m_result; }

    size_t m_n;
    size_t m_result;
};

int main()
{
    const size_t N = 200;
    std::vector<CalculateFibonacci> numbers;
    numbers.reserve(N);
    for (size_t i = 0; i < N; ++i) {
        numbers.push_back(CalculateFibonacci(i));
    }

    threadUtils::ThreadPool threadPool(2);

    for (size_t i = 0; i < N; ++i) {
        threadPool.Enqueue(numbers[i]);
    }

    std::cout << "Jobs for ThreadPool enqueued" << std::endl;

    threadPool.StopBlocking();

    std::cout << "All jobs are done!" << std::endl;

    for (size_t i = 0; i < N; ++i) {
        std::cout << "Fib[" << i << "]: " << numbers[i].get() << std::endl;
    }
}