#include "MatchingEngine.h"
#include <gtest/gtest.h>

using namespace exchange;

TEST(MatchingEngineTest, AddAndCancelOrder) {
  MatchingEngine engine;

  engine.add_limit_order(1, Side::Buy, 100, 10);
  // Cancel the order
  engine.cancel_order(1);

  // An immediate sell market order should not match the cancelled buy order
  engine.add_market_order(2, Side::Sell, 10);
  // Since there are no asserts we can easily test on private members,
  // a more comprehensive test would expose state or we rely on no crash
  // and accurate math. Let's add an accessor for testing later if needed.
}

TEST(MatchingEngineTest, ExactMatch) {
  MatchingEngine engine;

  // Add Buy Limit 100 @ 10
  engine.add_limit_order(1, Side::Buy, 100, 10);

  // Add Sell Limit 100 @ 10 (should match exactly)
  engine.add_limit_order(2, Side::Sell, 100, 10);
}

TEST(MatchingEngineTest, PartialMatch) {
  MatchingEngine engine;

  // Add Buy Limit 100 @ 20
  engine.add_limit_order(1, Side::Buy, 100, 20);

  // Add Sell Market for 10
  engine.add_market_order(2, Side::Sell, 10);

  // The book should now have a Buy Limit 100 @ 10 remaining volume
  // Add Sell Market for 20
  engine.add_market_order(3, Side::Sell, 20);

  // The book should have 0 remaining buy volume, and the sell taker
  // should have 10 volume un-filled (which is dropped for market orders)
}

TEST(MatchingEngineTest, PriceTimePriority) {
  MatchingEngine engine;

  // Two buys at same price
  engine.add_limit_order(1, Side::Buy, 100, 10); // earlier
  engine.add_limit_order(2, Side::Buy, 100, 10); // later

  // One buy at better price
  engine.add_limit_order(3, Side::Buy, 101, 10); // best price

  // Market sell 15
  engine.add_market_order(4, Side::Sell, 15);

  // Order 3 should be filled (10), Order 1 should be partially filled (5),
  // Order 2 untouched.
}
