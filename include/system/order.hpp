#ifndef ORDER_HPP
#define ORDER_HPP

#include "../../include/utils/time_date.hpp"
#include "../../include/utils/fixed_string.hpp"
#include "../../include/storage/bpt.hpp"
#include "../../include/stl/vector.hpp"

namespace sjtu {

struct Ticket {
    FixedString<20> train_id_;
    int start_station_;
    int end_station_;
    date departure_date_;
    time departure_time_;
    date arrival_date_;
    time arrival_time_;
    int duration_;
    int price_;
    int seat_;
};

struct TicketTrainIDCompare {
    bool operator()(const Ticket& a, const Ticket& b) const {
        return a.train_id_ < b.train_id_;
    }
};

struct TicketDurationCompare {
    bool operator()(const Ticket& a, const Ticket& b) const {
        if (a.duration_ == b.duration_) {
            return a.train_id_ < b.train_id_;
        }
        return a.duration_ < b.duration_;
    }
};

struct TicketPriceCompare {
    bool operator()(const Ticket& a, const Ticket& b) const {
        if (a.price_ == b.price_) {
            return a.train_id_ < b.train_id_;
        }
        return a.price_ < b.price_;
    }
};

struct TicketEndStationCompare {
    bool operator()(const Ticket& a, const Ticket& b) const {
        if (a.end_station_ == b.end_station_) {
            return a.train_id_ < b.train_id_;
        }
        return a.end_station_ < b.end_station_;
    }
};

enum class TicketStatus {
    Invalid = 0,
    Purchased,
    Pending,
    Refunded
};

struct OrderInfo {
    FixedString<20> user_;
    int purchase_timestamp_;
};

inline bool operator==(const OrderInfo& a, const OrderInfo& b) {
    return a.user_ == b.user_ && a.purchase_timestamp_ == b.purchase_timestamp_;
}

inline bool operator!=(const OrderInfo& a, const OrderInfo& b) {
    return !(a == b);
}

inline bool operator<(const OrderInfo& a, const OrderInfo& b) {
    if (a.user_ == b.user_) {
        return a.purchase_timestamp_ < b.purchase_timestamp_;
    }
    return a.user_ < b.user_;
}

inline std::ostream& operator<<(std::ostream& os, const OrderInfo& o) {
    os << o.user_.str() << " " << o.purchase_timestamp_;
    return os;
}

struct Order {
    OrderInfo info_;
    TicketStatus status_;
    Ticket ticket_;
};

struct TransferTicket {
    Ticket first_ticket_;
    Ticket second_ticket_;

    int price() const {
        return first_ticket_.price_ + second_ticket_.price_;
    }

    int duration() const {
        int arrival = int(second_ticket_.arrival_date_) * 1440 + second_ticket_.arrival_time_.hr_ * 60 + second_ticket_.arrival_time_.min_;
        int departure = int(first_ticket_.departure_date_) * 1440 + first_ticket_.departure_time_.hr_ * 60 +first_ticket_.departure_time_.min_;
        return arrival - departure;
    }

};

struct TransferTicketPriceCompare {
    bool operator()(const TransferTicket& a, const TransferTicket& b) const {
        int ap = a.price(), bp = b.price(), ad = a.duration(), bd = b.duration();
        if (ap == bp) {
            if (ad == bd) {
                if (a.first_ticket_.train_id_ == b.first_ticket_.train_id_) {
                    return a.second_ticket_.train_id_ < b.second_ticket_.train_id_;
                }
                return a.first_ticket_.train_id_ < b.first_ticket_.train_id_;
            }
            return ad < bd;
        }
        return ap < bp;
    }
};

struct TransferTicketDurationCompare {
    bool operator()(const TransferTicket& a, const TransferTicket& b) const {
        int ap = a.price(), bp = b.price(), ad = a.duration(), bd = b.duration();
        if (ad == bd) {
            if (ap == bp) {
                if (a.first_ticket_.train_id_ == b.first_ticket_.train_id_) {
                    return a.second_ticket_.train_id_ < b.second_ticket_.train_id_;
                }
                return a.first_ticket_.train_id_ < b.first_ticket_.train_id_;
            }
            return ap < bp;
        }
        return ad < bd;
    }
};

class OrderSystem {
private:
    BPlusTree<FixedString<20>, Order> user_order_map_;
    BPlusTree<OrderInfo, Order> order_map_;
    BPlusTree<int, Order> queue_map_;

public:
    OrderSystem(const std::string& name = "order") :
        user_order_map_(name + "_user_order_map.dat"), order_map_(name + "_order_map.dat"), queue_map_(name + "_queue_map.dat") {}

    void add_order(const Order& order);

    void delete_order(const Order& order);

    void query_order(const std::string& username, sjtu::vector<Order>& orders);

    void query_order(const OrderInfo& info, sjtu::vector<Order>& orders);

    void get_pending_queue(sjtu::vector<Order>& pending_orders);

    void remove_pending_order(int pending_id);

    void flush();

    void clear();

};

} // namespace sjtu

#endif // ORDER_HPP