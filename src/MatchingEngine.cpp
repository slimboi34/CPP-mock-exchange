#include "MatchingEngine.h"
#include <vector>

namespace exchange {

MatchingEngine::MatchingEngine(size_t max_orders) : order_pool_(max_orders) {
  order_map_.reserve(max_orders);
}

void MatchingEngine::cleanup_order(Order *order) {
  if (!order)
    return;
  order_map_.erase(order->id);
  order_pool_.deallocate(order);
}

void MatchingEngine::add_limit_order(OrderId id, Side side, Price price,
                                     Quantity quantity) {
  if (quantity == 0)
    return;

  // Check if order already exists
  if (order_map_.find(id) != order_map_.end()) {
    return; // Reject duplicate
  }

  // Create new order on the stack to use as taker
  Order taker_order(id, side, price, quantity);

  // Match against opposite side
  order_book_.match_taker_order(&taker_order);

  // Now we must clean up any fully filled maker orders that the OrderBook
  // detached. The OrderBook detached them, but they are still in memory. To
  // find them without modifying OrderBook signature, we can do a lazy sweep or
  // we should really have OrderBook return the filled orders.
  // Given the constraints, let's just sweep our order_map_ for quantity == 0.
  // (In a real high performance engine, OrderBook would notify the engine via
  // callback or return a list)
  std::vector<OrderId> to_remove;
  for (auto &[oid, param] : order_map_) {
    if (param->quantity == 0) {
      to_remove.push_back(oid);
    }
  }
  for (auto oid : to_remove) {
    Order *filled_order = order_map_[oid];
    cleanup_order(filled_order);
  }

  // If taker_order still has quantity, add to book as a maker
  if (taker_order.quantity > 0) {
    Order *new_order =
        order_pool_.allocate(taker_order.id, taker_order.side,
                             taker_order.price, taker_order.quantity);
    if (new_order) {
      order_book_.add_order(new_order);
      order_map_[new_order->id] = new_order;
    }
  }
}

void MatchingEngine::add_market_order(OrderId id, Side side,
                                      Quantity quantity) {
  if (quantity == 0)
    return;

  // A market order is just a limit order with an extreme price
  Price extreme_price =
      (side == Side::Buy) ? std::numeric_limits<Price>::max() : 0;

  // We don't add market orders to the book if they don't fully fill,
  // but the limit logic handles matching.
  Order taker_order(id, side, extreme_price, quantity);
  order_book_.match_taker_order(&taker_order);

  // Cleanup filled makers (same as limit order)
  std::vector<OrderId> to_remove;
  for (auto &[oid, param] : order_map_) {
    // Find fully filled orders
    if (param->quantity == 0) {
      to_remove.push_back(oid);
    }
  }
  for (auto oid : to_remove) {
    Order *filled_order = order_map_[oid];
    cleanup_order(filled_order);
  }
}

void MatchingEngine::cancel_order(OrderId id) {
  auto it = order_map_.find(id);
  if (it != order_map_.end()) {
    Order *order = it->second;
    order_book_.remove_order(order);
    cleanup_order(order);
  }
}

} // namespace exchange
