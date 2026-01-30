#include <stdexcept>

#include "../../include/command/command.hpp"

namespace sjtu {

Command::Command(TokenStream& stream) {
    const Token *time = stream.get();
    if (time == nullptr) {
        throw std::runtime_error("timestamp not found");
    }
    if (time->text.size() <= 2) {
        throw std::runtime_error("timestamp format error");
    }
    if (time->text.front() != '[' || time->text.back() != ']') {
        throw std::runtime_error("timestamp format error");
    }
    try {
        timestamp_ = std::stoi(time->text.substr(1, time->text.size() - 2));
    }
    catch(...) {
        throw std::runtime_error("timestamp invalid");
    }
    const Token *cmd = stream.get();
    if (cmd == nullptr) {
        throw std::runtime_error("command not found");
    }
    cmd_ = cmd->text;
    while (1) {
        const Token *key, *arg;
        key = stream.get();
        if (key == nullptr) {
            break;
        }
        if (key->text.size() != 2 || key->text[0] != '-') {
            throw std::runtime_error("key format error");
        }
        if (key->text[1] < 'a' || key->text[1] > 'z') {
            throw std::runtime_error("key invalid");
        }
        if (!args_[key->text[1] - 'a'].empty()) {
            throw std::runtime_error("duplicated key");
        }
        arg = stream.get();
        if (arg == nullptr) {
            throw std::runtime_error("missing argument for key " + key->text);
        }
        args_[key->text[1] - 'a'] = arg->text;
    }
}

int Command::timestamp() const {
    return timestamp_;
}

std::string Command::cmd() const {
    return cmd_;
}

std::string Command::arg(char key) const {
    return (key >= 'a' && key <= 'z') ? args_[key - 'a'] : "";
}

bool Command::check(const std::string& must, const std::string& optional) {
    for (const char ch : must) {
        if (args_[ch].empty()) {
            return false;
        }
    }
    for (char ch = 'a'; ch <= 'z'; ch++) {
        if (args_[ch].empty()) {
            continue;
        }
        if (must.find(ch) == std::string::npos && optional.find(ch) == std::string::npos) {
            return false;
        }
    }
    return true;
}

} // namespace sjtu