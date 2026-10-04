#include <benchmark/benchmark.h>

#include <cstdint>

#include "orderbook/order_book_v1.h"
#include "orderbook/order_book_v2.h"

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

}  // namespace
}  // namespace orderbook

BENCHMARK_MAIN();
