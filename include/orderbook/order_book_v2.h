#pragma once

#include <cstddef>
#include <deque>
#include <functional>
#include <map>
#include <optional>

#include "orderbook/order.h"

namespace orderbook {

// V2: same as V1, but bids_ sorted in descending order so that the best bid
// is begin() instead of rbegin().
// For now orders just rest in the book: no matching yet.
class OrderBookV2 {
public:
    // const& : read the caller's Order without copying it (see explanation).
    void add(const Order& order);

    // std::optional<T> ~ java.util.Optional<T>, but stored inline:
    // no heap allocation, it's just a T plus a bool.
    // The trailing "const" promises these methods don't modify the book.
    std::optional<Price> best_bid() const;  // highest buy price, if any
    std::optional<Price> best_ask() const;  // lowest sell price, if any

    std::size_t order_count() const;  // total resting orders

private:
    // Price level -> orders at that price, in arrival order (FIFO).
    // std::map is a sorted tree, ascending by key, descending for bids so begin() is always the top
    // of the book, makes it O(1).
    std::map<Price, std::deque<Order>, std::greater<>> bids_;
    std::map<Price, std::deque<Order>> asks_;
};

}  // namespace orderbook
