#pragma once

#include "MemoryPool.h"
#include "OrderBook.h"
#include "Types.h"
#include <unordered_map>

namespace exchange {

class MatchingEngine {
public:
  // Initialize with a given max capacity for orders to avoid dynamic allocation
  explicit MatchingEngine(size_t max_orders = 1000000);

  // Process a new limit order
  void add_limit_order(OrderId id, Side side, Price price, Quantity quantity);

  // Process a market order (matches against best price until filled or book
  // empty)
  void add_market_order(OrderId id, Side side, Quantity quantity);

  // Cancel an existing order by ID
  void cancel_order(OrderId id);

  // For GUI rendering
  const OrderBook &get_order_book() const { return order_book_; }

private:
  OrderBook order_book_;
  MemoryPool<Order> order_pool_;

  // Fast lookup for cancellations
  std::unordered_map<OrderId, Order *> order_map_;

  // Helper to remove order from memory and map
  void cleanup_order(Order *order);

  // Helper to clean up fully filled orders from the book after taker matches
  // In our OrderBook implementation, fully filled maker orders are detached
  // from the book. We need the MatchingEngine to iterate through order_map_ or
  // return a vector of filled IDs to clean them up. A better way is: when
  // OrderBook matches, we need to know WHICH makers were filled so
  // MatchingEngine can deallocate them.

  // Let's modify the add_limit_order to do the cleanup of filled maker orders.
};

} // namespace exchange
