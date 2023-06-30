#pragma once

#include <string>
#include <list>
#include <unordered_map>
#include <map>
#include <ctime>
#include <vector>

struct alignas(8) Order {
    char type;
    uint32_t oid;
    char side;
    uint32_t qty;
    double px;
    std::string symbol;
};

typedef std::list<std::string> results_t;
//we need the books to be sorted by first price, then by time, to ensure FIFO priority
typedef std::multimap<std::pair<double, std::time_t>, Order> order_book_t;

class SimpleCross {
    private:
        results_t results;
        Order order;
        std::unordered_map<std::string, order_book_t> buyBooks;
        std::unordered_map<std::string, order_book_t> sellBooks;
        std::unordered_map<uint32_t, Order> orderMap;
        std::map<double, std::vector<uint32_t>> orderMap_sortedbyPrice;

        void addOrder(const Order& order) noexcept;
        void removeOrder(uint32_t oid) noexcept;

    public:
        Order parseLine(const std::string& line);
        void validateOrder(const Order& order); //Handle input errors
        results_t processOrder(Order& order);
        results_t processCancel(Order& order);
        results_t processPrint();
        results_t action(const std::string& line);
};
