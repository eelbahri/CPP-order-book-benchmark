#include <iostream>

#include "orderbook/order_book_v3.h"

int main() {
    using namespace orderbook;

    OrderBookV3 book;
    book.add({.id = 1, .price = 10'050, .quantity = 100, .side = Side::Buy});
    book.add({.id = 2, .price = 10'050, .quantity = 40, .side = Side::Buy});
    book.add({.id = 3, .price = 10'055, .quantity = 70, .side = Side::Sell});

    std::cout << "best bid: " << book.best_bid_quantity().value_or(0) << " @ "
              << book.best_bid().value_or(0) << '\n'
              << "best ask: " << book.best_ask_quantity().value_or(0) << " @ "
              << book.best_ask().value_or(0) << '\n'
              << "orders:   " << book.order_count() << '\n';
    return 0;
}
