#include "orderbook/order_book_v2.h"

#include <cstddef>
#include <cstdint>
#include <optional>

namespace orderbook {

void OrderBookV2::add(const Order& order) {
    auto add_order = [&order](auto& container) { container[order.price].push_back(order); };
    if (Side::Buy == order.side) {
        add_order(bids_);
    } else {
        add_order(asks_);
    }
}

std::optional<Price> OrderBookV2::best_bid() const {
    if (bids_.empty()) {
        return std::nullopt;
    }
    return bids_.begin()->first;
}

std::optional<Quantity> OrderBookV2::best_bid_quantity() const {
    if (bids_.empty()) {
        return std::nullopt;
    }
    Quantity total_qty = 0;
    for (const auto& order : bids_.begin()->second) {
        total_qty += order.quantity;
    }
    return total_qty;
}

std::optional<Price> OrderBookV2::best_ask() const {
    if (asks_.empty()) {
        return std::nullopt;
    }
    return asks_.begin()->first;
}

std::optional<Quantity> OrderBookV2::best_ask_quantity() const {
    if (asks_.empty()) {
        return std::nullopt;
    }
    Quantity total_qty = 0;
    for (const auto& order : asks_.begin()->second) {
        total_qty += order.quantity;
    }
    return total_qty;
}

std::size_t OrderBookV2::order_count() const {
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
