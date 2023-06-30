#pragma once

#include "simple_cross.h"

class BookBuilder {
public:
    void addOrder(const Order& order) noexcept;
    void removeOrder(uint32_t oid) noexcept;
};
