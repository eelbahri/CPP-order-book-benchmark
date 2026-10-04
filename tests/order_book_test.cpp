#include <gtest/gtest.h>

#include "orderbook/order_book_v1.h"
#include "orderbook/order_book_v2.h"

namespace orderbook {
namespace {

Order buy(OrderId id, Price price, Quantity qty = 10) {
    return {.id = id, .price = price, .quantity = qty, .side = Side::Buy};
}

Order sell(OrderId id, Price price, Quantity qty = 10) {
    return {.id = id, .price = price, .quantity = qty, .side = Side::Sell};
}

template <typename Book>
class OrderBookTest : public ::testing::Test {};

using BookVersions = ::testing::Types<OrderBookV1, OrderBookV2>;
TYPED_TEST_SUITE(OrderBookTest, BookVersions);

TYPED_TEST(OrderBookTest, EmptyBookHasNoBestPrices) {
    const TypeParam book;
    EXPECT_FALSE(book.best_bid().has_value());
    EXPECT_FALSE(book.best_ask().has_value());
    EXPECT_EQ(book.order_count(), 0u);
}

TYPED_TEST(OrderBookTest, BestBidIsHighestBuyPrice) {
    TypeParam book;
    book.add(buy(1, 100));
    book.add(buy(2, 102));
    book.add(buy(3, 101));
    EXPECT_EQ(book.best_bid(), 102);
}

TYPED_TEST(OrderBookTest, BestAskIsLowestSellPrice) {
    TypeParam book;
    book.add(sell(1, 105));
    book.add(sell(2, 103));
    book.add(sell(3, 104));
    EXPECT_EQ(book.best_ask(), 103);
}

TYPED_TEST(OrderBookTest, SidesDoNotAffectEachOther) {
    TypeParam book;
    book.add(buy(1, 100));
    EXPECT_EQ(book.best_bid(), 100);
    EXPECT_FALSE(book.best_ask().has_value());
}

TYPED_TEST(OrderBookTest, CountsOrdersAtTheSamePrice) {
    TypeParam book;
    book.add(buy(1, 100));
    book.add(buy(2, 100));
    book.add(sell(3, 105));
    EXPECT_EQ(book.order_count(), 3u);
}

}  // namespace
}  // namespace orderbook
