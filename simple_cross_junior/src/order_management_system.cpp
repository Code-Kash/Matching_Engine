#include <sstream>
#include <iostream>
#include "simple_cross.h"

Order SimpleCross::parseLine(const std::string& line) {
  //initialize variables
  Order order;

  //parse the input string
  std::istringstream iss(line);
  iss >> order.type;
  if (order.type == 'O') {
    iss >> order.oid >> order.symbol >> order.side >> order.qty >> order.px;
  }
  else if (order.type == 'X') {
    iss >> order.oid;
  }

  return(order);
}

void SimpleCross::validateOrder(const Order& order) {
  try {
    //check the order
    if (order.type != 'O' && order.type != 'X' && order.type != 'P') {
      throw std::runtime_error("Invalid action");
    }
    if (order.type == 'O') {
      if (orderMap.find(order.oid) != orderMap.end()) {
        throw std::runtime_error("Duplicate order id");
      }
      if (order.symbol.length() > 8) {
        throw std::runtime_error("Symbol too long");
      }
      else if (order.symbol.length() == 0) {
        throw std::runtime_error("Symbol missing");
      }
      if (order.side != 'B' && order.side != 'S') {
        throw std::runtime_error("Invalid side");
      }
      if (order.qty <= 0) {
        throw std::runtime_error("Invalid qty");
      }
      if (order.px <= 0) {
        throw std::runtime_error("Invalid px");
      }
    }
    else if (order.type == 'X') {
      if (orderMap.find(order.oid) == orderMap.end()) {
        throw std::runtime_error("Order id not found");
      }
    }
  }
  catch (const std::runtime_error& e) {
    results.push_back("E " + std::to_string(order.oid) + " " + e.what());
  }
}


results_t SimpleCross::processOrder(Order& order) {
  //add the order to the book
  addOrder(order);

  //check for crossing
  if (order.side == 'B') {
    //check for crossing
    auto& sellBook = sellBooks[order.symbol];
    for (auto it = sellBook.begin(); it != sellBook.end() && order.qty > 0;) {
      if (it->second.px <= order.px) {
        //crossing
        uint16_t fillQty = std::min(order.qty, it->second.qty);
        results.push_back("F " + std::to_string(order.oid) + " " + order.symbol + " " + std::to_string(fillQty) + " " + std::to_string(it->second.px));
        results.push_back("F " + std::to_string(it->second.oid) + " " + it->second.symbol + " " + std::to_string(fillQty) + " " + std::to_string(it->second.px));
        order.qty -= fillQty;
        it->second.qty -= fillQty;
        if (it->second.qty == 0) {
          //Erase the order and get the new valid iterator
          auto eraseIt = it;
          ++it;
          removeOrder(eraseIt->second.oid);
        }
        else {
          ++it;
        }
      }
      else {
        break;
      }
    }
  }
  else {
    //check for crossing
    auto& buyBook = buyBooks[order.symbol];
    //need to reverse iterate to get the oldest order first
    for (auto it = buyBook.rbegin(); it != buyBook.rend() && order.qty > 0;) {
      if (it->second.px >= order.px) {
        //crossing
        uint16_t fillQty = std::min(order.qty, it->second.qty);
        results.push_back("F " + std::to_string(order.oid) + " " + order.symbol + " " + std::to_string(fillQty) + " " + std::to_string(it->second.px));
        results.push_back("F " + std::to_string(it->second.oid) + " " + it->second.symbol + " " + std::to_string(fillQty) + " " + std::to_string(it->second.px));
        order.qty -= fillQty;
        it->second.qty -= fillQty;
        if (it->second.qty == 0) {
          removeOrder(it->second.oid);
          ++it;
        }
        else {
          ++it;
        }
      }
      else {
        break;
      }
    }
  }

  //check for completed orders
  if (order.qty == 0) {
    //remove the order from the respective book and order map
    removeOrder(order.oid);
  }

  //return the results
  return(results);
}

results_t SimpleCross::processCancel(Order& order) {
  //perform error checking
  if (orderMap.find(order.oid) == orderMap.end()) {
    results.push_back("E " + std::to_string(order.oid) + " Order id not found");
    return(results);
  }

  //remove the order from the respective book and order map
  removeOrder(order.oid);

  //add the order to the results
  results.push_back("X " + std::to_string(order.oid));
  return(results);
}

results_t SimpleCross::processPrint() {
  for (auto rit = orderMap_sortedbyPrice.rbegin(); rit != orderMap_sortedbyPrice.rend(); ++rit) {
    double price = rit->first;
    const auto& orderIds = rit->second;

    for (uint32_t oid : orderIds) {
      const Order& order = orderMap[oid];
      results.push_back("P " + std::to_string(order.oid) + " " + order.symbol + " " + order.side + " " +
        std::to_string(order.qty) + " " + std::to_string(price));
    }
  }

  return results;
}

results_t SimpleCross::action(const std::string& line) {
  //clear the results
  results.clear();

  //parse the line
  Order order = parseLine(line);

  //validate the line
  validateOrder(order);

  //process the line
  if (results.size() == 0) {
    if (order.type == 'O') {
      results = processOrder(order);

      std::cout << "sellBook size after processing: " << sellBooks[order.symbol].size() << std::endl;
      std::cout << "buyBook size after processing: " << buyBooks[order.symbol].size() << std::endl;
    }
    else if (order.type == 'X') {
      results = processCancel(order);
      std::cout << "sellBook size after processing: " << sellBooks[order.symbol].size() << std::endl;
      std::cout << "buyBook size after processing: " << buyBooks[order.symbol].size() << std::endl;
    }
    else if (order.type == 'P') {
      results = processPrint();
      std::cout << "sellBook size after processing: " << sellBooks[order.symbol].size() << std::endl;
      std::cout << "buyBook size after processing: " << buyBooks[order.symbol].size() << std::endl;
    }
  }

  return results;
}