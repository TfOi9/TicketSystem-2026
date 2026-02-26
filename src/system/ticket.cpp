#include "../../include/system/ticket.hpp"

namespace sjtu {

TicketSystem::~TicketSystem() {
    if (cmd_) {
        delete cmd_;
        cmd_ = nullptr;
    }
}

void TicketSystem::run() {
    while (true) {
        std::string line;
        std::getline(std::cin, line);
        try {
            if (cmd_) {
                delete cmd_;
                cmd_ = nullptr;
            }
            TokenStream stream(line);
            cmd_ = new Command(stream);
            timestamp_ = cmd_->timestamp();
        }
        catch(...) {
            std::cout << "[" << timestamp_ << "] -1\n";
            continue;
        }
        std::cout << "[" << timestamp_ << "] ";
        std::string cmd = cmd_->cmd();
        if (cmd == "add_user") {
            if (!cmd_->check("cupnmg", "")) {
                std::cout << "-1\n";
            }
            else {
                add_user();
            }
        }
        else if (cmd == "login") {
            if (!cmd_->check("up", "")) {
                std::cout << "-1\n";
            }
            else {
                login();
            }
        }
        else if (cmd == "logout") {
            if (!cmd_->check("u", "")) {
                std::cout << "-1\n";
            }
            else {
                logout();
            }
        }
        else if (cmd == "query_profile") {
            if (!cmd_->check("cu", "")) {
                std::cout << "-1\n";
            }
            else {
                query_profile();
            }
        }
        else if (cmd == "modify_profile") {
            if (!cmd_->check("cu", "pnmg")) {
                std::cout << "-1\n";
            }
            else {
                modify_profile();
            }
        }
        else if (cmd == "add_train") {
            if (!cmd_->check("inmspxtody", "")) {
                std::cout << "-1\n";
            }
            else {
                add_train();
            }
        }
        else if (cmd == "delete_train") {
            if (!cmd_->check("i", "")) {
                std::cout << "-1\n";
            }
            else {
                delete_train();
            }
        }
        else if (cmd == "release_train") {
            if (!cmd_->check("i", "")) {
                std::cout << "-1\n";
            }
            else {
                release_train();
            }
        }
        else if (cmd == "query_train") {
            if (!cmd_->check("id", "")) {
                std::cout << "-1\n";
            }
            else {
                query_train();
            }
        }
        else if (cmd == "query_ticket") {
            if (!cmd_->check("std", "p")) {
                std::cout << "-1\n";
            }
            else {
                query_ticket();
            }
        }
        else if (cmd == "query_transfer") {
            if (!cmd_->check("std", "p")) {
                std::cout << "-1\n";
            }
            else {
                query_transfer();
            }
        }
        else if (cmd == "buy_ticket") {
            if (!cmd_->check("uidnft", "q")) {
                std::cout << "-1\n";
            }
            else {
                buy_ticket();
            }
        }
        else if (cmd == "query_order") {
            if (!cmd_->check("u", "")) {
                std::cout << "-1\n";
            }
            else {
                query_order();
            }
        }
        else if (cmd == "refund_ticket") {
            if (!cmd_->check("u", "n")) {
                std::cout << "-1\n";
            }
            else {
                refund_ticket();
            }
        }
        else if (cmd == "clean") {
            if (!cmd_->check("", "")) {
                std::cout << "-1\n";
            }
            else {
                clear();
            }
        }
        else if (cmd == "exit") {
            if (!cmd_->check("", "")) {
                std::cout << "-1\n";
            }
            else {
                std::cout << "bye\n";
                break;
            }
        }
    }
}

} // namespace sjtu