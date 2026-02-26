#include "../../include/system/ticket.hpp"
#include "../../include/utils/validator.hpp"
#include <optional>

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

void TicketSystem::add_user() {
    // std::cout << "add_user\n";
    int ret = 0;
    if (!verify_username(cmd_->arg('u')) || !verify_password(cmd_->arg('p'))
        || !verify_chinese_name(cmd_->arg('n')) || !verify_email(cmd_->arg('m'))) {
        std::cout << "-1\n";
        return;
    }
    if (!init_) {
        ret = user_.add_user("", cmd_->arg('u'), cmd_->arg('p'), cmd_->arg('n'), cmd_->arg('m'), 10);
        if (!ret) {
            init_ = true;
        }
    }
    else {
        int g = verify_privilege(cmd_->arg('g'));
        if (g == -1 || !verify_username(cmd_->arg('c'))) {
            std::cout << "-1\n";
            return;
        }
        ret = user_.add_user(cmd_->arg('c'), cmd_->arg('u'), cmd_->arg('p'), cmd_->arg('n'), cmd_->arg('m'), g);
    }
    std::cout << ret << '\n';
}

void TicketSystem::login() {
    // std::cout << "login\n";
    int ret = 0;
    if (!verify_username(cmd_->arg('u')) || !verify_password(cmd_->arg('p'))) {
        std::cout << "-1\n";
        return;
    }
    ret = user_.login(cmd_->arg('u'), cmd_->arg('p'));
    std::cout << ret << '\n';
}

void TicketSystem::logout() {
    // std::cout << "logout\n";
    int ret = 0;
    if (!verify_username(cmd_->arg('u'))) {
        std::cout << "-1\n";
        return;
    }
    ret = user_.logout(cmd_->arg('u'));
    std::cout << ret << '\n';
}

void TicketSystem::query_profile() {
    // std::cout << "query_profile\n";
    if (!verify_username(cmd_->arg('c')) || !verify_username(cmd_->arg('u'))) {
        std::cout << "-1\n";
        return;
    }
    auto profile = user_.query_profile(cmd_->arg('c'), cmd_->arg('u'));
    if (profile == std::nullopt) {
        std::cout << "-1\n";
    }
    else {
        std::cout << profile->username() << " " << profile->name() << " " << profile->email() << " " << profile->privilege() << "\n";
    }
}

void TicketSystem::modify_profile() {
    std::cout << "modify_profile\n";
}

void TicketSystem::add_train() {
    std::cout << "add_train\n";
}

void TicketSystem::delete_train() {
    std::cout << "delete_train\n";
}

void TicketSystem::release_train() {
    std::cout << "release_train\n";
}

void TicketSystem::query_train() {
    std::cout << "query_train\n";
}

void TicketSystem::query_ticket() {
    std::cout << "query_ticket\n";
}

void TicketSystem::query_transfer() {
    std::cout << "query_transfer\n";
}

void TicketSystem::buy_ticket() {
    std::cout << "buy_ticket\n";
}

void TicketSystem::query_order() {
    std::cout << "query_order\n";
}

void TicketSystem::refund_ticket() {
    std::cout << "refund_ticket\n";
}

void TicketSystem::clear() {
    // std::cout << "clear\n";
    user_.clear();
    train_.clear();
    order_.clear();
    timestamp_ = 0;
    if (cmd_) {
        delete cmd_;
        cmd_ = nullptr;
    }
    init_ = false;
}

} // namespace sjtu