#include <iostream>

#include "orderbook/order.h"

int main() {
    // int prices[3] = {1, 2, 3};
    // int i = 3;
    // std::cout << prices[i] << "\n"; // one past the end
    // Lives on the stack: destroyed automatically when main() returns.
    // Designated initializers (C++20) name each field, like a builder.
    const orderbook::Order order{
        .id = 1,
        .price = 10'050,
        .quantity = 100,
        .side = orderbook::Side::Buy,
    };

    std::cout << "Order #" << order.id << ' ' << orderbook::to_string(order.side) << ' '
              << order.quantity << " @ " << order.price << " ticks\n";
    std::cout << "sizeof(Order) = " << sizeof(orderbook::Order) << " bytes\n";
    return 0;
}
