#include "orderbook/order_book_v1.h"

#include <cstddef>
#include <optional>

namespace orderbook {

void OrderBookV1::add(const Order& order) {
    auto& side = (order.side == Side::Buy) ? bids_ : asks_;
    side[order.price].push_back(order);
}

std::optional<Price> OrderBookV1::best_bid() const {
    if (bids_.empty()) {
        return std::nullopt;
    }
    return bids_.rbegin()->first;
}

std::optional<Quantity> OrderBookV1::best_bid_quantity() const {
    if (bids_.empty()) {
        return std::nullopt;
    }
    Quantity total_qty = 0;
    for (const auto& order : bids_.rbegin()->second) {
        total_qty += order.quantity;
    }
    return total_qty;
}

std::optional<Price> OrderBookV1::best_ask() const {
    if (asks_.empty()) {
        return std::nullopt;
    }
    return asks_.begin()->first;
}

std::optional<Quantity> OrderBookV1::best_ask_quantity() const {
    if (asks_.empty()) {
        return std::nullopt;
    }
    Quantity total_qty = 0;
    for (const auto& order : asks_.begin()->second) {
        total_qty += order.quantity;
    }
    return total_qty;
}

std::size_t OrderBookV1::order_count() const {
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
