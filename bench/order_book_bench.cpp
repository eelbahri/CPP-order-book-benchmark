#include <benchmark/benchmark.h>

#include <algorithm>
#include <cstdint>
#include <random>
#include <vector>

#include "orderbook/order_book_v1.h"
#include "orderbook/order_book_v2.h"
#include "orderbook/order_book_v3.h"

namespace orderbook {
namespace {

template <typename Book>
Book make_book(std::int64_t orders, std::int64_t levels) {
    Book book;
    for (std::int64_t i = 0; i < orders; ++i) {
        book.add({.id = static_cast<OrderId>(i),
                  .price = i % levels,
                  .quantity = 10,
                  .side = Side::Buy});
    }
    return book;
}

// Adds `orders` buy orders spread round-robin over `levels` price levels.
// Each benchmark iteration builds a fresh book, so the timing includes the
// allocations AND the destruction of the book (freeing all that memory).
template <typename Book>
void BM_AddOrders(benchmark::State& state) {
    const auto orders = static_cast<std::int64_t>(state.range(0));
    const auto levels = static_cast<std::int64_t>(state.range(1));

    for (auto _ : state) {
        Book book = make_book<Book>(orders, levels);
        // Prevents the optimizer from deleting the loop as "unused work".
        benchmark::DoNotOptimize(book);
    }
    // Reports throughput (orders/second) instead of only total time.
    state.SetItemsProcessed(state.iterations() * orders);
}

// Case A: 1,000,000 orders over 10 price levels.
BENCHMARK_TEMPLATE(BM_AddOrders, OrderBookV1)->Args({1'000'000, 10})->Unit(benchmark::kMillisecond);
// Case B : 100,000 orders over 100,000 price levels. (1 order per level)
BENCHMARK_TEMPLATE(BM_AddOrders, OrderBookV1)
    ->Args({100'000, 100'000})
    ->Unit(benchmark::kMillisecond);
// Case C : 1,000,000 orders over 100,000 price levels. (10 orders per level)
BENCHMARK_TEMPLATE(BM_AddOrders, OrderBookV1)
    ->Args({1'000'000, 100'000})
    ->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(BM_AddOrders, OrderBookV2)->Args({1'000'000, 10})->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_AddOrders, OrderBookV2)
    ->Args({100'000, 100'000})
    ->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_AddOrders, OrderBookV2)
    ->Args({1'000'000, 100'000})
    ->Unit(benchmark::kMillisecond);

template <typename Book>
void BM_BestBid(benchmark::State& state) {
    const auto orders = static_cast<std::int64_t>(state.range(0));
    const auto levels = static_cast<std::int64_t>(state.range(1));

    Book book = make_book<Book>(orders, levels);
    for (auto _ : state) {
        benchmark::DoNotOptimize(book.best_bid());
    }
    state.SetItemsProcessed(state.iterations());
}

template <typename Book>
void BM_OrderCount(benchmark::State& state) {
    const auto orders = static_cast<std::int64_t>(state.range(0));
    const auto levels = static_cast<std::int64_t>(state.range(1));

    Book book = make_book<Book>(orders, levels);
    for (auto _ : state) {
        benchmark::DoNotOptimize(book.order_count());
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_TEMPLATE(BM_BestBid, OrderBookV1)->Args({1'000'000, 10})->Unit(benchmark::kNanosecond);
BENCHMARK_TEMPLATE(BM_BestBid, OrderBookV2)->Args({1'000'000, 10})->Unit(benchmark::kNanosecond);
BENCHMARK_TEMPLATE(BM_OrderCount, OrderBookV1)->Args({1'000'000, 10})->Unit(benchmark::kNanosecond);

BENCHMARK_TEMPLATE(BM_BestBid, OrderBookV1)->Args({100'000, 100'000})->Unit(benchmark::kNanosecond);
BENCHMARK_TEMPLATE(BM_BestBid, OrderBookV2)->Args({100'000, 100'000})->Unit(benchmark::kNanosecond);
BENCHMARK_TEMPLATE(BM_OrderCount, OrderBookV1)
    ->Args({100'000, 100'000})
    ->Unit(benchmark::kNanosecond);

// A more realistic flow of orders than make_book's "ascending prices, buys only":
//  - both sides, 50/50,
//  - prices concentrated near the top of the book: the distance from the
//    mid price follows a geometric distribution (most orders within a few
//    ticks, a long tail of rare deep orders, capped at 1000 ticks).
// Generated ONCE, before timing: we want to measure the book, not the
// random number generator. Fixed seed: every run sees the same orders.
std::vector<Order> realistic_orders(std::int64_t count) {
    constexpr Price kMid = 10'000;
    std::mt19937_64 rng{42};
    std::bernoulli_distribution is_buy{0.5};
    std::geometric_distribution<Price> distance{0.1};  // mean ~9 ticks
    std::uniform_int_distribution<Quantity> quantity{1, 100};

    std::vector<Order> orders;
    orders.reserve(static_cast<std::size_t>(count));
    for (std::int64_t i = 0; i < count; ++i) {
        const Price d = std::min<Price>(distance(rng), 1000);
        const bool buy = is_buy(rng);
        orders.push_back({.id = static_cast<OrderId>(i),
                          .price = buy ? kMid - 1 - d : kMid + 1 + d,
                          .quantity = quantity(rng),
                          .side = buy ? Side::Buy : Side::Sell});
    }
    return orders;
}

template <typename Book>
void BM_AddRealistic(benchmark::State& state) {
    const std::vector<Order> orders = realistic_orders(state.range(0));
    for (auto _ : state) {
        Book book;
        for (const Order& order : orders) {
            book.add(order);
        }
        benchmark::DoNotOptimize(book);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}

BENCHMARK_TEMPLATE(BM_AddRealistic, OrderBookV1)->Arg(1'000'000)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_AddRealistic, OrderBookV2)->Arg(1'000'000)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_AddRealistic, OrderBookV3)->Arg(1'000'000)->Unit(benchmark::kMillisecond);

}  // namespace
}  // namespace orderbook

BENCHMARK_MAIN();
