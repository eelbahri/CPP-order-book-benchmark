#pragma once  // include this header at most once per .cpp (like an include guard)

#include <cstdint>
#include <string_view>

namespace orderbook {

// Prices are integers counted in "ticks" (e.g. 1 tick = 0.01), never double:
// floating point can't represent 0.1 exactly, and 100.10 must equal 100.10.
using Price = std::int64_t;
using Quantity = std::uint32_t;
using OrderId = std::uint64_t;

// "enum class" is a scoped, strongly typed enum (closest to a Java enum,
// but it's just an integer underneath: no methods, no fields).
// ": std::uint8_t" sets its size to 1 byte.
enum class Side : std::uint8_t { Buy, Sell };

// A struct is a class whose members are public by default.
// No "new" needed: an Order is a plain value, 24 bytes, copied like an int.
// Rule of thumb : declare fields from largest to smallest alignment
struct Order {
    OrderId id;
    Price price;
    Quantity quantity;
    Side side;
};

// Declaration only. The definition (the body) lives in src/order.cpp.
std::string_view to_string(Side side);

}  // namespace orderbook
