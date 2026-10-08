#pragma once

#include <cstddef>
#include <deque>
#include <map>
#include <optional>

#include "orderbook/order.h"

namespace orderbook {

// V1, baseline. Price levels in a std::map (red-black tree), FIFO queue of
// orders per level in a std::deque.
// Costs measured on libc++: a new price level = 3 heap allocations (~4.2 KB,
// the deque allocates a 4 KB block up front); best_bid() is O(log n) because
// libc++ caches the tree's leftmost node but not its rightmost.
class OrderBookV1 {
public:
    void add(const Order& order);

    std::optional<Price> best_bid() const;
    std::optional<Quantity> best_bid_quantity() const;
    std::optional<Price> best_ask() const;
    std::optional<Quantity> best_ask_quantity() const;

    std::size_t order_count() const;

private:
    std::map<Price, std::deque<Order>> bids_;
    std::map<Price, std::deque<Order>> asks_;
};

}  // namespace orderbook
