#include <iostream>
#include <chrono>
#include "book_builder.h"

void SimpleCross::addOrder(const Order& order) noexcept {

  //find the time the order was inserted to be able to preserve FIFO 
  //for crossing orders with the same price
  auto timeNow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

  //add the order to the respective book
  if (order.side == 'B') {
    buyBooks[order.symbol].insert(std::make_pair(std::make_pair(order.px, timeNow), order));
  }
  else {
    sellBooks[order.symbol].insert(std::make_pair(std::make_pair(order.px, timeNow), order));
  }

  //add the order to the order maps
  orderMap[order.oid] = order;
  orderMap_sortedbyPrice[order.px].push_back(order.oid);
}

void SimpleCross::removeOrder(uint32_t oid) noexcept {
    // Find the order in the order map
    auto it = orderMap.find(oid);

    // Remove from respective book
    const Order& order = it->second;
    order_book_t& book = (order.side == 'B') ? buyBooks[order.symbol] : sellBooks[order.symbol];

    for (auto it = book.begin(); it != book.end();) {
        if (it->second.oid == oid) {
            it = book.erase(it);
        } else {
            ++it;
        }
    }

    // Remove from maps
    orderMap.erase(oid);

    for (auto& entry : orderMap_sortedbyPrice) {
        auto& orderIds = entry.second;
        orderIds.erase(std::remove(orderIds.begin(), orderIds.end(), oid), orderIds.end());
    }
    std::cout << orderMap_sortedbyPrice.size() << '\n';
}