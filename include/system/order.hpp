#ifndef ORDER_HPP
#define ORDER_HPP

#include "../../include/utils/time_date.hpp"
#include "../../include/utils/fixed_string.hpp"
#include "../../include/storage/bpt.hpp"
#include "../../include/stl/vector.hpp"

namespace sjtu {

struct Ticket {
    int train_id_;
    int start_station_;
    int end_station_;
    date travel_date_;
    time departure_time_;
    time arrival_time_;
    int price_;
    int seat_;
};

enum class TicketStatus {
    Invalid = 0,
    Purchased,
    Pending,
    Refunded
};

struct OrderInfo {
    FixedString<20> user_;
    date purchase_date_;
    time purchase_time;
};

struct Order {
    OrderInfo info_;
    TicketStatus status_;
    Ticket ticket_;
};

struct TransferTicket {
    Ticket first_ticket_;
    Ticket second_ticket_;
};

class OrderSystem {
private:
    BPlusTree<OrderInfo, Order> order_map_;
    BPlusTree<int, Order> queue_map_;

public:
    OrderSystem(const std::string& name = "order") :
        order_map_(name + "_order_map.dat"), queue_map_(name + "_queue_map.dat") {}

    void add_order(const Order& order);

    void delete_order(const Order& order);

    void query_order(const std::string& username, sjtu::vector<Order>& orders);

    void get_pending_queue(sjtu::vector<Order>& pending_orders);

    void remove_pending_order(int pending_id);

    void clear();

};

} // namespace sjtu

#endif // ORDER_HPP