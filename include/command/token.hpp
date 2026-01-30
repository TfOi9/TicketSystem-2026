#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <cstddef>
#include <string>
#include <vector>

namespace sjtu {

struct Token {
    std::string text;
    size_t pos;
};

class TokenStream {
private:
    std::vector<Token> tokens_;
    size_t cur_;

public:
    TokenStream() = default;

    explicit TokenStream(std::vector<Token>&& tokens);

    TokenStream(const std::string& line);

    const Token* peek() const;

    const Token* get();

    bool empty() const;

    void clear();

    size_t position() const;

    void push(Token&& token);

};

} // namespace sjtu

#endif // TOKEN_HPP