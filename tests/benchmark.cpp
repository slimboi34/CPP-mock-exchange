#include "MatchingEngine.h"
#include <benchmark/benchmark.h>
#include <random>

using namespace exchange;

static void BM_AddLimitOrder_NoMatch(benchmark::State &state) {
  MatchingEngine engine;
  OrderId id = 1;
  for (auto _ : state) {
    state.PauseTiming();
    Price p = 100 + (id % 100);
    state.ResumeTiming();

    engine.add_limit_order(id++, Side::Buy, p, 10);
  }
}
BENCHMARK(BM_AddLimitOrder_NoMatch);

static void BM_MatchLimitOrder(benchmark::State &state) {
  MatchingEngine engine;
  // Pre-fill book with asks
  for (int i = 0; i < 10000; ++i) {
    engine.add_limit_order(i + 1, Side::Sell, 100 + (i % 100), 10);
  }

  OrderId taker_id = 100000;
  for (auto _ : state) {
    state.PauseTiming();
    // we add a taker buy that matches the best ask (which is 100)
    state.ResumeTiming();

    engine.add_limit_order(taker_id++, Side::Buy, 100, 10);

    state.PauseTiming();
    // add more liquidity so we don't run out
    engine.add_limit_order(taker_id++, Side::Sell, 100, 10);
    state.ResumeTiming();
  }
}
BENCHMARK(BM_MatchLimitOrder);

static void BM_MarketOrderMatch(benchmark::State &state) {
  MatchingEngine engine;
  // Pre-fill book with asks
  for (int i = 0; i < 10000; ++i) {
    engine.add_limit_order(i + 1, Side::Sell, 100 + (i % 10), 10);
  }

  OrderId taker_id = 100000;
  for (auto _ : state) {
    engine.add_market_order(taker_id++, Side::Buy, 10);

    state.PauseTiming();
    // replace liquidity
    engine.add_limit_order(taker_id++, Side::Sell, 105, 10);
    state.ResumeTiming();
  }
}
BENCHMARK(BM_MarketOrderMatch);

static void BM_OrderCancellation(benchmark::State &state) {
  MatchingEngine engine;
  OrderId id = 1;
  for (auto _ : state) {
    state.PauseTiming();
    engine.add_limit_order(id, Side::Buy, 100, 10);
    state.ResumeTiming();

    engine.cancel_order(id);

    state.PauseTiming();
    id++;
    state.ResumeTiming();
  }
}
BENCHMARK(BM_OrderCancellation);

BENCHMARK_MAIN();
