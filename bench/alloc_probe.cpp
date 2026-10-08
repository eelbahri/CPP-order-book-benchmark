// Counts heap allocations per operation by replacing the global operator new.
// Answers "what does a new price level cost?" without guessing.

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <new>

#include "orderbook/order_book_v1.h"
#include "orderbook/order_book_v2.h"
#include "orderbook/order_book_v3.h"

namespace {
std::size_t g_allocations = 0;
std::size_t g_bytes = 0;
}  // namespace

void* operator new(std::size_t size) {
    ++g_allocations;
    g_bytes += size;
    if (void* p = std::malloc(size)) {
        return p;
    }
    throw std::bad_alloc{};
}
void operator delete(void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }

namespace {

using namespace orderbook;

struct Delta {
    double allocations;
    double bytes;
};

// Average allocations and bytes per add() over `count` adds.
template <typename Book, typename MakeOrder>
Delta measure(Book& book, int count, MakeOrder make_order) {
    const std::size_t a0 = g_allocations;
    const std::size_t b0 = g_bytes;
    for (int i = 0; i < count; ++i) {
        book.add(make_order(i));
    }
    return {static_cast<double>(g_allocations - a0) / count,
            static_cast<double>(g_bytes - b0) / count};
}

template <typename Book>
void report(const char* name) {
    constexpr int kLevels = 10'000;
    Book book;
    const Delta new_level = measure(book, kLevels, [](int i) {
        return Order{.id = 0, .price = i, .quantity = 1, .side = Side::Buy};
    });
    const Delta same_level = measure(book, kLevels, [](int i) {
        return Order{.id = 0, .price = i % kLevels, .quantity = 1, .side = Side::Buy};
    });
    std::printf(
        "%-12s new level: %6.3f allocs %7.1f bytes | existing level: %6.3f allocs %5.1f bytes\n",
        name, new_level.allocations, new_level.bytes, same_level.allocations, same_level.bytes);
}

}  // namespace

int main() {
    std::printf("Average heap allocations per add() (10,000 adds each)\n");
    report<OrderBookV1>("OrderBookV1");
    report<OrderBookV2>("OrderBookV2");
    report<OrderBookV3>("OrderBookV3");
    return 0;
}
