#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <vector>

#include "orderbook/order.h"

namespace orderbook {

// V3: no tree, no deque. Data-oriented layout:
//  - Price levels live in a sorted std::vector, best price at the BACK
//    (bids ascending, asks descending): best price, and inserting/removing
//    levels near the top of the book, only touch the end of the vector.
//  - All orders of the book live in ONE std::vector (a pool). Orders at the
//    same price form a FIFO linked list through indices into that pool.
// A new price level therefore costs zero heap allocations (amortized).
class OrderBookV3 {
public:
    void add(const Order& order);

    std::optional<Price> best_bid() const;
    std::optional<Quantity> best_bid_quantity() const;
    std::optional<Price> best_ask() const;
    std::optional<Quantity> best_ask_quantity() const;

    std::size_t order_count() const;

    // Position of an order in orders_. An index, not a pointer: it stays
    // valid when the vector reallocates, and it is 4 bytes instead of 8.
    using OrderIndex = std::uint32_t;

    // "No order" marker, like null. The max value can never be a real index
    // because the pool would need 4 billion orders first.
    static constexpr OrderIndex kNone = std::numeric_limits<OrderIndex>::max();

    // Hot data: read on every level search. 24 bytes (see sizeof exercise).
    struct Level {
        Price price;
        Quantity total_quantity;  // sum of the orders' quantities at this price
        OrderIndex head;          // oldest order: matched first (FIFO)
        OrderIndex tail;          // newest order: where new orders are appended
    };

    // Cold data: only read when matching walks a level's FIFO.
    // 16 bytes - 4 orders fit in a cache line
    struct OrderSlot {
        OrderId id;         // 8 bytes
        Quantity quantity;  // 4 bytes
        OrderIndex next;    // 4 bytes - next order at the same price, or kNone
    };

private:
    std::vector<Level> bids_;  // ascending:  best (highest) bid at back()
    std::vector<Level> asks_;  // descending: best (lowest) ask at back()
    std::vector<OrderSlot> orders_;
};

}  // namespace orderbook
