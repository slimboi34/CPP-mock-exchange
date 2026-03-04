#pragma once

#include "Types.h"
#include <map>
#include <unordered_map>
#include <vector>

namespace exchange {

struct PriceLevel {
  Order *head = nullptr;
  Order *tail = nullptr;
  Quantity total_volume = 0;

  void add_order(Order *order) {
    order->prev = tail;
    order->next = nullptr;
    if (tail) {
      tail->next = order;
    } else {
      head = order;
    }
    tail = order;
    total_volume += order->quantity;
  }

  void remove_order(Order *order) {
    if (order->prev) {
      order->prev->next = order->next;
    } else {
      head = order->next;
    }

    if (order->next) {
      order->next->prev = order->prev;
    } else {
      tail = order->prev;
    }

    total_volume -= order->quantity;
    order->prev = nullptr;
    order->next = nullptr;
  }

  bool is_empty() const { return head == nullptr; }
};

class OrderBook {
public:
  // Returns true if the order was added to the book, false if invalid
  bool add_order(Order *order);
  void remove_order(Order *order);

  Order *get_best_bid();
  Order *get_best_ask();

  bool has_bids() const { return !bids_.empty(); }
  bool has_asks() const { return !asks_.empty(); }

  const auto &get_bids() const { return bids_; }
  const auto &get_asks() const { return asks_; }

  // Match against existing orders. Returns filled quantity.
  // If order acts as a taker, this reduces maker orders in the book.
  Quantity match_taker_order(Order *order);

private:
  // Reverse order for bids: highest price first
  std::map<Price, PriceLevel, std::greater<Price>> bids_;
  // Natural order for asks: lowest price first
  std::map<Price, PriceLevel, std::less<Price>> asks_;
};

} // namespace exchange
