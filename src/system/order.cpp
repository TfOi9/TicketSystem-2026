#include "../../include/system/order.hpp"

namespace sjtu {

void OrderSystem::add_order(const Order& order) {
    user_order_map_.insert(order.info_.user_, order);
    order_map_.insert(order.info_, order);
}

void OrderSystem::delete_order(const Order &order) {
    user_order_map_.erase(order.info_.user_, order);
    order_map_.erase(order.info_, order);
}

void OrderSystem::query_order(const std::string &username, sjtu::vector<Order> &orders) {
    user_order_map_.find_all(FixedString<20>(username), orders);
}

void OrderSystem::query_order(const OrderInfo& info, sjtu::vector<Order>& orders) {
    order_map_.find_all(info, orders);
}

void OrderSystem::get_pending_queue(sjtu::vector<Order> &pending_orders) {
    queue_map_.serialize(pending_orders);
}

void OrderSystem::remove_pending_order(int pending_id) {
    auto order = queue_map_.find(pending_id);
    if (order.has_value()) {
        queue_map_.erase(pending_id, order.value());
    }
}

void OrderSystem::flush() {
    user_order_map_.flush();
    order_map_.flush();
    queue_map_.flush();
}

void OrderSystem::clear() {
    user_order_map_.clear();
    order_map_.clear();
    queue_map_.clear();
}

} // namespace sjtu