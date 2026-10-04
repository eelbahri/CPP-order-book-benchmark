#include "orderbook/order.h"

#include <gtest/gtest.h>

namespace orderbook {
namespace {

TEST(SideTest, ToStringNamesEachSide) {
    EXPECT_EQ(to_string(Side::Buy), "Buy");
    EXPECT_EQ(to_string(Side::Sell), "Sell");
}

TEST(OrderTest, IsAPlainValue) {
    Order a{
        .id = 1,
        .price = 100,
        .quantity = 10,
        .side = Side::Sell,
    };
    Order b = a;  // a COPY, not a second reference (unlike Java)
    b.quantity = 99;
    EXPECT_EQ(a.quantity, 10u);  // a is untouched
}

TEST(OrderTest, IsASell) {
    Order a{
        .id = 1,
        .price = 100,
        .quantity = 10,
        .side = Side::Sell,
    };
    EXPECT_EQ(a.side, Side::Sell);
}

}  // namespace
}  // namespace orderbook
