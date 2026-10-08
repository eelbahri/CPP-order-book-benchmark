#pragma once

#include <cstdint>
#include <string_view>

namespace orderbook {

// Prices are integer ticks, never floating point: price comparisons must be exact.
using Price = std::int64_t;
using Quantity = std::uint32_t;
using OrderId = std::uint64_t;

enum class Side : std::uint8_t { Buy, Sell };

// Fields ordered by decreasing alignment: 24 bytes, versus 32 bytes when
// `side` comes first (7 bytes of padding before `id`, 4 more before `price`).
struct Order {
    OrderId id;
    Price price;
    Quantity quantity;
    Side side;
};

std::string_view to_string(Side side);

}  // namespace orderbook
