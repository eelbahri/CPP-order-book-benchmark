#include "orderbook/order_book_v3.h"

#include <cstddef>
#include <functional>
#include <optional>

#include "orderbook/order.h"

namespace orderbook {
namespace {  // helpers private to this file

template <typename Better>
OrderBookV3::Level& find_or_insert_level(std::vector<OrderBookV3::Level>& levels, Price price,
                                         Better is_better) {
    std::size_t index = levels.size();
    while (index > 0 && is_better(levels[index - 1].price, price)) {
        index = index - 1;
    }

    // now levels[i-1] (if it exists) is the first level NOT better than ours
    if (index > 0 && levels[index - 1].price == price) {
        return levels[index - 1];
    }

    auto it = levels.insert(levels.begin() + static_cast<std::ptrdiff_t>(index),
                            {.price = price,
                             .total_quantity = 0,
                             .head = OrderBookV3::kNone,
                             .tail = OrderBookV3::kNone});
    return *it;
}

}  // namespace

void OrderBookV3::add(const Order& order) {
    const auto next_order_position = static_cast<OrderIndex>(orders_.size());
    orders_.push_back({order.id, order.quantity, kNone});

    Level& level = (order.side == Side::Buy)
                       ? find_or_insert_level(bids_, order.price, std::greater<>{})
                       : find_or_insert_level(asks_, order.price, std::less<>{});

    level.total_quantity += order.quantity;
    if (kNone == level.head) {  // is level empty
        level.head = next_order_position;
    } else {
        orders_[level.tail].next = next_order_position;
    }
    level.tail = next_order_position;
}

std::optional<Price> OrderBookV3::best_bid() const {
    if (bids_.empty()) {
        return std::nullopt;
    }
    return bids_.back().price;
}

std::optional<Quantity> OrderBookV3::best_bid_quantity() const {
    if (bids_.empty()) {
        return std::nullopt;
    }
    return bids_.back().total_quantity;
}

std::optional<Price> OrderBookV3::best_ask() const {
    if (asks_.empty()) {
        return std::nullopt;
    }
    return asks_.back().price;
}

std::optional<Quantity> OrderBookV3::best_ask_quantity() const {
    if (asks_.empty()) {
        return std::nullopt;
    }
    return asks_.back().total_quantity;
}

std::size_t OrderBookV3::order_count() const { return orders_.size(); }

}  // namespace orderbook
