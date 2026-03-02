#ifndef TICKET_HPP
#define TICKET_HPP

#include <csignal>
#include <fstream>

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
        std::cerr << "ots = " << order_timestamp_ << std::endl;
    }

    ~TicketSystem();

    void run(const volatile std::sig_atomic_t* signal_status = nullptr);

    void flush();

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