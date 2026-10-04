#include "orderbook/order.h"

namespace orderbook {

std::string_view to_string(Side side) {
    switch (side) {
        case Side::Buy:
            return "Buy";
        case Side::Sell:
            return "Sell";
    }
    return "Unknown";  // unreachable for valid values, but silences a warning
}

}  // namespace orderbook
