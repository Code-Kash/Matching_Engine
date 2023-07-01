#pragma once

#include "simple_cross.h"

class OrderManagementSystem {
private:
    SimpleCross simpleCross;

public:
    void validateOrder(const Order& order);
    [[nodiscard]] results_t processOrder(Order& order);
    [[nodiscard]] results_t processCancel(Order& order);
    [[nodiscard]] results_t processPrint();
    [[nodiscard]] results_t action(const std::string& line);
};
