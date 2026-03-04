#pragma once

#include <cstdint>
#include <string>

namespace exchange {

using OrderId = uint64_t;
using Price = uint64_t; // Prices can be represented as integers (e.g.
                        // multiplied by 10000)
using Quantity = uint64_t;

enum class Side : uint8_t { Buy, Sell };

struct Order {
  OrderId id;
  Side side;
  Price price;
  Quantity quantity;

  // Pointers for a doubly linked list in the order book's price level
  Order *prev = nullptr;
  Order *next = nullptr;

  Order() = default;
  Order(OrderId id, Side side, Price price, Quantity quantity)
      : id(id), side(side), price(price), quantity(quantity), prev(nullptr),
        next(nullptr) {}
};

} // namespace exchange
