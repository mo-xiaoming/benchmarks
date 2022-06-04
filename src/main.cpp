#include <benchmark/benchmark.h>
#include <boost/unordered_map.hpp>
#include <boost/container/flat_map.hpp>
#include <sparsehash/dense_hash_map>

#include <iostream>
#include <algorithm>
#include <iterator>
#include <map>
#include <unordered_map>
#include <vector>
#include <random>

namespace {
  struct Value { char b[40]; };

  inline constexpr auto MaxTypes = 259U;
  inline constexpr auto NrAccess = 10'000U;

  auto gen_random_int(int min, int max) {
    static std::default_random_engine e{};
    static std::uniform_int_distribution<> d{min, max};
    return d(e);
  }

  auto init_map(std::size_t rows) {
    auto m = std::unordered_map<int, Value>();
    m.reserve(rows);
    std::generate_n(std::inserter(m, m.begin()), rows, [] {
        const auto key = gen_random_int(0, MaxTypes);
        const auto value = Value{};
        return std::pair(key, value);
    });
    return m;
  }

  auto init_access(std::size_t count) {
    auto v = std::vector<int>();
    v.reserve(count);
    std::generate_n(std::back_inserter(v), count, []{ return gen_random_int(0, MaxTypes); });
    return v;
  };

  template <typename Map>
  auto create_data_to_map(long init_map_size, std::size_t access_size) {
    const auto data = [init_map_size] {
      const auto init = init_map(static_cast<std::size_t>(init_map_size));
      Map d;
      d.insert(init.cbegin(), init.cend());
      return d;
    }();
    const auto access = init_access(access_size);
    return std::pair(data, access);
  }
}

static void bench_boost_unordered_map(benchmark::State& state) {
  auto [data, access] = create_data_to_map<boost::unordered_map<int, Value>>(state.range(0), NrAccess);
  for (auto _ : state) {
    for (auto i : access) {
      const auto it = data.find(i);
      benchmark::DoNotOptimize(&*it);
    }
  }
}
BENCHMARK(bench_boost_unordered_map)->Arg(1)->Arg(3)->Arg(5)->Arg(10)->Arg(20)->Arg(50)->Arg(100)->Arg(200)->Arg(250);

static void bench_std_unordered_map(benchmark::State& state) {
  auto [data, access] = create_data_to_map<std::unordered_map<int, Value>>(state.range(0), NrAccess);
  for (auto _ : state) {
    for (auto i : access) {
      const auto it = data.find(i);
      benchmark::DoNotOptimize(&*it);
    }
  }
}
BENCHMARK(bench_std_unordered_map)->Arg(1)->Arg(3)->Arg(5)->Arg(10)->Arg(20)->Arg(50)->Arg(100)->Arg(200)->Arg(250);

static void bench_std_map(benchmark::State& state) {
  auto [data, access] = create_data_to_map<std::map<int, Value>>(state.range(0), NrAccess);
  for (auto _ : state) {
    for (auto i : access) {
      const auto it = data.find(i);
      benchmark::DoNotOptimize(&*it);
    }
  }
}
BENCHMARK(bench_std_map)->Arg(1)->Arg(3)->Arg(5)->Arg(10)->Arg(20)->Arg(50)->Arg(100)->Arg(200)->Arg(250);

static void bench_boost_flat_map(benchmark::State& state) {
  auto [data, access] = create_data_to_map<boost::container::flat_map<int, Value>>(state.range(0), NrAccess);
  for (auto _ : state) {
    for (auto i : access) {
      const auto it = data.find(i);
      benchmark::DoNotOptimize(&*it);
    }
  }
}
BENCHMARK(bench_boost_flat_map)->Arg(1)->Arg(3)->Arg(5)->Arg(10)->Arg(20)->Arg(50)->Arg(100)->Arg(200)->Arg(250);

static void bench_dense_hash_map(benchmark::State& state) {
  auto [data, access] = create_data_to_map<boost::unordered_map<int, Value>>(state.range(0), NrAccess);
  google::dense_hash_map<int, Value> new_data(data.cbegin(), data.cend(), -1, static_cast<std::size_t>(state.range(0)));
  for (auto _ : state) {
    for (auto i : access) {
      const auto it = new_data.find(i);
      benchmark::DoNotOptimize(&*it);
    }
  }
}
BENCHMARK(bench_dense_hash_map)->Arg(1)->Arg(3)->Arg(5)->Arg(10)->Arg(20)->Arg(50)->Arg(100)->Arg(200)->Arg(250);

static void bench_sorted_vector(benchmark::State& state) {
  auto [data, access] = create_data_to_map<boost::container::flat_map<int, Value>>(state.range(0), NrAccess);
  std::vector<int> tags;
  std::vector<Value> values;
  for (const auto& [k, v] : data) {
    const auto it = std::lower_bound(tags.cbegin(), tags.cend(), k);
    if (it != tags.cend() && *it == k) {
      continue;
    }
    const auto dis = std::distance(tags.cbegin(), it);
    tags.insert(it, k);
    values.insert(values.cbegin() + dis, v);
  }
  for (auto _ : state) {
    for (auto i : access) {
      if (const auto it = std::lower_bound(tags.cbegin(), tags.cend(), i); *it == i) {
        benchmark::DoNotOptimize(*(values.cbegin() + (it-tags.cbegin())));
      }
    }
  }
}
BENCHMARK(bench_sorted_vector)->Arg(1)->Arg(3)->Arg(5)->Arg(10)->Arg(20)->Arg(50)->Arg(100)->Arg(200)->Arg(250);

BENCHMARK_MAIN();
