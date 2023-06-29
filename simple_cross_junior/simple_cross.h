#include <string>
#include <list>
#include <unordered_map>
#include <map>
#include <ctime>

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
        order_book_t buyBook;
        order_book_t sellBook;
        std::unordered_map<uint32_t, Order> orderMap;

        void addOrder(const Order& order) noexcept;
        void removeOrder(const Order& order) noexcept;

    public:
        Order parseLine(const std::string& line);
        void validateOrder(const Order& order); //Handle input errors
        results_t processOrder(Order& order);
        results_t processCancel(Order& order);
        results_t processPrint(const Order& order);
        results_t processError(const Order& order);
        results_t action(const std::string& line);
};
