#ifndef RESULT_HPP
#define RESULT_HPP

#include <iostream>
#include <memory>
#include <utility>
#include <functional>

#include "../stl/vector.hpp"
#include "../system/order.hpp"

namespace sjtu {

enum class ResultType {
    Base = 0,
    Text,
    Failure,
    Success,
    Profile,
    Order,
    Ticket,
    Transfer,
    Train
};

struct TrainStationInfo {
    FixedString<40> station_name_;
    time arrival_time_;
    time leaving_time_;
    int price_;
    int seat_;
    bool has_arrival_;
    bool has_leaving_;
};

struct TrainInfo {
    FixedString<20> train_id_;
    char type_;
    int station_num_;
    date query_date_;
    TrainStationInfo stations_[100];
};

class Result {
protected:
    ResultType type_;
    explicit Result(ResultType type) : type_(type) {}

public:
    virtual ~Result() = default;

    ResultType type() {
        return type_;
    }

    virtual void print(std::ostream& os) = 0;

    virtual std::pair<const char *, uint32_t> serialize() = 0;

    using Deserializer = std::function<std::unique_ptr<Result>(const char *, uint32_t)>;

    static void register_deserializer(ResultType type, Deserializer fn);

    static void register_builtin_deserializers();

    static std::unique_ptr<Result> deserialize(ResultType type, const char *data, uint32_t size);

};

class FailureResult : public Result {
public:
    FailureResult() : Result(ResultType::Failure) {}

    void print(std::ostream& os) override;

    std::pair<const char *, uint32_t> serialize() override;

    static std::unique_ptr<FailureResult> deserialize(const char *data, uint32_t size);

};

class SuccessResult : public Result {
public:
    SuccessResult() : Result(ResultType::Success) {}

    void print(std::ostream& os) override;

    std::pair<const char *, uint32_t> serialize() override;

    static std::unique_ptr<SuccessResult> deserialize(const char *data, uint32_t size);

};

class ProfileResult : public Result {
private:
    std::string username_;
    std::string name_;
    std::string email_;
    int privilege_;

public:
    ProfileResult(const std::string& username, const std::string name, const std::string& email, int privilege)
        : Result(ResultType::Profile), username_(username), name_(name), email_(email), privilege_(privilege) {}

    const std::string& username() const { return username_; }
    const std::string& name() const { return name_; }
    const std::string& email() const { return email_; }
    int privilege() const { return privilege_; }

    void print(std::ostream& os) override;

    std::pair<const char *, uint32_t> serialize() override;

    static std::unique_ptr<ProfileResult> deserialize(const char *data, uint32_t size);

};

class OrderResult : public Result {
    sjtu::vector<CompleteOrder> orders_;

public:
    OrderResult(const sjtu::vector<CompleteOrder>& orders) : Result(ResultType::Order), orders_(orders) {}

    void print(std::ostream& os) override;

    std::pair<const char *, uint32_t> serialize() override;

    static std::unique_ptr<OrderResult> deserialize(const char *data, uint32_t size);

};

class TicketResult : public Result {
    sjtu::vector<CompleteTicket> tickets_;

public:
    TicketResult(const sjtu::vector<CompleteTicket>& tickets) : Result(ResultType::Ticket), tickets_(tickets) {}

    const sjtu::vector<CompleteTicket>& tickets() const { return tickets_; }

    void print(std::ostream& os) override;

    std::pair<const char *, uint32_t> serialize() override;

    static std::unique_ptr<TicketResult> deserialize(const char *data, uint32_t size);

};

class TransferResult : public Result {
    sjtu::vector<CompleteTransferTicket> tickets_;

public:
    TransferResult(const sjtu::vector<CompleteTransferTicket>& tickets) : Result(ResultType::Transfer), tickets_(tickets) {}

    void print(std::ostream& os) override;

    std::pair<const char *, uint32_t> serialize() override;

    static std::unique_ptr<TransferResult> deserialize(const char *data, uint32_t size);

};

class TrainResult : public Result {
    TrainInfo train_;

public:
    TrainResult(const TrainInfo& train) : Result(ResultType::Train), train_(train) {}

    void print(std::ostream& os) override;

    std::pair<const char *, uint32_t> serialize() override;

    static std::unique_ptr<TrainResult> deserialize(const char *data, uint32_t size);

};

class TextResult : public Result {
private:
    std::string text_;

public:
    TextResult() : Result(ResultType::Text) {}

    explicit TextResult(const std::string& text) : Result(ResultType::Text), text_(text) {}

    void print(std::ostream& os) override;

    std::pair<const char *, uint32_t> serialize() override;

    static std::unique_ptr<TextResult> deserialize(const char *data, uint32_t size);

};

} // namespace sjtu

#endif // RESULT_HPP