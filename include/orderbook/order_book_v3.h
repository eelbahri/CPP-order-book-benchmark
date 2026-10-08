#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <vector>

#include "orderbook/order.h"

namespace orderbook {

// V3: data-oriented layout, no tree, no per-level allocation.
//  - Price levels are kept in a sorted std::vector with the best price at the
//    back (bids ascending, asks descending), so the top of the book, where
//    most activity happens, is at the cheap end of the vector.
//  - All orders live in a single pool (std::vector<OrderSlot>). Orders at the
//    same price form a FIFO singly linked list of indices into the pool.
// Trade-off: finding a level is a linear scan from the best price, O(distance
// from the top of the book). Fast near the top, slow for deep levels.
class OrderBookV3 {
public:
    void add(const Order& order);

    std::optional<Price> best_bid() const;
    std::optional<Quantity> best_bid_quantity() const;
    std::optional<Price> best_ask() const;
    std::optional<Quantity> best_ask_quantity() const;

    std::size_t order_count() const;

    // Indices rather than pointers: they survive pool reallocation and take
    // 4 bytes instead of 8.
    using OrderIndex = std::uint32_t;
    static constexpr OrderIndex kNone = std::numeric_limits<OrderIndex>::max();

    // Hot data, read on every level search. 24 bytes.
    struct Level {
        Price price;
        Quantity total_quantity;
        OrderIndex head;  // oldest order, matched first
        OrderIndex tail;  // newest order, where new orders are appended
    };

    // Cold data, only needed when matching walks a level. 16 bytes.
    struct OrderSlot {
        OrderId id;
        Quantity quantity;
        OrderIndex next;  // next order at the same price, or kNone
    };

private:
    std::vector<Level> bids_;  // ascending: best bid at back()
    std::vector<Level> asks_;  // descending: best ask at back()
    std::vector<OrderSlot> orders_;
};

}  // namespace orderbook
