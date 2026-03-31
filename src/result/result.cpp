#include "../../include/result/result.hpp"
#include "../../include/stl/unordered_map.hpp"
#include "../../include/system/order.hpp"

#include <cstring>
#include <unordered_map>
#include <memory>

namespace sjtu {

namespace {

sjtu::unordered_map<ResultType, Result::Deserializer>& deserializer_registry() {
    static sjtu::unordered_map<ResultType, Result::Deserializer> registry;
    return registry;
}

} // namespace

void Result::register_deserializer(ResultType type, Deserializer fn) {
    deserializer_registry()[type] = std::move(fn);
}

void Result::register_builtin_deserializers() {
    static bool initialized = false;
    if (initialized) {
        return;
    }
    initialized = true;

    register_deserializer(ResultType::Text,
        [](const char *data, uint32_t size) -> std::unique_ptr<Result> {
            return TextResult::deserialize(data, size);
        });
    register_deserializer(ResultType::Failure,
        [](const char *data, uint32_t size) -> std::unique_ptr<Result> {
            return FailureResult::deserialize(data, size);
        });
    register_deserializer(ResultType::Success,
        [](const char *data, uint32_t size) -> std::unique_ptr<Result> {
            return SuccessResult::deserialize(data, size);
        });
    register_deserializer(ResultType::Profile,
        [](const char *data, uint32_t size) -> std::unique_ptr<Result> {
            return ProfileResult::deserialize(data, size);
        });
    register_deserializer(ResultType::Order,
        [](const char *data, uint32_t size) -> std::unique_ptr<Result> {
            return OrderResult::deserialize(data, size);
        });
    register_deserializer(ResultType::Ticket,
        [](const char *data, uint32_t size) -> std::unique_ptr<Result> {
            return TicketResult::deserialize(data, size);
        });
    register_deserializer(ResultType::Transfer,
        [](const char *data, uint32_t size) -> std::unique_ptr<Result> {
            return TransferResult::deserialize(data, size);
        });
    register_deserializer(ResultType::Train,
        [](const char *data, uint32_t size) -> std::unique_ptr<Result> {
            return TrainResult::deserialize(data, size);
        });
}

std::unique_ptr<Result> Result::deserialize(ResultType type, const char *data, uint32_t size) {
    register_builtin_deserializers();
    auto& registry = deserializer_registry();
    auto it = registry.find(type);
    if (it == registry.end()) {
        return nullptr;
    }
    return (*it->second)(data, size);
}

void FailureResult::print(std::ostream& os) {
    os << "-1\n";
}

std::pair<const char *, uint32_t> FailureResult::serialize() {
    char *str = new char[4]();
    str[3] = 1;
    return std::make_pair(str, 4);
}

std::unique_ptr<FailureResult> FailureResult::deserialize(const char *data, uint32_t size) {
    return std::make_unique<FailureResult>();
}

void SuccessResult::print(std::ostream& os) {
    os << "0\n";
}

std::pair<const char *, uint32_t> SuccessResult::serialize() {
    char *str = new char[4]();
    str[3] = 2;
    return std::make_pair(str, 4);
}

std::unique_ptr<SuccessResult> SuccessResult::deserialize(const char *data, uint32_t size) {
    return std::make_unique<SuccessResult>();
}

void ProfileResult::print(std::ostream &os) {
    os << username_ << ' ' << name_ << ' ' << email_ << ' ' << privilege_ << '\n';
}

std::pair<const char *, uint32_t> ProfileResult::serialize() {
    uint32_t size = username_.size() + name_.size() + email_.size() + 16;
    char *str = new char[size]();
    uint32_t l1 = username_.size(), l2 = name_.size(), l3 = email_.size();
    memcpy(str, &l1, 4);
    memcpy(str + 4, &l2, 4);
    memcpy(str + 8, &l3, 4);
    memcpy(str + 12, username_.c_str(), username_.size());
    memcpy(str + username_.size() + 12, name_.c_str(), name_.size());
    memcpy(str + username_.size() + name_.size() + 12, email_.c_str(), email_.size());
    memcpy(str + username_.size() + name_.size() + email_.size() + 12, &privilege_, 4);
    return std::make_pair(str, size);
}

std::unique_ptr<ProfileResult> ProfileResult::deserialize(const char *data, uint32_t size) {
    uint32_t l1, l2, l3;
    memcpy(&l1, data, 4);
    memcpy(&l2, data + 4, 4);
    memcpy(&l3, data + 8, 4);
    std::string username(data + 12, l1), name(data + l1 + 12, l2), email(data + l1 + l2 + 12, l3);
    int privilege;
    memcpy(&privilege, data + l1 + l2 + l3 + 12, 4);
    return std::make_unique<ProfileResult>(username, name, email, privilege);
}

void OrderResult::print(std::ostream &os) {
    for (const auto& order : orders_) {
        os << order.ticket_.train_id_ << " ";
        std::string from = order.ticket_.start_station_.str();
        std::string to = order.ticket_.end_station_.str();
        os << from << " ";
        print_time_date(order.ticket_.departure_date_, order.ticket_.departure_time_, os, true);
        os << " -> " << to << " ";
        print_time_date(order.ticket_.arrival_date_, order.ticket_.arrival_time_, os, true);
        os << " " << order.ticket_.price_ << " " << order.ticket_.seat_ << "\n";
    }
}

std::pair<const char *, uint32_t> OrderResult::serialize() {
    const uint32_t recordSize = static_cast<uint32_t>(sizeof(CompleteOrder));
    uint32_t size = static_cast<uint32_t>(orders_.size()) * recordSize + 4;
    uint32_t len = static_cast<uint32_t>(orders_.size());
    char *str = new char[size]();
    int i = 0;
    memcpy(str, &len, 4);
    for (auto o : orders_) {
        memcpy(str + static_cast<uint32_t>(i) * recordSize + 4, &o, recordSize);
        i++;
    }
    return std::make_pair(str, size);
}

std::unique_ptr<OrderResult> OrderResult::deserialize(const char *data, uint32_t size) {
    if (size < 4) {
        return std::make_unique<OrderResult>(sjtu::vector<CompleteOrder>());
    }
    const uint32_t recordSize = static_cast<uint32_t>(sizeof(CompleteOrder));
    uint32_t len = 0;
    memcpy(&len, data, 4);
    const uint64_t maxLenBySize = (size - 4) / recordSize;
    if (static_cast<uint64_t>(len) > maxLenBySize) {
        len = static_cast<uint32_t>(maxLenBySize);
    }
    sjtu::vector<CompleteOrder> vec;
    for (uint32_t i = 0; i < len; i++) {
        CompleteOrder o;
        memcpy(&o, data + i * recordSize + 4, recordSize);
        vec.push_back(o);
    }
    return std::make_unique<OrderResult>(vec);
}

void TicketResult::print(std::ostream &os) {
    for (const auto& ticket : tickets_) {
        os << ticket.train_id_ << " " << ticket.start_station_ << " ";
        print_time_date(ticket.departure_date_, ticket.departure_time_, os, true);
        os << " -> " << ticket.end_station_ << " ";
        print_time_date(ticket.arrival_date_, ticket.arrival_time_, os, true);
        os << " " << ticket.price_ << " " << ticket.seat_ << "\n";
    }
}

std::pair<const char *, uint32_t> TicketResult::serialize() {
    const uint32_t recordSize = static_cast<uint32_t>(sizeof(CompleteTicket));
    uint32_t size = static_cast<uint32_t>(tickets_.size()) * recordSize + 4;
    uint32_t len = static_cast<uint32_t>(tickets_.size());
    char *str = new char[size]();
    int i = 0;
    memcpy(str, &len, 4);
    for (auto t : tickets_) {
        memcpy(str + static_cast<uint32_t>(i) * recordSize + 4, &t, recordSize);
        i++;
    }
    return std::make_pair(str, size);
}

std::unique_ptr<TicketResult> TicketResult::deserialize(const char *data, uint32_t size) {
    if (size < 4) {
        return std::make_unique<TicketResult>(sjtu::vector<CompleteTicket>());
    }
    const uint32_t recordSize = static_cast<uint32_t>(sizeof(CompleteTicket));
    uint32_t len = 0;
    memcpy(&len, data, 4);
    const uint64_t maxLenBySize = (size - 4) / recordSize;
    if (static_cast<uint64_t>(len) > maxLenBySize) {
        len = static_cast<uint32_t>(maxLenBySize);
    }
    sjtu::vector<CompleteTicket> vec;
    for (uint32_t i = 0; i < len; i++) {
        CompleteTicket t;
        memcpy(&t, data + i * recordSize + 4, recordSize);
        vec.push_back(t);
    }
    return std::make_unique<TicketResult>(vec);
}

void TransferResult::print(std::ostream &os) {
    for (const auto& transfer : tickets_) {
        const CompleteTicket& ticket = transfer.first_ticket_;
        os << ticket.train_id_ << " " << ticket.start_station_ << " ";
        print_time_date(ticket.departure_date_, ticket.departure_time_, os, true);
        os << " -> " << ticket.end_station_ << " ";
        print_time_date(ticket.arrival_date_, ticket.arrival_time_, os, true);
        os << " " << ticket.price_ << " " << ticket.seat_ << "\n";
        const CompleteTicket& ticket2 = transfer.second_ticket_;
        os << ticket2.train_id_ << " " << ticket2.start_station_ << " ";
        print_time_date(ticket2.departure_date_, ticket2.departure_time_, os, true);
        os << " -> " << ticket2.end_station_ << " ";
        print_time_date(ticket2.arrival_date_, ticket2.arrival_time_, os, true);
        os << " " << ticket2.price_ << " " << ticket2.seat_ << "\n";
    }
}

std::pair<const char *, uint32_t> TransferResult::serialize() {
    const uint32_t recordSize = static_cast<uint32_t>(sizeof(CompleteTransferTicket));
    uint32_t size = static_cast<uint32_t>(tickets_.size()) * recordSize + 4;
    uint32_t len = static_cast<uint32_t>(tickets_.size());
    char *str = new char[size]();
    int i = 0;
    memcpy(str, &len, 4);
    for (auto t : tickets_) {
        memcpy(str + static_cast<uint32_t>(i) * recordSize + 4, &t, recordSize);
        i++;
    }
    return std::make_pair(str, size);
}

std::unique_ptr<TransferResult> TransferResult::deserialize(const char *data, uint32_t size) {
    if (size < 4) {
        return std::make_unique<TransferResult>(sjtu::vector<CompleteTransferTicket>());
    }
    const uint32_t recordSize = static_cast<uint32_t>(sizeof(CompleteTransferTicket));
    uint32_t len = 0;
    memcpy(&len, data, 4);
    const uint64_t maxLenBySize = (size - 4) / recordSize;
    if (static_cast<uint64_t>(len) > maxLenBySize) {
        len = static_cast<uint32_t>(maxLenBySize);
    }
    sjtu::vector<CompleteTransferTicket> vec;
    for (uint32_t i = 0; i < len; i++) {
        CompleteTransferTicket t;
        memcpy(&t, data + i * recordSize + 4, recordSize);
        vec.push_back(t);
    }
    return std::make_unique<TransferResult>(vec);
}

void TrainResult::print(std::ostream &os) {
    os << train_.train_id_ << " " << train_.type_ << "\n";
    for (int i = 0; i < train_.station_num_; i++) {
        const TrainStationInfo& station = train_.stations_[i];
        os << station.station_name_ << " ";
        if (station.has_arrival_) {
            print_time_date(train_.query_date_, station.arrival_time_, os);
        }
        else {
            os << "xx-xx xx:xx";
        }
        os << " -> ";
        if (station.has_leaving_) {
            print_time_date(train_.query_date_, station.leaving_time_, os);
        }
        else {
            os << "xx-xx xx:xx";
        }
        os << " " << station.price_ << " ";
        if (station.seat_ >= 0) {
            os << station.seat_;
        }
        else {
            os << "x";
        }
        os << "\n";
    }
}

std::pair<const char *, uint32_t> TrainResult::serialize() {
    uint32_t size = sizeof(TrainInfo) + 4;
    char *str = new char[size]();
    uint32_t len = 1;
    memcpy(str, &len, 4);
    memcpy(str + 4, &train_, sizeof(TrainInfo));
    return std::make_pair(str, size);
}

std::unique_ptr<TrainResult> TrainResult::deserialize(const char *data, uint32_t size) {
    if (size < 4 + sizeof(TrainInfo)) {
        return nullptr;
    }
    TrainInfo train{};
    memcpy(&train, data + 4, sizeof(TrainInfo));
    return std::make_unique<TrainResult>(train);
}

void TextResult::print(std::ostream& os) {
    os << text_;
}

std::pair<const char *, uint32_t> TextResult::serialize() {
    uint32_t text_len = static_cast<uint32_t>(text_.size());
    uint32_t size = text_len + 4;
    char *str = new char[size]();
    memcpy(str, &text_len, 4);
    if (text_len > 0) {
        memcpy(str + 4, text_.data(), text_len);
    }
    return std::make_pair(str, size);
}

std::unique_ptr<TextResult> TextResult::deserialize(const char *data, uint32_t size) {
    if (size < 4) {
        return std::make_unique<TextResult>("");
    }
    uint32_t text_len = 0;
    memcpy(&text_len, data, 4);
    if (4 + text_len > size) {
        return std::make_unique<TextResult>("");
    }
    return std::make_unique<TextResult>(std::string(data + 4, text_len));
}

} // namespace sjtu