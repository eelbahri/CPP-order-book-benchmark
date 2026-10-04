#include "orderbook/order_book.h"

#include <cstddef>
#include <optional>

namespace orderbook {

void OrderBook::add(const Order& order) {
    auto& side = (order.side == Side::Buy) ? bids_ : asks_;
    side[order.price].push_back(order);
}

std::optional<Price> OrderBook::best_bid() const {
    if (bids_.empty()) {
        return std::nullopt;
    }
    return bids_.rbegin()->first;
}

std::optional<Price> OrderBook::best_ask() const {
    if (asks_.empty()) {
        return std::nullopt;
    }
    return asks_.begin()->first;
}

std::size_t OrderBook::order_count() const {
    std::size_t count = 0;
    for (const auto& [key, value] : bids_) {
        count += value.size();
    }
    for (const auto& [key, value] : asks_) {
        count += value.size();
    }
    return count;
}

}  // namespace orderbook
