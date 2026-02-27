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

void TicketSystem::flush() {
    user_.flush();
    train_.flush();
    order_.flush();
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
    // std::cout << "modify_profile\n";
    if (!verify_username(cmd_->arg('c')) || !verify_username(cmd_->arg('u'))) {
        std::cout << "-1\n";
        return;
    }
    if (!cmd_->arg('p').empty() && !verify_password(cmd_->arg('p'))) {
        std::cout << "-1\n";
        return;
    }
    if (!cmd_->arg('n').empty() && !verify_chinese_name(cmd_->arg('n'))) {
        std::cout << "-1\n";
        return;
    }
    if (!cmd_->arg('m').empty() && !verify_email(cmd_->arg('m'))) {
        std::cout << "-1\n";
        return;
    }
    int g = -1;
    if (!cmd_->arg('g').empty() && (g = verify_privilege(cmd_->arg('g'))) == -1) {
        std::cout << "-1\n";
        return;
    }
    std::cerr << g << std::endl;
    auto profile = user_.modify_profile(cmd_->arg('c'), cmd_->arg('u'), cmd_->arg('p'),
        cmd_->arg('n'), cmd_->arg('m'), g);
    if (profile == std::nullopt) {
        std::cout << "-1\n";
    }
    else {
        std::cout << profile->username() << " " << profile->name() << " " << profile->email() << " " << profile->privilege() << "\n";
    }
}

void TicketSystem::add_train() {
    // std::cout << "add_train\n";
    Train train;
    if (!verify_train_name(cmd_->arg('i'))) {
        std::cout << "-1\n";
        return;
    }
    train.trainID_ = FixedString<20>(cmd_->arg('i'));
    try {
        train.stationNum_ = sjtu::stoi(cmd_->arg('n'));
    }
    catch(...) {
        std::cerr << "bad name\n";
        std::cout << "-1\n";
        return;
    }
    if (train.stationNum_ < 2 || train.stationNum_ > 100) {
        std::cerr << "bad station num\n";
        std::cout << "-1\n";
        return;
    }
    try {
        train.seatNum_ = sjtu::stoi(cmd_->arg('m'));
    }
    catch(...) {
        std::cerr << "bad seat num\n";
        std::cout << "-1\n";
        return;
    }
    if (train.seatNum_ > 100000) {
        std::cerr << "too many seats\n";
        std::cout << "-1\n";
        return;
    }
    sjtu::vector<std::string> stations = separate_by_pipe(cmd_->arg('s'));
    if (stations.size() != train.stationNum_) {
        std::cerr << "bad station str\n";
        std::cout << "-1\n";
        return;
    }
    for (int i = 0; i < train.stationNum_; i++) {
        std::string str = stations[i];
        if (!verify_station_name(str)) {
            std::cerr << "some bad station name\n";
            std::cout << "-1\n";
            return;
        }
        int station_id = train_.add_station(str);
        train.stations_[i] = station_id;
    }
    sjtu::vector<std::string> prices = separate_by_pipe(cmd_->arg('p'));
    if (prices.size() != train.stationNum_ - 1) {
        std::cerr << "bad price str\n";
        std::cout << "-1\n";
        return;
    }
    try {
        for (int i = 0; i < train.stationNum_ - 1; i++) {
            train.prices_[i] = sjtu::stoi(prices[i]);
        }
    }
    catch(...) {
        std::cerr << "some bad prices\n";
        std::cout << "-1\n";
        return;
    }
    try {
        train.startTime_ = parse_time(cmd_->arg('x'));
    }
    catch(...) {
        std::cerr << "bad time\n";
        std::cout << "-1\n";
        return;
    }
    sjtu::vector<std::string> travelTimes = separate_by_pipe(cmd_->arg('t'));
    if (travelTimes.size() != train.stationNum_ - 1) {
        std::cerr << "bad travel time str\n";
        std::cout << "-1\n";
        return;
    }
    try {
        for (int i = 0; i < train.stationNum_ - 1; i++) {
            train.travelTimes_[i] = sjtu::stoi(travelTimes[i]);
        }
    }
    catch(...) {
        std::cerr << "some bad travel times\n";
        std::cout << "-1\n";
        return;
    }
    if (train.stationNum_ == 2) {
        if (cmd_->arg('o') != "_") {
            std::cerr << "o_\n";
            std::cout << "-1\n";
            return;
        }
    }
    else {
        sjtu::vector<std::string> stopoverTimes = separate_by_pipe(cmd_->arg('o'));
        if (stopoverTimes.size() != train.stationNum_ - 2) {
            std::cerr << "bad stopover times str\n";
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
            std::cerr << "some bad stopover times\n";
            std::cout << "-1\n";
            return;
        }
    }
    if (cmd_->arg('d').size() != 11 || cmd_->arg('d')[5] != '|') {
        std::cerr << "bad date syntax\n";
        std::cout << "-1\n";
        return;
    }
    try {
        train.startSaleDate_ = parse_date(cmd_->arg('d').substr(0, 5));
        train.endSaleDate_ = parse_date(cmd_->arg('d').substr(6, 5));
    }
    catch(...) {
        std::cerr << "bad date\n";
        std::cout << "-1\n";
        return;
    }
    if (train.startSaleDate_.month_ < 6 || train.startSaleDate_.month_ > 8 ||
            train.endSaleDate_.month_ < 6 || train.endSaleDate_.month_ > 8) {
        std::cerr << "bad month\n";
        std::cout << "-1\n";
        return;
    }
    std::string y = cmd_->arg('y');
    if (y.size() != 1 || y[0] < 'A' || y[0] > 'Z') {
        std::cerr << "bad type\n";
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
    std::cout << ret << "\n";
}

void TicketSystem::delete_train() {
    // std::cout << "delete_train\n";
    if (!verify_train_name(cmd_->arg('i'))) {
        std::cout << "-1\n";
        return;
    }
    int ret = train_.delete_train(cmd_->arg('i'));
    std::cout << ret << "\n";
}

void TicketSystem::release_train() {
    // std::cout << "release_train\n";
    if (!verify_train_name(cmd_->arg('i'))) {
        std::cout << "-1\n";
        return;
    }
    int ret = train_.release_train(cmd_->arg('i'));
    std::cout << ret << "\n";
}

void TicketSystem::query_train() {
    // std::cout << "query_train\n";
    if (!verify_train_name(cmd_->arg('i'))) {
        std::cerr << "bad train name\n";
        std::cout << "-1\n";
        return;
    }
    date d;
    try {
        d = parse_date(cmd_->arg('d'));
    }
    catch(...) {
        std::cerr << "bad date syntax\n";
        std::cout << "-1\n";
        return;
    }
    if (d.month_ < 6 || d.month_ > 8) {
        std::cerr << "bad month\n";
        std::cout << "-1\n";
        return;
    }
    auto res = train_.query_train(cmd_->arg('i'));
    if (!res.has_value()) {
        std::cerr << "train not found\n";
        std::cout << "-1\n";
        return;
    }
    Train train = res.value();
    if (int(d) < int(train.startSaleDate_) || int(d) > int(train.endSaleDate_)) {
        std::cerr << "no train at that date\n";
        std::cout << "-1\n";
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