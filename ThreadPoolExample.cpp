#include <iostream>
#include <benchmark/benchmark.h>
#include "ThreadPool.hpp"

size_t func(size_t n)
{
    size_t ret = 0;
    for (size_t i = 0; i < 1'000'000; ++i) {
        benchmark::DoNotOptimize(ret += i * n);
    }

    return ret;
};

static void BM_ThreadPool(benchmark::State& state)
{
    const size_t size = state.range(0);
    const unsigned int Nsize = state.range(1);
    std::vector< std::future<size_t> > results;
    results.reserve(size);
    threadUtils::ThreadPool thPool(Nsize);

    for (auto _ : state) {
        for (size_t i = 0; i < size; ++i) {
            results.emplace_back( thPool.addTask(func, i * 200) );
        }

        for (size_t i = 0; i < size; ++i) {
            results[i].wait();
        }
    }
}

BENCHMARK(BM_ThreadPool)->Unit(benchmark::kMillisecond)
    ->Args({1'000, 1})
    ->Args({5'000, 1})
    ->Args({10'000, 1})
    ->Args({40'000, 1})
    ->Args({1'000, 2})
    ->Args({5'000, 2})
    ->Args({10'000, 2})
    ->Args({40'000, 2})
    ->Args({1'000, 4})
    ->Args({5'000, 4})
    ->Args({10'000, 4})
    ->Args({40'000, 4});

static void BM_OneThread(benchmark::State& state)
{
    const size_t size = state.range(0);
    std::vector< std::future<size_t> > results;
    results.reserve(size);

    for (auto _ : state) {
        for (size_t i = 0; i < size; ++i) {
            results.emplace_back( std::async(std::launch::deferred, func, i * 200) );
        }

        for (size_t i = 0; i < size; ++i) {
            results[i].wait();
        }
    }
}

BENCHMARK(BM_OneThread)->Unit(benchmark::kMillisecond)
        ->Arg(1'000)
        ->Arg(5'000)
        ->Arg(10'000)
        ->Arg(40'000);

BENCHMARK_MAIN();