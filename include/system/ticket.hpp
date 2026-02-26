#ifndef TICKET_HPP
#define TICKET_HPP

#include "user.hpp"
#include "train.hpp"
#include "order.hpp"
#include "../command/command.hpp"

namespace sjtu {

class TicketSystem {
private:
    UserSystem user_;
    TrainSystem train_;
    OrderSystem order_;
    int timestamp_;
    Command *cmd_ = nullptr;
    bool init_ = false;

public:
    TicketSystem(const std::string& name = "ticket_system") :
        user_(name + "_user"), train_(name + "_train"), order_(name + "_order") {}

    ~TicketSystem();

    void run();

    void add_user();

    void login();

    void logout();

    void query_profile();

    void modify_profile();

    void add_train();

    void delete_train();

    void release_train();

    void query_train();

    void query_ticket();

    void query_transfer();

    void buy_ticket();

    void query_order();

    void refund_ticket();

    void clear();

};

} // namespace sjtu

#endif // TICKET_HPP