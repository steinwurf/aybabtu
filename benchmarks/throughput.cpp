// Copyright (c) Steinwurf ApS 2016.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include <cstdint>
#include <vector>

#include <benchmark/benchmark.h>

#include <aybabtu/base64.hpp>

static void encode(benchmark::State& state, std::size_t size)
{
    state.counters["size"] = size;

    std::vector<uint8_t> data_in(size);
    std::generate(data_in.begin(), data_in.end(), rand);

    for (auto _ : state)
    {
        auto result = aybabtu::base64::encode(data_in.data(), data_in.size());
        benchmark::DoNotOptimize(result);
    }

    state.SetBytesProcessed(size * state.iterations());
}

static void decode(benchmark::State& state, std::size_t size)
{
    state.counters["size"] = size;

    std::vector<uint8_t> data_in(size);
    std::generate(data_in.begin(), data_in.end(), rand);
    auto result = aybabtu::base64::encode(data_in.data(), data_in.size());

    std::vector<uint8_t> data_out(size);

    for (auto _ : state)
    {
        aybabtu::base64::decode(data_out.data(), result);
        benchmark::DoNotOptimize(data_out);
    }

    state.SetBytesProcessed(size * state.iterations());
}

static void BenchmarkArguments(benchmark::internal::Benchmark* b)
{
    b->Unit(benchmark::kMillisecond);
    b->Repetitions(5);
}

BENCHMARK_CAPTURE(encode, 100, 100)->Apply(BenchmarkArguments);
BENCHMARK_CAPTURE(encode, 1000, 1000)->Apply(BenchmarkArguments);
BENCHMARK_CAPTURE(encode, 10000, 10000)->Apply(BenchmarkArguments);
BENCHMARK_CAPTURE(decode, 100, 100)->Apply(BenchmarkArguments);
BENCHMARK_CAPTURE(decode, 1000, 1000)->Apply(BenchmarkArguments);
BENCHMARK_CAPTURE(decode, 10000, 10000)->Apply(BenchmarkArguments);

BENCHMARK_MAIN();
