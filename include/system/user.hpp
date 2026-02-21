#ifndef USER_HPP
#define USER_HPP

#include <optional>
#include <string>

#include "../utils/fixed_string.hpp"
#include "../storage/bpt.hpp"
#include "../stl/unordered_map.hpp"

namespace sjtu {

class User {
private:
    FixedString<20> username_;
    FixedString<30> password_;
    FixedString<20> name_;
    FixedString<30> email_;
    int privilege_;

public:
    User() = default;

    User(const std::string& username, const std::string& password, const std::string& name, const std::string& email, int privilege) :
        username_(username), password_(password), name_(name), email_(email), privilege_(privilege) {}

    std::string username() const {
        return username_.str();
    }

    std::string password() const {
        return password_.str();
    }

    std::string name() const {
        return name_.str();
    }

    std::string email() const {
        return email_.str();
    }

    int privilege() const {
        return privilege_;
    }

    std::string stringify() const {
        return username() + name() + email() + (privilege_ == 10 ? "10" : std::string() + char('0' + privilege_));
    }

};

class UserManager {
private:
    BPlusTree<FixedString<20>, User> user_map_;
    sjtu::unordered_map<FixedString<20>, int> login_list_;

public:
    UserManager(const std::string& file_name = "user.dat") : user_map_(file_name) {}

    ~UserManager() = default;

    int add_user(const std::string& cur_username, const std::string& username, const std::string& password, const std::string& name, const std::string& email, int privilege);

    int login(const std::string& username, const std::string& password);

    int logout(const std::string& username);

    std::optional<User> query_profile(const std::string& cur_username, const std::string& username);

    std::optional<User> modify_profile(const std::string& cur_username, const std::string& username, const std::string& password = "", const std::string& name = "", const std::string& email = "", int privilege = -1);

};

} // namespace sjtu

#endif // USER_HPP