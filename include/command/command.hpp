#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <string>

#include "token.hpp"

namespace sjtu {

class Command {
private:
    int timestamp_;
    std::string cmd_;
    std::string args_[26];

public:
    Command() = default;

    Command(const Command& oth) = default;

    Command(TokenStream& stream);

    ~Command() = default;

    int timestamp() const;

    std::string cmd() const;

    std::string arg(char key) const;

};

} // namespace sjtu

#endif // COMMAND_HPP