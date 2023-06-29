/*
SimpleCross - a process that matches internal orders

Overview:
    * Accept/remove orders as they are entered and keep a book of
      resting orders
    * Determine if an accepted order would be satisfied by previously
      accepted orders (i.e. a buy would cross a resting sell)
    * Output (print) crossing events and remove completed (fully filled)
      orders from the book

Inputs:
    A string of space separated values representing an action.  The number of
    values is determined by the action to be performed and have the following
    format:

    ACTION [OID [SYMBOL SIDE QTY PX]]

    ACTION: single character value with the following definitions
    O - place order, requires OID, SYMBOL, SIDE, QTY, PX
    X - cancel order, requires OID
    P - print sorted book (see example below)

    OID: positive 32-bit integer value which must be unique for all orders

    SYMBOL: alpha-numeric string value. Maximum length of 8.

    SIDE: single character value with the following definitions
    B - buy
    S - sell

    QTY: positive 16-bit integer value

    PX: positive double precision value (7.5 format)

Outputs:
    A list of strings of space separated values that show the result of the
    action (if any).  The number of values is determined by the result type and
    have the following format:

    RESULT OID [SYMBOL [SIDE] (FILL_QTY | OPEN_QTY) (FILL_PX | ORD_PX)]

    RESULT: single character value with the following definitions
    F - fill (or partial fill), requires OID, SYMBOL, FILL_QTY, FILL_PX
    X - cancel confirmation, requires OID
    P - book entry, requires OID, SYMBOL, SIDE, OPEN_QTY, ORD_PX (see example below)
    E - error, requires OID. Remainder of line represents string value description of the error

    FILL_QTY: positive 16-bit integer value representing qty of the order filled by
              this crossing event

    OPEN_QTY: positive 16-bit integer value representing qty of the order not yet filled

    FILL_PX:  positive double precision value representing price of the fill of this
              order by this crossing event (7.5 format)

    ORD_PX:   positive double precision value representing original price of the order (7.5 format)
              (7.5 format means up to 7 digits before the decimal and exactly 5 digits after the decimal)

Conditions/Assumptions:
    * The implementation should be a standalone Linux console application (include
      source files, testing tools and Makefile in submission)
    * The use of third party libraries is not permitted. 
    * The app should respond to malformed input and other errors with a RESULT
      of type 'E' and a descriptive error message
    * Development should be production level quality. Design and
      implementation choices should be documented
	* Performance is always a concern in software, but understand that this is an unrealistic test. 
	  Only be concerned about performance where it makes sense to the important sections of this application (i.e. reading actions.txt is not important).
    * All orders are standard limit orders (a limit order means the order remains in the book until it
      is either canceled, or fully filled by order(s) for its same symbol on the opposite side with an
      equal or better price).
    * Orders should be selected for crossing using price-time (FIFO) priority
    * Orders for different symbols should not cross (i.e. the book must support multiple symbols)

Example session:
    INPUT                                   | OUTPUT
    ============================================================================
    "O 10000 IBM B 10 100.00000"            | results.size() == 0
    "O 10001 IBM B 10 99.00000"             | results.size() == 0
    "O 10002 IBM S 5 101.00000"             | results.size() == 0
    "O 10003 IBM S 5 100.00000"             | results.size() == 2
                                            | results[0] == "F 10003 IBM 5 100.00000"
                                            | results[1] == "F 10000 IBM 5 100.00000"
    "O 10004 IBM S 5 100.00000"             | results.size() == 2
                                            | results[0] == "F 10004 IBM 5 100.00000"
                                            | results[1] == "F 10000 IBM 5 100.00000"
    "X 10002"                               | results.size() == 1
                                            | results[0] == "X 10002"
    "O 10005 IBM B 10 99.00000"             | results.size() == 0
    "O 10006 IBM B 10 100.00000"            | results.size() == 0
    "O 10007 IBM S 10 101.00000"            | results.size() == 0
    "O 10008 IBM S 10 102.00000"            | results.size() == 0
    "O 10008 IBM S 10 102.00000"            | results.size() == 1
                                            | results[0] == "E 10008 Duplicate order id"
    "O 10009 IBM S 10 102.00000"            | results.size() == 0
    "P"                                     | results.size() == 6
                                            | results[0] == "P 10009 IBM S 10 102.00000"
                                            | results[1] == "P 10008 IBM S 10 102.00000"
                                            | results[2] == "P 10007 IBM S 10 101.00000"
                                            | results[3] == "P 10006 IBM B 10 100.00000"
                                            | results[4] == "P 10001 IBM B 10 99.00000"
                                            | results[5] == "P 10005 IBM B 10 99.00000"
    "O 10010 IBM B 13 102.00000"            | results.size() == 4
                                            | results[0] == "F 10010 IBM 10 101.00000"
                                            | results[1] == "F 10007 IBM 10 101.00000"
                                            | results[2] == "F 10010 IBM 3 102.00000"
                                            | results[3] == "F 10008 IBM 3 102.00000"

So, for the example actions.txt, the desired output from the application with the below main is:
F 10003 IBM 5 100.00000
F 10000 IBM 5 100.00000
F 10004 IBM 5 100.00000
F 10000 IBM 5 100.00000
X 10002
E 10008 Duplicate order id
P 10009 IBM S 10 102.00000
P 10008 IBM S 10 102.00000
P 10007 IBM S 10 101.00000
P 10006 IBM B 10 100.00000
P 10001 IBM B 10 99.00000
P 10005 IBM B 10 99.00000
F 10010 IBM 10 101.00000
F 10007 IBM 10 101.00000
F 10010 IBM 3 102.00000
F 10008 IBM 3 102.00000

*/

// Stub implementation and example driver for SimpleCross.
// Your crossing logic should be accesible from the SimpleCross class.
// Other than the signature of SimpleCross::action() you are free to modify as needed.
#include <sstream>
#include <fstream>
#include <iostream>
#include <chrono>
#include "simple_cross.h"

void SimpleCross::addOrder(const Order& order) noexcept {

  //find the time the order was inserted to be able to preserve FIFO 
  //for crossing orders with the same price
  auto timeNow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

  //add the order to the respective book
  if (order.side == 'B') {
    buyBook.insert(std::make_pair(std::make_pair(order.px, timeNow), order));
  }
  else {
    sellBook.insert(std::make_pair(std::make_pair(order.px, timeNow), order));
  }

  //add the order to the order map
  orderMap[order.oid] = order;
}

void SimpleCross::removeOrder(const Order& order) noexcept {
  //remove from respective book
  if (order.side == 'B') {
    //remove the specific price and order id
    for (auto it = buyBook.begin(); it != buyBook.end();) {
      if (it->second.oid == order.oid) {
        it = buyBook.erase(it);
      }
      else {
        ++it;
      }
    }
  }
  else {
    for (auto it = sellBook.begin(); it != sellBook.end();) {
      if (it->second.oid == order.oid) {
        it = sellBook.erase(it);
      }
      else {
        ++it;
      }
    }
  }

  //remove from map
  orderMap.erase(order.oid);
}

Order SimpleCross::parseLine(const std::string& line) {
  std::cout << "Parsing Line: " << line << '\n';
  //initialize variables
  Order order;

  //parse the input string
  std::istringstream iss(line);
  iss >> order.type;
  if (order.type == 'O') {
    iss >> order.oid >> order.symbol >> order.side >> order.qty >> order.px;
      std::cout << "Parsed Line: " << order.type << order.oid << " "
      << order.symbol << " " << order.side << " "
      << order.qty << " " << order.px << '\n';
  }
  else if (order.type == 'X') {
    iss >> order.oid;
    std::cout << "Parsed Line " << order.type << order.oid << '\n';
  }
  else if (order.type == 'P') {
    //do nothing
  }

  return(order);
}

void SimpleCross::validateOrder(const Order& order) {
  //check the order
  if (order.type != 'O' && order.type != 'X' && order.type != 'P') {
    results.push_back("E " + std::to_string(order.oid) + " Invalid action");
    return;
  }
  if (order.type == 'O') {
    if (orderMap.find(order.oid) != orderMap.end()) {
      results.push_back("E " + std::to_string(order.oid) + " Duplicate order id");
      return;
    }
    if (order.symbol.length() > 8) {
      results.push_back("E " + std::to_string(order.oid) + " Symbol too long");
      return;
    }
    else if (order.symbol.length() == 0) {
      results.push_back("E " + std::to_string(order.oid) + " Symbol missing");
      return;
    }
    if (order.side != 'B' && order.side != 'S') {
      results.push_back("E " + std::to_string(order.oid) + " Invalid side");
      return;
    }
    if (order.qty <= 0) {
      results.push_back("E " + std::to_string(order.oid) + " Invalid qty");
      return;
    }
    if (order.px <= 0) {
      results.push_back("E " + std::to_string(order.oid) + " Invalid px");
      return;
    }
  }
  else if (order.type == 'X') {
    if (orderMap.find(order.oid) == orderMap.end()) {
      results.push_back("E " + std::to_string(order.oid) + " Order id not found");
      return;
    }
  }
  else if (order.type == 'P') {
    if (buyBook.size() == 0 && sellBook.size() == 0) {
      results.push_back("E " + std::to_string(order.oid) + " Books empty");
      return;
    }
  }

}

results_t SimpleCross::processOrder(Order& order) {
  std::cout << "Processing Order: " << order.oid << '\n';
  //add the order to the book
  addOrder(order);

  //check for crossing
  if (order.side == 'B') {
    //check for crossing
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
          it = sellBook.erase(it);
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
    for (auto it = buyBook.rbegin(); it != buyBook.rend() && order.qty > 0;) {
      if (it->first.first >= order.px) {
        //crossing
        uint16_t fillQty = std::min(order.qty, it->second.qty);
        results.push_back("F " + std::to_string(order.oid) + " " + order.symbol + " " + std::to_string(fillQty) + " " + std::to_string(it->second.px));
        results.push_back("F " + std::to_string(it->second.oid) + " " + it->second.symbol + " " + std::to_string(fillQty) + " " + std::to_string(it->second.px));
        order.qty -= fillQty;
        it->second.qty -= fillQty;
        if (it->second.qty == 0) {
          it = decltype(it)(buyBook.erase(std::next(it).base()));
        }
        else {
          Order newOrder = it->second;
          newOrder.qty -= fillQty;
          buyBook.insert({{newOrder.px, std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())}, newOrder});
          it = decltype(it)(buyBook.erase((++it).base()));
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
    removeOrder(order);
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
  removeOrder(order);

  //add the order to the results
  results.push_back("X " + std::to_string(order.oid));
  return(results);
}

results_t SimpleCross::processPrint(const Order& order) {
  //print the book
  for (auto it = sellBook.begin(); it != sellBook.end(); ++it) {
    results.push_back("P " + std::to_string(it->second.oid) + " " + it->second.symbol + " " + it->second.side + " " + std::to_string(it->second.qty) + " " + std::to_string(it->first.first));
  }
  for (auto it = buyBook.rbegin(); it != buyBook.rend(); ++it) {
    results.push_back("P " + std::to_string(it->second.oid) + " " + it->second.symbol + " " + it->second.side + " " + std::to_string(it->second.qty) + " " + std::to_string(it->first.first));
  }
  return(results);
}

results_t SimpleCross::processError(const Order& order) {
  //add the order to the results
  results.push_back("E " + std::to_string(order.oid) + " Invalid action");
  return(results);
}

results_t SimpleCross::action(const std::string& line) {
  std::cout << "Processing Line: " << line << '\n';
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
    }
    else if (order.type == 'X') {
      results = processCancel(order);
    }
    else if (order.type == 'P') {
      results = processPrint(order);
    }
    else {
      results = processError(order);
    }
  }

  return results;
}

int main(int argc, char **argv)
{
    Order order;
    std::cout << sizeof(order) << '\n'; //should be 32 bytes
    SimpleCross scross;
    std::string line;
    std::ifstream actions("actions.txt", std::ios::in);
    std::cout << "Reading actions.txt\n";
    if (!actions.is_open()) {
    std::cerr << "Failed to open the file 'actions.txt'.\n";
    return 1;
    }
    while (std::getline(actions, line))
    {
        std::cout << "Read Line: " << line << '\n';
        results_t results = scross.action(line);
        for (results_t::const_iterator it=results.begin(); it!=results.end(); ++it)
        {
            std::cout << *it << std::endl;
        }
    }
    return 0;
}