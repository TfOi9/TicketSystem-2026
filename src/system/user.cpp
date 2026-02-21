#include "../../include/system/user.hpp"
#include <optional>

namespace sjtu {

int UserManager::add_user(const std::string &cur_username, const std::string &username, const std::string &password, const std::string &name, const std::string &email, int privilege) {
    auto it = login_list_.find(FixedString<20>(cur_username));
    if (it == login_list_.end()) {
        return -1;
    }
    if (*(it->second) <= privilege) {
        return -1;
    }
    if (user_map_.find(FixedString<20>(username)) != std::nullopt) {
        return -1;
    }
    User new_user(username, password, name, email, privilege);
    user_map_.insert(FixedString<20>(username), new_user);
    return 0;
}

int UserManager::login(const std::string &username, const std::string &password) {
    FixedString<20> fixed_username = FixedString<20>(username);
    if (login_list_.find(fixed_username) != login_list_.end()) {
        return -1;
    }
    std::optional<User> target_user = user_map_.find(fixed_username);
    if (target_user == std::nullopt) {
        return -1;
    }
    if (target_user->password() != password) {
        return -1;
    }
    login_list_.insert(fixed_username, target_user->privilege());
    return 0;
}

int UserManager::logout(const std::string &username) {
    return login_list_.erase(FixedString<20>(username)) ? 0 : -1;
}

std::optional<User> UserManager::query_profile(const std::string &cur_username, const std::string &username) {
    auto it = login_list_.find(FixedString<20>(cur_username));
    if (it == login_list_.end()) {
        return std::nullopt;
    }
    std::optional<User> target_user = user_map_.find(FixedString<20>(username));
    if (target_user == std::nullopt) {
        return std::nullopt;
    }
    if (target_user->privilege() > *(it->second)) {
        return std::nullopt;
    }
    return target_user;
}

std::optional<User> UserManager::modify_profile(const std::string &cur_username, const std::string &username, const std::string& password, const std::string& name, const std::string& email, int privilege) {
    auto it = login_list_.find(FixedString<20>(cur_username));
    if (it == login_list_.end()) {
        return std::nullopt;
    }
    std::optional<User> target_user = user_map_.find(FixedString<20>(username));
    if (target_user == std::nullopt) {
        return std::nullopt;
    }
    if (cur_username != username && *(it->second) <= target_user->privilege()) {
        return std::nullopt;
    }
    if (privilege != -1 && privilege >= target_user->privilege()) {
        return std::nullopt;
    }
    user_map_.erase(FixedString<20>(username), target_user.value());
    User modified_user(username, password == "" ? target_user->password() : password, name == "" ? target_user->name() : name, email == "" ? target_user->email() : email, privilege == -1 ? target_user->privilege() : privilege);
    user_map_.insert(FixedString<20>(username), modified_user);
    if (cur_username == username && privilege != -1) {
        login_list_[FixedString<20>(username)] = privilege;
    }
    return modified_user;
}

} // namespace sjtu