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

// Synthetic workload: buy orders only, prices cycling 0, 1, ..., levels-1.
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

// Realistic workload: both sides, prices concentrated near the top of the book.
// Distance from the mid price is geometric (mean ~9 ticks, capped at 1000).
// Generated once, outside the timed region, with a fixed seed.
std::vector<Order> realistic_orders(std::int64_t count) {
    constexpr Price kMid = 10'000;
    std::mt19937_64 rng{42};
    std::bernoulli_distribution is_buy{0.5};
    std::geometric_distribution<Price> distance{0.1};
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

// Builds a fresh book per iteration: timing includes allocation and teardown.
template <typename Book>
void BM_AddSynthetic(benchmark::State& state) {
    const auto orders = static_cast<std::int64_t>(state.range(0));
    const auto levels = static_cast<std::int64_t>(state.range(1));
    for (auto _ : state) {
        Book book = make_book<Book>(orders, levels);
        benchmark::DoNotOptimize(book);
    }
    state.SetItemsProcessed(state.iterations() * orders);
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

template <typename Book>
void BM_BestBid(benchmark::State& state) {
    const Book book = make_book<Book>(state.range(0), state.range(1));
    for (auto _ : state) {
        benchmark::DoNotOptimize(book.best_bid());
    }
}

template <typename Book>
void BM_OrderCount(benchmark::State& state) {
    const Book book = make_book<Book>(state.range(0), state.range(1));
    for (auto _ : state) {
        benchmark::DoNotOptimize(book.order_count());
    }
}

// Synthetic add() scenarios, {orders, levels}:
//   few levels / one new level per order / deep levels hit in rotation.
#define ADD_SYNTHETIC(Book)                   \
    BENCHMARK_TEMPLATE(BM_AddSynthetic, Book) \
        ->Args({1'000'000, 10})               \
        ->Args({100'000, 100'000})            \
        ->Args({1'000'000, 1'000})            \
        ->Unit(benchmark::kMillisecond)
ADD_SYNTHETIC(OrderBookV1);
ADD_SYNTHETIC(OrderBookV2);
ADD_SYNTHETIC(OrderBookV3);

BENCHMARK_TEMPLATE(BM_AddRealistic, OrderBookV1)->Arg(1'000'000)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_AddRealistic, OrderBookV2)->Arg(1'000'000)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_AddRealistic, OrderBookV3)->Arg(1'000'000)->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(BM_BestBid, OrderBookV1)->Args({1'000'000, 10})->Args({100'000, 100'000});
BENCHMARK_TEMPLATE(BM_BestBid, OrderBookV2)->Args({1'000'000, 10})->Args({100'000, 100'000});
BENCHMARK_TEMPLATE(BM_BestBid, OrderBookV3)->Args({1'000'000, 10})->Args({100'000, 100'000});

BENCHMARK_TEMPLATE(BM_OrderCount, OrderBookV1)->Args({1'000'000, 10})->Args({100'000, 100'000});
BENCHMARK_TEMPLATE(BM_OrderCount, OrderBookV3)->Args({1'000'000, 10})->Args({100'000, 100'000});

}  // namespace
}  // namespace orderbook

BENCHMARK_MAIN();
