#pragma once

#include "simple_cross.h"

class OrderManagementSystem {
private:
    SimpleCross simpleCross;

public:
    void validateOrder(const Order& order);
    results_t processOrder(Order& order);
    results_t processCancel(Order& order);
    results_t processPrint();
    results_t action(const std::string& line);
};
