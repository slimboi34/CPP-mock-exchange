#include "OrderBook.h"
#include <algorithm>

namespace exchange {

bool OrderBook::add_order(Order *order) {
  if (!order || order->quantity == 0)
    return false;

  if (order->side == Side::Buy) {
    bids_[order->price].add_order(order);
  } else {
    asks_[order->price].add_order(order);
  }
  return true;
}

void OrderBook::remove_order(Order *order) {
  if (!order)
    return;

  if (order->side == Side::Buy) {
    auto it = bids_.find(order->price);
    if (it != bids_.end()) {
      it->second.remove_order(order);
      if (it->second.is_empty()) {
        bids_.erase(it);
      }
    }
  } else {
    auto it = asks_.find(order->price);
    if (it != asks_.end()) {
      it->second.remove_order(order);
      if (it->second.is_empty()) {
        asks_.erase(it);
      }
    }
  }
}

Order *OrderBook::get_best_bid() {
  if (bids_.empty())
    return nullptr;
  return bids_.begin()->second.head;
}

Order *OrderBook::get_best_ask() {
  if (asks_.empty())
    return nullptr;
  return asks_.begin()->second.head;
}

Quantity OrderBook::match_taker_order(Order *taker_order) {
  if (!taker_order || taker_order->quantity == 0)
    return 0;

  Quantity initial_qty = taker_order->quantity;

  if (taker_order->side == Side::Buy) {
    // Taker is Buy, match against Asks starting from lowest price
    auto it = asks_.begin();
    while (it != asks_.end() && taker_order->quantity > 0) {
      Price ask_price = it->first;
      // Buy limit price must be >= ask price
      if (taker_order->price < ask_price)
        break;

      PriceLevel &level = it->second;
      Order *maker_order = level.head;

      while (maker_order && taker_order->quantity > 0) {
        // Determine trade quantity
        Quantity trade_qty =
            std::min(taker_order->quantity, maker_order->quantity);

        // Update quantities
        taker_order->quantity -= trade_qty;
        maker_order->quantity -= trade_qty;
        level.total_volume -= trade_qty;

        Order *next_maker = maker_order->next;
        if (maker_order->quantity == 0) {
          // Maker order is fully filled
          level.remove_order(maker_order);
          // Signal the caller to handle the fully filled maker_order
          // (we just decouple it from the book, memory management is done via
          // engine)
        }
        maker_order = next_maker;
      }

      if (level.is_empty()) {
        it = asks_.erase(it);
      } else {
        break; // Level not empty means taker is fully filled
      }
    }
  } else {
    // Taker is Sell, match against Bids starting from highest price
    auto it = bids_.begin();
    while (it != bids_.end() && taker_order->quantity > 0) {
      Price bid_price = it->first;
      // Sell limit price must be <= bid price
      if (taker_order->price > bid_price)
        break;

      PriceLevel &level = it->second;
      Order *maker_order = level.head;

      while (maker_order && taker_order->quantity > 0) {
        Quantity trade_qty =
            std::min(taker_order->quantity, maker_order->quantity);

        taker_order->quantity -= trade_qty;
        maker_order->quantity -= trade_qty;
        level.total_volume -= trade_qty;

        Order *next_maker = maker_order->next;
        if (maker_order->quantity == 0) {
          level.remove_order(maker_order);
        }
        maker_order = next_maker;
      }

      if (level.is_empty()) {
        it = bids_.erase(it);
      } else {
        break;
      }
    }
  }

  return initial_qty - taker_order->quantity;
}

} // namespace exchange
