#pragma once

#include <cstddef>
#include <deque>
#include <map>
#include <optional>

#include "orderbook/order.h"

namespace orderbook {

// V1: the naive version, kept frozen as the baseline for comparisons.
// Price levels in std::map (sorted tree), orders per level in std::deque.
// Measured: best_bid() is O(log n) on libc++ (rbegin() walks the tree).
// For now orders just rest in the book: no matching yet.
class OrderBookV1 {
public:
    // const& : read the caller's Order without copying it (see explanation).
    void add(const Order& order);

    // std::optional<T> ~ java.util.Optional<T>, but stored inline:
    // no heap allocation, it's just a T plus a bool.
    // The trailing "const" promises these methods don't modify the book.
    std::optional<Price> best_bid() const;  // highest buy price, if any
    std::optional<Quantity> best_bid_quantity() const;
    std::optional<Price> best_ask() const;  // lowest sell price, if any
    std::optional<Quantity> best_ask_quantity() const;

    std::size_t order_count() const;  // total resting orders

private:
    // Price level -> orders at that price, in arrival order (FIFO).
    // std::map is a sorted tree, ascending by key (like Java's TreeMap).
    std::map<Price, std::deque<Order>> bids_;
    std::map<Price, std::deque<Order>> asks_;
};

}  // namespace orderbook
