#include "orderbook/order.h"

#include <gtest/gtest.h>

namespace orderbook {
namespace {

TEST(SideTest, ToStringNamesEachSide) {
    EXPECT_EQ(to_string(Side::Buy), "Buy");
    EXPECT_EQ(to_string(Side::Sell), "Sell");
}

TEST(OrderTest, HasNoPaddingWaste) {
    static_assert(sizeof(Order) == 24, "Order layout changed: check field order");
    SUCCEED();
}

TEST(OrderTest, CopiesAreIndependentValues) {
    const Order a{.id = 1, .price = 100, .quantity = 10, .side = Side::Sell};
    Order b = a;
    b.quantity = 99;
    EXPECT_EQ(a.quantity, 10u);
}

}  // namespace
}  // namespace orderbook
