#include <sstream>
#include <iostream>
#include <cctype>
#include <algorithm>
#include "simple_cross.h"

Order SimpleCross::parseLine(const std::string& line) {
  //initialize Order to default values
  Order order{ ' ', 0, ' ', 0, 0.0, ""};

  //parse the input string
  std::istringstream iss(line);
  iss >> order.type;
  try {
    if (order.type == 'O') {
      //Check if oid is negative
      int negativeCheck;
      iss >> negativeCheck;
      if (negativeCheck < 0) {
        throw std::runtime_error("oid must be positive 32bit");
      }
      order.oid = negativeCheck;

      iss >> order.symbol;

      //Special case where side is BB or SS or even B1, etc.
      std::string fullSide;
      iss >> fullSide;
      if (fullSide.size() > 1) {
        throw std::runtime_error("side must be either B or S");
      }
      order.side = fullSide[0];

      //Check if qty is too large 
      double qtyCheck; 
      iss >> qtyCheck;
      if (qtyCheck > 65535) {
        throw std::runtime_error("qty must be positive 16bit");
      }
      order.qty = (uint16_t)qtyCheck;

      iss >> order.px;
    }
    else if (order.type == 'X') {
      iss >> order.oid;
    }
    //if print, do nothing

    //check if any extra characters exist in the line, or if there are missing arguments for qty or px
    if (iss.rdbuf()->in_avail() != 0) {
      throw std::runtime_error("Extra characters in line or missing arguments for qty or px");
    }
  }
  catch (const std::runtime_error& e) {
    std::string orderId = (order.oid == 3435973836) ? "0" : std::to_string(order.oid);
    results.push_back("E " + orderId + " " + e.what());
  }

  return(order);
}

void SimpleCross::validateOrder(const Order& order) {
  //check the order
  if (order.type != 'O' && order.type != 'X' && order.type != 'P') {
    throw std::runtime_error("Invalid action (Not O or X or P)");
  }

  if (order.type == 'O') {
    //check for valid order id (positive 32bit and unique)
    if (order.oid <= 0) {
      throw std::runtime_error("oid must be positive 32bit");
    }
    if (orderMap.find(order.oid) != orderMap.end()) {
      throw std::runtime_error("Duplicate order id");
    }

    //check for valid symbol (alphanumeric and 8 characters or less)
    if (order.symbol.length() > 8) {
      throw std::runtime_error("Symbol too long");
    }
    else if (order.symbol.length() == 0) {
      throw std::runtime_error("Symbol missing");
    }
    else if (std::any_of(order.symbol.begin(), order.symbol.end(), [](char c) {return !std::isalnum(c); })) {
      throw std::runtime_error("Symbol not alphanumeric");
    }

    //check for valid side (B or S)
    if (order.side != 'B' && order.side != 'S') {
      throw std::runtime_error("Side must be either B or S");
    }

    //check for valid qty (positive 16bit)
    if (order.qty <= 0) {
      throw std::runtime_error("qty must be positive 16bit");
    }

    //check for valid px (positive double 7.5 format (7 digits before decimal, 5 after))
    if (order.px <= 0) {
      throw std::runtime_error("px must be positive double");
    }
    else if (order.px > 9999999) {
      throw std::runtime_error("px must be 7 digits or less before decimal");
    }
    else if (order.px - (uint32_t)(order.px) > 0.000009) {
      throw std::runtime_error("px must be 5 digits or less after decimal");
    }
  }
  else if (order.type == 'X') {
    if (orderMap.find(order.oid) == orderMap.end()) {
      throw std::runtime_error("Order id not found");
    }
  }
}


results_t SimpleCross::processOrder(Order& order) {
  //add the order to the book
  addOrder(order);

  //store a list of orders to be removed
  std::vector<uint32_t> orderIds_to_be_removed;

  if (order.side == 'B') {
    //check for crossing
    auto& sellBook = sellBooks[order.symbol];
    for (auto it = sellBook.begin(); it != sellBook.end() && order.qty > 0;) {
      if (it->second.px <= order.px) {
        //crossing
        uint16_t fillQty = std::min(order.qty, it->second.qty);
        results.push_back("F " + std::to_string(order.oid) + " " + order.symbol + " " + std::to_string(fillQty) + " " + std::to_string(it->second.px));
        results.push_back("F " + std::to_string(it->second.oid) + " " + it->second.symbol + " " + std::to_string(fillQty) + " " + std::to_string(it->second.px));
        //keep track of order quantities
        order.qty -= fillQty;
        it->second.qty -= fillQty;
        if (it->second.qty == 0) {
          //if an order reaches 0 quantity, add it to the list of orders to be removed
          orderIds_to_be_removed.push_back(it->second.oid);
        }
        //increment the iterator
        ++it;
      }
      else { 
        //no crossing
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
        uint16_t fillQty = std::min(order.qty, it->second.qty);
        results.push_back("F " + std::to_string(order.oid) + " " + order.symbol + " " + std::to_string(fillQty) + " " + std::to_string(it->second.px));
        results.push_back("F " + std::to_string(it->second.oid) + " " + it->second.symbol + " " + std::to_string(fillQty) + " " + std::to_string(it->second.px));
        order.qty -= fillQty;
        it->second.qty -= fillQty;
        if (it->second.qty == 0) {
          orderIds_to_be_removed.push_back(it->second.oid);
        }
        ++it;
      }
      else {
        break;
      }
    }
  }

  //remove all orders that were fully filled (and already existing)
  for (uint32_t oid : orderIds_to_be_removed) {
    removeOrder(oid);
  }

  //check to see if current order was also filled, if so remove it
  if (order.qty == 0) {
    removeOrder(order.oid);
  }

  //return the results
  return(results);
}

results_t SimpleCross::processCancel(Order& order) {
  //See if the order exists
  if (orderMap.find(order.oid) == orderMap.end()) {
    results.push_back("E " + std::to_string(order.oid) + " Order id not found");
    return(results);
  }

  //remove the order from the respective book and order maps
  removeOrder(order.oid);

  //add the order to the results
  results.push_back("X " + std::to_string(order.oid));
  return(results);
}

results_t SimpleCross::processPrint() {
  //print the book in descending order of price

  //iterate through the map in reverse order grabbing all the order ids
  for (auto rit = orderMap_sortedbyPrice.rbegin(); rit != orderMap_sortedbyPrice.rend(); ++rit) {
    double price = rit->first;
    const auto& orderIds = rit->second;

    //print all of the open orders
    for (uint32_t oid : orderIds) {
      const Order& order = orderMap[oid];
      results.push_back("P " + std::to_string(order.oid) + " " + order.symbol + " " + order.side + " " +
        std::to_string(order.qty) + " " + std::to_string(price));
    }
  }

  return results;
}

results_t SimpleCross::action(const std::string& line) {
  //main entry point for the OMS

  //clear the results
  results.clear();

  //initialize order struct to default values
  Order order{ ' ', 0, ' ', 0, 0.0, ""};

  //parse the line and validate the order with error checking
  order = parseLine(line);
  if (results.size() != 0) {
    return results;
  }
  try {
    //validate the order
    validateOrder(order);
  }
  catch (const std::runtime_error& e) {
    results.push_back("E " + std::to_string(order.oid) + " " + e.what());
    return results;
  }


  //process the line, if results.size() != 0 then an error occurred
  if (results.size() == 0) {
    if (order.type == 'O') {
      results = processOrder(order);
    }
    else if (order.type == 'X') {
      results = processCancel(order);
    }
    else if (order.type == 'P') {
      results = processPrint();
    }
  }

  return results;
}