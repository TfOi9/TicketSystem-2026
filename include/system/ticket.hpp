#ifndef TICKET_HPP
#define TICKET_HPP

#include <csignal>
#include <fstream>
#include <iostream>
#include <memory>

#include "user.hpp"
#include "train.hpp"
#include "order.hpp"
#include "../command/command.hpp"
#include "../result/result.hpp"

namespace sjtu {

class TicketSystem {
private:
    UserSystem user_;
    TrainSystem train_;
    OrderSystem order_;
    int timestamp_;
    Command *cmd_ = nullptr;
    std::fstream timestamp_file_;
    int order_timestamp_;

public:
    TicketSystem(const std::string& name = "ticket_system") :
        user_(name + "_user"), train_(name + "_train"), order_(name + "_order") {
        timestamp_file_.open("timestamp.dat", std::ios::in | std::ios::out | std::ios::binary);
        if (!timestamp_file_) {
            timestamp_file_.open("timestamp.dat", std::ios::out | std::ios::binary);
            order_timestamp_ = 0;
        }
        else {
            timestamp_file_.seekg(0, std::ios::beg);
            timestamp_file_.read(reinterpret_cast<char *>(&order_timestamp_), sizeof(int));
        }
        // std::cerr << "ots = " << order_timestamp_ << std::endl;
    }

    ~TicketSystem();

    void run(const volatile std::sig_atomic_t* signal_status = nullptr);

    std::unique_ptr<Result> handle(const Command& command);

    bool bootstrapRootSession();

    CompleteTicket complete(const Ticket& ticket);

    CompleteOrder complete(const Order& order);

    CompleteTransferTicket complete(const TransferTicket& transfer);

    void flush();

    void add_user(bool pack = false, Result **res = nullptr);

    void login(bool pack = false, Result **res = nullptr);

    void logout(bool pack = false, Result **res = nullptr);

    void query_profile(bool pack = false, Result **res = nullptr);

    void modify_profile(bool pack = false, Result **res = nullptr);

    void add_train(bool pack = false, Result **res = nullptr);

    void delete_train(bool pack = false, Result **res = nullptr);

    void release_train(bool pack = false, Result **res = nullptr);

    void query_train(bool pack = false, Result **res = nullptr);

    void query_ticket(bool pack = false, Result **res = nullptr);

    void query_transfer(bool pack = false, Result **res = nullptr);

    void buy_ticket(bool pack = false, Result **res = nullptr);

    void query_order(bool pack = false, Result **res = nullptr);

    void refund_ticket(bool pack = false, Result **res = nullptr);

    void clear();

};

} // namespace sjtu

#endif // TICKET_HPP