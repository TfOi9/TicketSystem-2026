#include "../../include/system/ticket.hpp"
#include "../../include/utils/validator.hpp"
#include "../../include/command/command.hpp"
#include "../../include/system/order.hpp"
#include "../../include/utils/fixed_string.hpp"
#include "../../include/result/result.hpp"
#include <memory>
#include <optional>
#include <sstream>

namespace sjtu {

CompleteTicket TicketSystem::complete(const Ticket& ticket) {
    CompleteTicket t {
        .train_id_ = ticket.train_id_,
        .start_station_ = FixedString<40>(train_.station_name(ticket.start_station_)),
        .end_station_ = FixedString<40>(train_.station_name(ticket.end_station_)),
        .departure_date_ = ticket.departure_date_,
        .departure_time_ = ticket.departure_time_,
        .arrival_date_ = ticket.arrival_date_,
        .arrival_time_ = ticket.arrival_time_,
        .duration_ = ticket.duration_,
        .price_ = ticket.price_,
        .seat_ = ticket.seat_
    };
    return t;
}

CompleteOrder TicketSystem::complete(const Order& order) {
    CompleteOrder o {
        .info_ = order.info_,
        .status_ = order.status_,
        .ticket_ = complete(order.ticket_)
    };
    return o;
}

CompleteTransferTicket TicketSystem::complete(const TransferTicket& transfer) {
    CompleteTransferTicket t {
        .first_ticket_ = complete(transfer.first_ticket_),
        .second_ticket_ = complete(transfer.second_ticket_),
        .price_ = transfer.price(),
        .duration_ = transfer.duration()
    };
    return t;
}

TicketSystem::~TicketSystem() {
    // std::cerr << "ots = " << order_timestamp_ << std::endl;
    timestamp_file_.seekp(0, std::ios::beg);
    timestamp_file_.write(reinterpret_cast<char *>(&order_timestamp_), sizeof(int));
    if (timestamp_file_.is_open()) {
        timestamp_file_.close();
    }
    if (cmd_) {
        delete cmd_;
        cmd_ = nullptr;
    }
}

void TicketSystem::run(const volatile std::sig_atomic_t* signal_status) {
    while (true) {
        if (signal_status && *signal_status != 0) {
            flush();
            return;
        }
        std::string line;
        if (!std::getline(std::cin, line)) {
            if (signal_status && *signal_status != 0) {
                flush();
                return;
            }
            if (std::cin.eof()) {
                return;
            }
            std::cin.clear();
            continue;
        }
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
                // std::cerr << "bad check\n";
                std::cout << "-1\n";
            }
            else {
                // std::cerr << "modify\n";
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
        else {
            std::cout << "-1\n";
        }
    }
}

std::unique_ptr<Result> TicketSystem::handle(const Command &command) {
    if (cmd_) delete cmd_;
    cmd_ = new Command(command);
    Result *res = nullptr;
    std::string cmd = cmd_->cmd();
    if (cmd == "add_user") {
        if (!cmd_->check("cupnmg", "")) {
            res = new FailureResult();
        }
        else {
            add_user(true, &res);
        }
    }
    else if (cmd == "login") {
        if (!cmd_->check("up", "")) {
            res = new FailureResult();
        }
        else {
            login(true, &res);
        }
    }
    else if (cmd == "logout") {
        if (!cmd_->check("u", "")) {
            res = new FailureResult();
        }
        else {
            logout(true, &res);
        }
    }
    else if (cmd == "query_profile") {
        if (!cmd_->check("cu", "")) {
            res = new FailureResult();
        }
        else {
            query_profile(true, &res);
        }
    }
    else if (cmd == "modify_profile") {
        if (!cmd_->check("cu", "pnmg")) {
            res = new FailureResult();
        }
        else {
            modify_profile(true, &res);
        }
    }
    else if (cmd == "add_train") {
        if (!cmd_->check("inmspxtody", "")) {
            res = new FailureResult();
        }
        else {
            add_train(true, &res);
        }
    }
    else if (cmd == "delete_train") {
        if (!cmd_->check("i", "")) {
            res = new FailureResult();
        }
        else {
            delete_train(true, &res);
        }
    }
    else if (cmd == "release_train") {
        if (!cmd_->check("i", "")) {
            res = new FailureResult();
        }
        else {
            release_train(true, &res);
        }
    }
    else if (cmd == "query_train") {
        if (!cmd_->check("id", "")) {
            res = new FailureResult();
        }
        else {
            query_train(true, &res);
        }
    }
    else if (cmd == "query_ticket") {
        if (!cmd_->check("std", "p")) {
            res = new FailureResult();
        }
        else {
            query_ticket(true, &res);
        }
    }
    else if (cmd == "query_transfer") {
        if (!cmd_->check("std", "p")) {
            res = new FailureResult();
        }
        else {
            query_transfer(true, &res);
        }
    }
    else if (cmd == "buy_ticket") {
        if (!cmd_->check("uidnft", "q")) {
            res = new FailureResult();
        }
        else {
            buy_ticket(true, &res);
        }
    }
    else if (cmd == "query_order") {
        if (!cmd_->check("u", "")) {
            res = new FailureResult();
        }
        else {
            query_order(true, &res);
        }
    }
    else if (cmd == "refund_ticket") {
        if (!cmd_->check("u", "n")) {
            res = new FailureResult();
        }
        else {
            refund_ticket(true, &res);
        }
    }
    else if (cmd == "clean") {
        if (!cmd_->check("", "")) {
            res = new FailureResult();
        }
        else {
            clear();
            res = new SuccessResult();
        }
    }
    else {
        res = new FailureResult();
    }

    return std::unique_ptr<Result>(res);
}

bool TicketSystem::bootstrapRootSession() {
    const std::string username = "root";
    const std::string password = "sjtu";
    const std::string name = "管理员";
    const std::string email = "yyu@apex.sjtu.edu.cn";

    if (user_.empty()) {
        if (user_.add_user("", username, password, name, email, 10) != 0) {
            return false;
        }
    }

    if (!user_.logged_in(username)) {
        if (user_.login(username, password) != 0) {
            return false;
        }
    }

    return true;
}

void TicketSystem::flush() {
    timestamp_file_.seekp(0, std::ios::beg);
    timestamp_file_.write(reinterpret_cast<char *>(&order_timestamp_), sizeof(int));
    user_.flush();
    train_.flush();
    order_.flush();
}

void TicketSystem::add_user(bool pack, Result **res) {
    // std::cout << "add_user\n";
    int ret = 0;
    if (!verify_username(cmd_->arg('u')) || !verify_password(cmd_->arg('p'))
        || !verify_chinese_name(cmd_->arg('n')) || !verify_email(cmd_->arg('m'))) {
        if (pack) {
            std::cerr << "bad args\n";
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    if (user_.empty()) {
        std::cerr << "user emp\n";
        ret = user_.add_user("", cmd_->arg('u'), cmd_->arg('p'), cmd_->arg('n'), cmd_->arg('m'), 10);
    }
    else {
        int g = verify_privilege(cmd_->arg('g'));
        if (g == -1 || !verify_username(cmd_->arg('c'))) {
            std::cerr << "bad g\n";
            if (pack) {
                if (res) *res = new FailureResult();
                return;
            }
            std::cout << "-1\n";
            return;
        }
        // std::cerr << "add_user " << g << std::endl;
        ret = user_.add_user(cmd_->arg('c'), cmd_->arg('u'), cmd_->arg('p'), cmd_->arg('n'), cmd_->arg('m'), g);
    }
    if (pack) {
        std::cerr << "packed\n";
        if (!ret) {
            std::cerr << "success\n";
            if (res) *res = new SuccessResult();
        }
        else {
            if (res) *res = new FailureResult();
        }
        return;
    }
    std::cout << ret << '\n';
}

void TicketSystem::login(bool pack, Result **res) {
    // std::cout << "login\n";
    int ret = 0;
    if (!verify_username(cmd_->arg('u')) || !verify_password(cmd_->arg('p'))) {
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    ret = user_.login(cmd_->arg('u'), cmd_->arg('p'));
    if (pack) {
            if (!ret) {
                if (res) *res = new SuccessResult();
            }
            else {
                if (res) *res = new FailureResult();
            }
        return;
    }
    std::cout << ret << '\n';
}

void TicketSystem::logout(bool pack, Result **res) {
    // std::cout << "logout\n";
    int ret = 0;
    if (!verify_username(cmd_->arg('u'))) {
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    ret = user_.logout(cmd_->arg('u'));
    if (pack) {
            if (!ret) {
                if (res) *res = new SuccessResult();
            }
            else {
                if (res) *res = new FailureResult();
            }
        return;
    }
    std::cout << ret << '\n';
}

void TicketSystem::query_profile(bool pack, Result **res) {
    // std::cout << "query_profile\n";
    if (!verify_username(cmd_->arg('c')) || !verify_username(cmd_->arg('u'))) {
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    auto profile = user_.query_profile(cmd_->arg('c'), cmd_->arg('u'));
    if (profile == std::nullopt) {
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
    }
    else {
        if (pack) {
            if (res) *res = new ProfileResult(profile->username(), profile->name(), profile->email(), profile->privilege());
            return;
        }
        std::cout << profile->username() << " " << profile->name() << " " << profile->email() << " " << profile->privilege() << "\n";
    }
}

void TicketSystem::modify_profile(bool pack, Result **res) {
    // std::cerr << "modify_profile\n";
    // std::cerr << cmd_->arg('c') << " " << cmd_->arg('u') << std::endl;
    if (!verify_username(cmd_->arg('c')) || !verify_username(cmd_->arg('u'))) {
        // std::cerr << "bad username\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    if (!cmd_->arg('p').empty() && !verify_password(cmd_->arg('p'))) {
        // std::cerr << "bad pwd\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    if (!cmd_->arg('n').empty() && !verify_chinese_name(cmd_->arg('n'))) {
        // std::cerr << "bad name\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    if (!cmd_->arg('m').empty() && !verify_email(cmd_->arg('m'))) {
        // std::cerr << "bad email\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    int g = -1;
    if (!cmd_->arg('g').empty() && (g = verify_privilege(cmd_->arg('g'))) == -1) {
        // std::cerr << "bad g\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    // std::cerr << "g = " << g << std::endl;
    auto profile = user_.modify_profile(cmd_->arg('c'), cmd_->arg('u'), cmd_->arg('p'),
        cmd_->arg('n'), cmd_->arg('m'), g);
    // std::cerr << "test\n";
    if (profile == std::nullopt) {
        // std::cerr << "bad modify\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
    }
    else {
        if (pack) {
            if (res) *res = new ProfileResult(profile->username(), profile->name(), profile->email(), profile->privilege());
            return;
        }
        std::cout << profile->username() << " " << profile->name() << " " << profile->email() << " " << profile->privilege() << "\n";
    }
}

void TicketSystem::add_train(bool pack, Result **res) {
    // std::cout << "add_train\n";
    Train train;
    if (!verify_train_name(cmd_->arg('i'))) {
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    train.trainID_ = FixedString<20>(cmd_->arg('i'));
    try {
        train.stationNum_ = sjtu::stoi(cmd_->arg('n'));
    }
    catch(...) {
        // std::cerr << "bad name\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    if (train.stationNum_ < 2 || train.stationNum_ > 100) {
        // std::cerr << "bad station num\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    try {
        train.seatNum_ = sjtu::stoi(cmd_->arg('m'));
    }
    catch(...) {
        // std::cerr << "bad seat num\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    if (train.seatNum_ > 100000) {
        // std::cerr << "too many seats\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    sjtu::vector<std::string> stations = separate_by_pipe(cmd_->arg('s'));
    if (stations.size() != train.stationNum_) {
        // std::cerr << "bad station str\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    for (int i = 0; i < train.stationNum_; i++) {
        std::string str = stations[i];
        if (!verify_station_name(str)) {
            // std::cerr << "some bad station name\n";
            if (pack) {
                if (res) *res = new FailureResult();
                return;
            }
            std::cout << "-1\n";
            return;
        }
        int station_id = train_.add_station(str);
        train.stations_[i] = station_id;
    }
    sjtu::vector<std::string> prices = separate_by_pipe(cmd_->arg('p'));
    if (prices.size() != train.stationNum_ - 1) {
        // std::cerr << "bad price str\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    try {
        for (int i = 0; i < train.stationNum_ - 1; i++) {
            train.prices_[i] = sjtu::stoi(prices[i]);
        }
    }
    catch(...) {
        // std::cerr << "some bad prices\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    try {
        train.startTime_ = parse_time(cmd_->arg('x'));
    }
    catch(...) {
        // std::cerr << "bad time\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    sjtu::vector<std::string> travelTimes = separate_by_pipe(cmd_->arg('t'));
    if (travelTimes.size() != train.stationNum_ - 1) {
        // std::cerr << "bad travel time str\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    try {
        for (int i = 0; i < train.stationNum_ - 1; i++) {
            train.travelTimes_[i] = sjtu::stoi(travelTimes[i]);
        }
    }
    catch(...) {
        // std::cerr << "some bad travel times\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    if (train.stationNum_ == 2) {
        if (cmd_->arg('o') != "_") {
            // std::cerr << "o_\n";
            if (pack) {
                if (res) *res = new FailureResult();
                return;
            }
            std::cout << "-1\n";
            return;
        }
    }
    else {
        sjtu::vector<std::string> stopoverTimes = separate_by_pipe(cmd_->arg('o'));
        if (stopoverTimes.size() != train.stationNum_ - 2) {
            // std::cerr << "bad stopover times str\n";
            if (pack) {
                if (res) *res = new FailureResult();
                return;
            }
            std::cout << "-1\n";
            return;
        }
        try {
            train.stopoverTimes_[0] = 0;
            for (int i = 0; i < train.stationNum_ - 2; i++) {
                train.stopoverTimes_[i + 1] = sjtu::stoi(stopoverTimes[i]);
            }
        }
        catch(...) {
            // std::cerr << "some bad stopover times\n";
            if (pack) {
                if (res) *res = new FailureResult();
                return;
            }
            std::cout << "-1\n";
            return;
        }
    }
    if (cmd_->arg('d').size() != 11 || cmd_->arg('d')[5] != '|') {
        // std::cerr << "bad date syntax\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    try {
        train.startSaleDate_ = parse_date(cmd_->arg('d').substr(0, 5));
        train.endSaleDate_ = parse_date(cmd_->arg('d').substr(6, 5));
    }
    catch(...) {
        // std::cerr << "bad date\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    if (train.startSaleDate_.month_ < 6 || train.startSaleDate_.month_ > 8 ||
            train.endSaleDate_.month_ < 6 || train.endSaleDate_.month_ > 8) {
        // std::cerr << "bad month\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    std::string y = cmd_->arg('y');
    if (y.size() != 1 || y[0] < 'A' || y[0] > 'Z') {
        // std::cerr << "bad type\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    train.type_ = y[0];
    train.released_ = false;
    train.arrivalTimes_[0] = train.startTime_;
    for (int i = 1; i < train.stationNum_; i++) {
        train.arrivalTimes_[i] = train.arrivalTimes_[i - 1] + (train.travelTimes_[i - 1] + train.stopoverTimes_[i - 1]);
    }
    for (int i = int(train.startSaleDate_); i <= int(train.endSaleDate_); i++) {
        for (int j = 0; j < train.stationNum_ - 1; j++) {
            train.seats_[i][j] = train.seatNum_;
        }
    }
    int ret = train_.add_train(train);
    if (pack) {
            if (!ret) {
                if (res) *res = new SuccessResult();
            }
            else {
                if (res) *res = new FailureResult();
            }
        return;
    }
    std::cout << ret << "\n";
}

void TicketSystem::delete_train(bool pack, Result **res) {
    // std::cout << "delete_train\n";
    if (!verify_train_name(cmd_->arg('i'))) {
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    int ret = train_.delete_train(cmd_->arg('i'));
    if (pack) {
            if (!ret) {
                if (res) *res = new SuccessResult();
            }
            else {
                if (res) *res = new FailureResult();
            }
        return;
    }
    std::cout << ret << "\n";
}

void TicketSystem::release_train(bool pack, Result **res) {
    // std::cout << "release_train\n";
    if (!verify_train_name(cmd_->arg('i'))) {
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    int ret = train_.release_train(cmd_->arg('i'));
    if (pack) {
            if (!ret) {
                if (res) *res = new SuccessResult();
            }
            else {
                if (res) *res = new FailureResult();
            }
        return;
    }
    std::cout << ret << "\n";
}

void TicketSystem::query_train(bool pack, Result **res) {
    // std::cout << "query_train\n";
    if (!verify_train_name(cmd_->arg('i'))) {
        // std::cerr << "bad train name\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    date d;
    try {
        d = parse_date(cmd_->arg('d'));
    }
    catch(...) {
        // std::cerr << "bad date syntax\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    if (d.month_ < 6 || d.month_ > 8) {
        // std::cerr << "bad month\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    auto ress = train_.query_train(cmd_->arg('i'));
    if (!ress.has_value()) {
        // std::cerr << "train not found\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    Train train = ress.value();
    if (int(d) < int(train.startSaleDate_) || int(d) > int(train.endSaleDate_)) {
        // std::cerr << "no train at that date\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    if (pack) {
        TrainInfo info{};
        info.train_id_ = train.trainID_;
        info.type_ = train.type_;
        info.station_num_ = train.stationNum_;
        info.query_date_ = d;
        int price = 0;
        for (int i = 0; i < train.stationNum_; i++) {
            TrainStationInfo station{};
            station.station_name_ = FixedString<40>(train_.station_name(train.stations_[i]));
            station.arrival_time_ = train.arrivalTimes_[i];
            station.leaving_time_ = train.arrivalTimes_[i] + train.stopoverTimes_[i];
            station.price_ = price;
            station.seat_ = (i < train.stationNum_ - 1) ? train.seats_[int(d)][i] : -1;
            station.has_arrival_ = (i != 0);
            station.has_leaving_ = (i < train.stationNum_ - 1);
            info.stations_[i] = station;
            price += train.prices_[i];
        }
        if (res) *res = new TrainResult(info);
        return;
    }
    std::cout << train.trainID_.str() << " " << train.type_ << "\n";
    int price = 0;
    for (int i = 0; i < train.stationNum_; i++) {
        std::string station_name = train_.station_name(train.stations_[i]);
        time arrival_time = train.arrivalTimes_[i];
        time leaving_time = train.arrivalTimes_[i] + train.stopoverTimes_[i];
        std::cout << station_name << " ";
        if (i) print_time_date(d, arrival_time, std::cout);
        else std::cout << "xx-xx xx:xx";
        std::cout << " -> ";
        if (i < train.stationNum_ - 1) print_time_date(d, leaving_time, std::cout);
        else std::cout << "xx-xx xx:xx";
        std::cout << " " << price << " ";
        if (i < train.stationNum_ - 1) std::cout << train.seats_[int(d)][i];
        else std::cout << "x";
        std::cout << "\n";
        price += train.prices_[i];
    }
}

void TicketSystem::query_ticket(bool pack, Result **res) {
    // std::cout << "query_ticket\n";
    if (!verify_station_name(cmd_->arg('s')) || !verify_station_name(cmd_->arg('t'))) {
        // std::cerr << "bad station name\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "0\n";
        return;
    }
    if (cmd_->arg('s') == cmd_->arg('t')) {
        // std::cerr << "same station\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "0\n";
        return;
    }
    date d;
    try {
        d = parse_date(cmd_->arg('d'));
    }
    catch(...) {
        // std::cerr << "bad date syntax\n";
        if (pack) {
            if (res) *res = new FailureResult();
        }
        std::cout << "0\n";
        return;
    }
    if (d.month_ < 6 || d.month_ > 9 || d.month_ == 9 && d.day_ > 3) {
        // std::cerr << "no train at date\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "0\n";
        return;
    }
    if (!cmd_->arg('p').empty() && cmd_->arg('p') != "time" && cmd_->arg('p') != "cost") {
        // std::cerr << "bad sorting protocol\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "0\n";
        return;
    }
    sjtu::vector<TrainPosition> start_trains, end_trains;
    int start_query = train_.query_station(cmd_->arg('s'), start_trains);
    int end_query = train_.query_station(cmd_->arg('t'), end_trains);
    if (start_query || end_query) {
        // std::cerr << "bad query\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "0\n";
        return;
    }
    start_trains.sort(TrainPositionCompare());
    end_trains.sort(TrainPositionCompare());
    int start_ptr = 0, end_ptr = 0;
    int d_val = int(d);
    sjtu::vector<Ticket> tickets;
    while (start_ptr < start_trains.size()) {
        while (end_ptr < end_trains.size() && end_trains[end_ptr].train_id_ < start_trains[start_ptr].train_id_) {
            end_ptr++;
        }
        if (end_ptr < end_trains.size() && end_trains[end_ptr].train_id_ == start_trains[start_ptr].train_id_ &&
            start_trains[start_ptr].pos_ < end_trains[end_ptr].pos_) {
            int train_id = start_trains[start_ptr].train_id_, 
                spos = start_trains[start_ptr].pos_, 
                epos = end_trains[end_ptr].pos_;
            Train train = train_.query_train(train_id);
            date first_date = train.startSaleDate_ + (train.arrivalTimes_[spos] + train.stopoverTimes_[spos]).day_offset_;
            date last_date = train.endSaleDate_ + (train.arrivalTimes_[spos] + train.stopoverTimes_[spos]).day_offset_;
            if (d_val >= int(first_date) && d_val <= int(last_date)) {
                int depart = d_val - (train.arrivalTimes_[spos] + train.stopoverTimes_[spos]).day_offset_;
                int min_seats = train.seatNum_;
                for (int i = spos; i < epos; i++) {
                    if (train.seats_[depart][i] < min_seats) {
                        min_seats = train.seats_[depart][i];
                    }
                }
                Ticket ticket;
                ticket.train_id_ = train.trainID_;
                // // std::cerr << train.trainID_ << std::endl;
                ticket.start_station_ = train.stations_[spos];
                ticket.end_station_ = train.stations_[epos];
                ticket.departure_date_ = d_val;
                // // std::cerr << train.arrivalTimes_[spos].day_offset_ << std::endl;
                // // std::cerr << d_val << " " << depart << std::endl;
                // // std::cerr << date(d_val).month_ << " " << date(d_val).day_ << std::endl;
                ticket.arrival_date_ = d_val
                    + train.arrivalTimes_[epos].day_offset_
                    - (train.arrivalTimes_[spos] + train.stopoverTimes_[spos]).day_offset_;
                ticket.departure_time_ = train.arrivalTimes_[spos] + train.stopoverTimes_[spos];
                ticket.arrival_time_ = train.arrivalTimes_[epos];
                ticket.duration_ = int(ticket.arrival_time_) - int(ticket.departure_time_);
                int price = 0;
                for (int i = spos; i < epos; i++) {
                    price += train.prices_[i];
                }
                ticket.price_ = price;
                ticket.seat_ = min_seats;
                tickets.push_back(ticket);
            }
        }
        start_ptr++;
    }
    if (cmd_->arg('p') == "cost") {
        tickets.sort(TicketPriceCompare());
    }
    else {
        tickets.sort(TicketDurationCompare());
    }
    if (pack) {
        sjtu::vector<CompleteTicket> packed_tickets;
        for (int i = 0; i < tickets.size(); i++) {
            packed_tickets.push_back(complete(tickets[i]));
        }
        if (res) *res = new TicketResult(packed_tickets);
        return;
    }
    std::cout << tickets.size() << "\n";
    for (int i = 0; i < tickets.size(); i++) {
        Ticket& ticket = tickets[i];
        std::cout << ticket.train_id_ << " " << cmd_->arg('s') << " ";
        print_time_date(ticket.departure_date_, ticket.departure_time_, std::cout, true);
        std::cout << " -> " << cmd_->arg('t') << " ";
        print_time_date(ticket.arrival_date_, ticket.arrival_time_, std::cout, true);
        std::cout << " " << ticket.price_ << " " << ticket.seat_ << "\n";
    }
}

void TicketSystem::query_transfer(bool pack, Result **res) {
    // std::cout << "query_transfer\n";
    if (!verify_station_name(cmd_->arg('s')) || !verify_station_name(cmd_->arg('t'))) {
        // std::cerr << "bad station name\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "0\n";
        return;
    }
    if (cmd_->arg('s') == cmd_->arg('t')) {
        // std::cerr << "same station\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "0\n";
        return;
    }
    date d;
    try {
        d = parse_date(cmd_->arg('d'));
    }
    catch(...) {
        // std::cerr << "bad date syntax\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "0\n";
        return;
    }
    if (d.month_ < 6 || d.month_ > 9 || d.month_ == 9 && d.day_ > 3) {
        // std::cerr << "no train at date\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "0\n";
        return;
    }
    if (!cmd_->arg('p').empty() && cmd_->arg('p') != "time" && cmd_->arg('p') != "cost") {
        // std::cerr << "bad sorting protocol\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "0\n";
        return;
    }
    sjtu::vector<TrainPosition> start_trains, end_trains;
    int start_query = train_.query_station(cmd_->arg('s'), start_trains);
    int end_query = train_.query_station(cmd_->arg('t'), end_trains);
    int start_station = train_.station_id(cmd_->arg('s'));
    int end_station = train_.station_id(cmd_->arg('t'));
    if (start_query || end_query) {
        // std::cerr << "bad query\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "0\n";
        return;
    }
    int d_val = int(d);
    sjtu::vector<Ticket> candidate_tickets;
    for (int i = 0; i < start_trains.size(); i++) {
        Train train = train_.query_train(start_trains[i].train_id_);
        int start_pos = start_trains[i].pos_;
        date first_date = train.startSaleDate_ + (train.arrivalTimes_[start_pos] + train.stopoverTimes_[start_pos]).day_offset_;
        date last_date = train.endSaleDate_ + (train.arrivalTimes_[start_pos] + train.stopoverTimes_[start_pos]).day_offset_;
        if (d_val < int(first_date) || d_val > int(last_date)) {
            continue;
        }
        int price = 0;
        int min_seats = train.seatNum_;
        int departure_d = d_val - (train.arrivalTimes_[start_pos] + train.stopoverTimes_[start_pos]).day_offset_;
        for (int j = start_pos + 1; j < train.stationNum_; j++) {
            price += train.prices_[j - 1];
            int cur_seat = train.seats_[departure_d][j - 1];
            if (cur_seat < min_seats) {
                min_seats = cur_seat;
            }
            if (train.stations_[j] == end_station) {
                continue;
            }
            Ticket ticket;
            ticket.train_id_ = train.trainID_;
            ticket.start_station_ = start_station;
            ticket.end_station_ = train.stations_[j];
            ticket.departure_date_ = date(d_val);
            ticket.departure_time_ = train.arrivalTimes_[start_pos] + train.stopoverTimes_[start_pos];
            ticket.arrival_date_ = date(d_val
                + train.arrivalTimes_[j].day_offset_
                - (train.arrivalTimes_[start_pos] + train.stopoverTimes_[start_pos]).day_offset_);
            ticket.arrival_time_ = train.arrivalTimes_[j];
            ticket.duration_ = int(ticket.arrival_time_) - int(ticket.departure_time_);
            ticket.price_ = price;
            ticket.seat_ = min_seats;
            candidate_tickets.push_back(ticket);
        }
    }
    candidate_tickets.sort(TicketEndStationCompare());
    // // std::cerr << candidate_tickets.size() << std::endl;
    sjtu::vector<TransferTicket> tickets;
    for (int i = 0; i < end_trains.size(); i++) {
        Train train = train_.query_train(end_trains[i].train_id_);
        int price = 0;
        for (int j = end_trains[i].pos_ - 1; j >= 0; j--) {
            price += train.prices_[j];
            Ticket t{};
            t.end_station_ = train.stations_[j];
            time real_departure_time = train.arrivalTimes_[j] + (j ? train.stopoverTimes_[j] : 0);
            time departure_time = real_departure_time;
            departure_time.day_offset_ = 0;
            auto it = candidate_tickets.lower_bound(t, TicketEndStationCompare());
            while (it != candidate_tickets.end() && (*it).end_station_ == train.stations_[j]) {
                if ((*it).train_id_ == train.trainID_) {
                    it++;
                    continue;
                }
                time last_arrival_time = (*it).arrival_time_;
                last_arrival_time.day_offset_ = 0;
                int last_arrival_date = int((*it).arrival_date_);
                int earliest_departure_date =
                    (int(last_arrival_time) <= int(departure_time)) ? last_arrival_date : (last_arrival_date + 1);
                int station_day_offset = (train.arrivalTimes_[j] + train.stopoverTimes_[j]).day_offset_;
                int second_train_depart_date = earliest_departure_date - station_day_offset;
                if (second_train_depart_date < int(train.startSaleDate_)) {
                    second_train_depart_date = int(train.startSaleDate_);
                }
                if (second_train_depart_date > int(train.endSaleDate_)) {
                    // // std::cerr << train.trainID_ << " " << second_train_depart_date << std::endl;
                    it++;
                    continue;
                }
                int departure_date = second_train_depart_date + station_day_offset;
                int min_seats = train.seatNum_;
                for (int k = j; k < end_trains[i].pos_; k++) {
                    int cur_seat = train.seats_[second_train_depart_date][k];
                    if (cur_seat < min_seats) {
                        min_seats = cur_seat;
                    }
                }
                Ticket ticket;
                ticket.train_id_ = train.trainID_;
                ticket.start_station_ = train.stations_[j];
                ticket.end_station_ = end_station;
                ticket.departure_date_ = date(departure_date);
                ticket.departure_time_ = real_departure_time;
                ticket.arrival_date_ = date(departure_date
                    + train.arrivalTimes_[end_trains[i].pos_].day_offset_
                    - (train.arrivalTimes_[j] + train.stopoverTimes_[j]).day_offset_);
                ticket.arrival_time_ = train.arrivalTimes_[end_trains[i].pos_];
                ticket.duration_ = int(ticket.arrival_time_) - int(ticket.departure_time_);
                ticket.price_ = price;
                ticket.seat_ = min_seats;
                TransferTicket transfer_ticket;
                transfer_ticket.first_ticket_ = *it;
                transfer_ticket.second_ticket_ = ticket;
                tickets.push_back(transfer_ticket);
                it++;
            }
        }
    }
    if (tickets.empty()) {
        if (pack) {
            if (res) *res = new TransferResult(sjtu::vector<CompleteTransferTicket>());
            return;
        }
        std::cout << "0\n";
        return;
    }
    bool by_cost = (cmd_->arg('p') == "cost");
    if (by_cost) {
        tickets.sort(TransferTicketPriceCompare());
    }
    else {
        tickets.sort(TransferTicketDurationCompare());
    }
    if (pack) {
        sjtu::vector<CompleteTransferTicket> packed_transfers;
        packed_transfers.push_back(complete(tickets[0]));
        if (res) *res = new TransferResult(packed_transfers);
        return;
    }
    auto print_ticket = [&](std::ostream& os, const Ticket& ticket) {
        os << ticket.train_id_ << " " << train_.station_name(ticket.start_station_) << " ";
        print_time_date(ticket.departure_date_, ticket.departure_time_, std::cout, true);
        os << " -> " << train_.station_name(ticket.end_station_) << " ";
        print_time_date(ticket.arrival_date_, ticket.arrival_time_, std::cout, true);
        os << " " << ticket.price_ << " " << ticket.seat_ << "\n";
    };
    // for (int i = 0; i < tickets.size(); i++) {
    //     // std::cerr << "[transfer_sorted " << i << "] ";
    //     print_ticket(// std::cerr, tickets[i].first_ticket_);
    //     // std::cerr << "[transfer_sorted " << i << "] ";
    //     print_ticket(// std::cerr, tickets[i].second_ticket_);
    // }
    print_ticket(std::cout, tickets[0].first_ticket_);
    print_ticket(std::cout, tickets[0].second_ticket_);
}

void TicketSystem::buy_ticket(bool pack, Result **res) {
    // std::cout << "buy_ticket\n";
    if (!verify_username(cmd_->arg('u'))) {
        // std::cerr << "bad user name\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    if (!user_.logged_in(cmd_->arg('u'))) {
        // std::cerr << "user not logged in\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    if (!verify_train_name(cmd_->arg('i'))) {
        // std::cerr << "bad train name\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    auto ans = train_.query_train(cmd_->arg('i'));
    if (ans == std::nullopt) {
        // std::cerr << "train not found\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    Train& train = ans.value();
    if (!train.released_) {
        // std::cerr << "train not released\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    date d;
    try {
        d = parse_date(cmd_->arg('d'));
    }
    catch(...) {
        // std::cerr << "bad date\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    std::string from = cmd_->arg('f');
    std::string to = cmd_->arg('t');
    if (!verify_station_name(from) || !verify_station_name(to)) {
        // std::cerr << "bad station name\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    int n;
    try {
        n = sjtu::stoi(cmd_->arg('n'));
    }
    catch(...) {
        // std::cerr << "bad number\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    if (n <= 0) {
        // std::cerr << "n not positive\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    bool accept_queue;
    if (cmd_->arg('q') == "true") {
        accept_queue = true;
    }
    else if (cmd_->arg('q') == "false" || cmd_->arg('q').empty()) {
        accept_queue = false;
    }
    else {
        // std::cerr << "bad queue acceptance\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    int from_id = train_.station_id(from);
    int to_id = train_.station_id(to);
    if (from_id == -1 || to_id == -1) {
        // std::cerr << "station not found\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    int spos = -1, epos = -1;
    for (int i = 0; i < train.stationNum_; i++) {
        if (train.stations_[i] == from_id) {
            spos = i;
        }
        if (train.stations_[i] == to_id) {
            epos = i;
        }
    }
    if (spos == -1 || epos == -1 || spos >= epos) {
        // std::cerr << "station not on train or bad sequence\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    int departure_date = int(d) - (train.arrivalTimes_[spos] + train.stopoverTimes_[spos]).day_offset_;
    if (departure_date < int(train.startSaleDate_) || departure_date > int(train.endSaleDate_)) {
        // std::cerr << "outside selling period\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    int min_seats = train.seatNum_, price = 0;
    for (int i = spos; i < epos; i++) {
        int cur_seat = train.seats_[departure_date][i];
        if (cur_seat < min_seats) {
            min_seats = cur_seat;
        }
        price += train.prices_[i];
    }
    if (min_seats < n) {
        if (accept_queue && n <= train.seatNum_) {
            int total_price = price * n;
            OrderInfo order_info{FixedString<20>(cmd_->arg('u')), order_timestamp_};
            Ticket ticket{FixedString<20>(cmd_->arg('i')), from_id, to_id,
                d, train.arrivalTimes_[spos] + train.stopoverTimes_[spos],
                d + (train.arrivalTimes_[epos].day_offset_
                    - (train.arrivalTimes_[spos] + train.stopoverTimes_[spos]).day_offset_),
                train.arrivalTimes_[epos],
                int(train.arrivalTimes_[epos]) - int(train.arrivalTimes_[spos] + train.stopoverTimes_[spos]),
                price, n
            };
            Order order{order_info, TicketStatus::Pending, ticket};
            order_.add_order(order);
            order_.add_pending_order(order);
            order_timestamp_++;
            if (pack) {
                if (res) *res = new SuccessResult();
                return;
            }
            std::cout << "queue\n";
        }
        else {
            // std::cerr << "ticket not enough\n";
            if (pack) {
                if (res) *res = new FailureResult();
                return;
            }
            std::cout << "-1\n";
            return;
        }
    }
    else {
        for (int i = spos; i < epos; i++) {
            train.seats_[departure_date][i] -= n;
        }
        train_.update_train(train_.train_id(cmd_->arg('i')), train);
        int total_price = price * n;
        OrderInfo order_info{FixedString<20>(cmd_->arg('u')), order_timestamp_};
        Ticket ticket{FixedString<20>(cmd_->arg('i')), from_id, to_id,
            d, train.arrivalTimes_[spos] + train.stopoverTimes_[spos],
            d + (train.arrivalTimes_[epos].day_offset_
                - (train.arrivalTimes_[spos] + train.stopoverTimes_[spos]).day_offset_),
            train.arrivalTimes_[epos],
            int(train.arrivalTimes_[epos]) - int(train.arrivalTimes_[spos] + train.stopoverTimes_[spos]),
            price, n
        };
        Order order{order_info, TicketStatus::Purchased, ticket};
        order_.add_order(order);
        order_timestamp_++;
        if (pack) {
            if (res) *res = new SuccessResult();
            return;
        }
        std::cout << total_price << "\n";
    }
}

void TicketSystem::query_order(bool pack, Result **res) {
    // std::cout << "query_order\n";
    if (!verify_username(cmd_->arg('u'))) {
        // std::cerr << "bad username\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    if (!user_.logged_in(cmd_->arg('u'))) {
        // std::cerr << "user not logged in\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    sjtu::vector<Order> orders;
    order_.query_order(cmd_->arg('u'), orders);
    orders.sort(OrderTimeReverseCompare());
    if (pack) {
        sjtu::vector<CompleteOrder> packed_orders;
        for (int i = 0; i < orders.size(); i++) {
            packed_orders.push_back(complete(orders[i]));
        }
        if (res) *res = new OrderResult(packed_orders);
        return;
    }
    std::cout << orders.size() << "\n";
    for (int i = 0; i < orders.size(); i++) {
        Order& order = orders[i];
        switch (order.status_) {
            case TicketStatus::Pending:
                std::cout << "[pending] "; break;
            case TicketStatus::Purchased:
                std::cout << "[success] "; break;
            case TicketStatus::Refunded:
                std::cout << "[refunded] "; break;
            default:
                // std::cerr << "bad order\n";
                std::cout << "[invalid] ";
        }
        std::cout << order.ticket_.train_id_ << " ";
        std::string from = train_.station_name(order.ticket_.start_station_);
        std::string to = train_.station_name(order.ticket_.end_station_);
        std::cout << from << " ";
        print_time_date(order.ticket_.departure_date_, order.ticket_.departure_time_, std::cout, true);
        std::cout << " -> " << to << " ";
        print_time_date(order.ticket_.arrival_date_, order.ticket_.arrival_time_, std::cout, true);
        std::cout << " " << order.ticket_.price_ << " " << order.ticket_.seat_ << "\n";
        // // std::cerr << order.info_.purchase_timestamp_ << "\n";
    }
}

void TicketSystem::refund_ticket(bool pack, Result **res) {
    // std::cout << "refund_ticket\n";
    if (!verify_username(cmd_->arg('u'))) {
        // std::cerr << "bad username\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    if (!user_.logged_in(cmd_->arg('u'))) {
        // std::cerr << "user not logged in\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    int n = 1;
    if (!cmd_->arg('n').empty()) {
        try {
            n = sjtu::stoi(cmd_->arg('n'));
        }
        catch(...) {
            // std::cerr << "bad number\n";
            if (pack) {
                if (res) *res = new FailureResult();
                return;
            }
            std::cout << "-1\n";
            return;
        }
        if (n <= 0) {
            // std::cerr << "n not positive\n";
            if (pack) {
                if (res) *res = new FailureResult();
                return;
            }
            std::cout << "-1\n";
            return;
        }
    }
    sjtu::vector<Order> orders;
    order_.query_order(cmd_->arg('u'), orders);
    orders.sort(OrderTimeReverseCompare());
    if (n > orders.size()) {
        // std::cerr << "order not found\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    Order& order = orders[n - 1];
    if (order.status_ == TicketStatus::Refunded) {
        // std::cerr << "order already refunded\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    else if (order.status_ == TicketStatus::Pending) {
        order_.remove_pending_order(order.info_.purchase_timestamp_);
    }
    else if (order.status_ == TicketStatus::Purchased) {
        Train train = train_.query_train(order.ticket_.train_id_.str()).value();
        int spos = -1, epos = -1;
        for (int i = 0; i < train.stationNum_; i++) {
            if (train.stations_[i] == order.ticket_.start_station_) {
                spos = i;
            }
            if (train.stations_[i] == order.ticket_.end_station_) {
                epos = i;
            }
        }
        int departure_date = int(order.ticket_.departure_date_)
            - (train.arrivalTimes_[spos] + train.stopoverTimes_[spos]).day_offset_;
        for (int i = spos; i < epos; i++) {
            train.seats_[departure_date][i] += order.ticket_.seat_;
        }
        // train_.update_train(train_.train_id(train.trainID_.str()), train);
        sjtu::vector<Order> queue;
        order_.get_pending_queue(queue);
        queue.sort(OrderTimeCompare());
        for (int i = 0; i < queue.size(); i++) {
            Order& cur_order = queue[i];
            if (cur_order.ticket_.train_id_ != order.ticket_.train_id_) {
                continue;
            }
            int cur_spos = -1, cur_epos = -1;
            for (int j = 0; j < train.stationNum_; j++) {
                if (train.stations_[j] == cur_order.ticket_.start_station_) {
                    cur_spos = j;
                }
                if (train.stations_[j] == cur_order.ticket_.end_station_) {
                    cur_epos = j;
                }
            }
            if (cur_epos <= spos || cur_spos >= epos) {
                continue;
            }
            int cur_departure_date = int(cur_order.ticket_.departure_date_)
                - (train.arrivalTimes_[cur_spos] + train.stopoverTimes_[cur_spos]).day_offset_;
            if (departure_date != cur_departure_date) {
                continue;
            }
            int min_seats = train.seatNum_;
            for (int j = cur_spos; j < cur_epos; j++) {
                int cur_seat = train.seats_[departure_date][j];
                if (cur_seat < min_seats) {
                    min_seats = cur_seat;
                }
            }
            if (min_seats >= cur_order.ticket_.seat_) {
                for (int j = cur_spos; j < cur_epos; j++) {
                    train.seats_[departure_date][j] -= cur_order.ticket_.seat_;
                }
                order_.remove_pending_order(cur_order.info_.purchase_timestamp_);
                order_.delete_order(cur_order);
                cur_order.status_ = TicketStatus::Purchased;
                order_.add_order(cur_order);
            }
        }
        train_.update_train(train_.train_id(train.trainID_.str()), train);
    }
    else {
        // std::cerr << "unexpected invalid order\n";
        if (pack) {
            if (res) *res = new FailureResult();
            return;
        }
        std::cout << "-1\n";
        return;
    }
    order_.delete_order(order);
    order.status_ = TicketStatus::Refunded;
    order_.add_order(order);
    if (pack) {
        if (res) *res = new SuccessResult();
        return;
    }
    std::cout << "0\n";
}

void TicketSystem::clear() {
    // std::cout << "clear\n";
    user_.clear();
    train_.clear();
    order_.clear();
    timestamp_ = 0;
    order_timestamp_ = 0;
    if (timestamp_file_.is_open()) {
        timestamp_file_.close();
        timestamp_file_.open("timestamp.dat", std::ios::out | std::ios::binary);
    }
    if (cmd_) {
        delete cmd_;
        cmd_ = nullptr;
    }
}

} // namespace sjtu