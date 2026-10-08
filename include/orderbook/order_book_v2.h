#pragma once

#include <cstddef>
#include <deque>
#include <functional>
#include <map>
#include <optional>

#include "orderbook/order.h"

namespace orderbook {

// V2: V1 with bids sorted in descending order, so the best bid is begin(),
// which libc++ caches: best_bid() becomes O(1).
class OrderBookV2 {
public:
    void add(const Order& order);

    std::optional<Price> best_bid() const;
    std::optional<Quantity> best_bid_quantity() const;
    std::optional<Price> best_ask() const;
    std::optional<Quantity> best_ask_quantity() const;

    std::size_t order_count() const;

private:
    std::map<Price, std::deque<Order>, std::greater<>> bids_;
    std::map<Price, std::deque<Order>> asks_;
};

}  // namespace orderbook
