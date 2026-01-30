#include "../../include/command/token.hpp"

namespace sjtu {

TokenStream::TokenStream(std::vector<Token>&& tokens) : tokens_(std::move(tokens)), cur_(0) {}

TokenStream::TokenStream(const std::string& line) : cur_(0) {
    std::string text;
    for (const char ch : line) {
        if (ch == ' ') {
            if (!text.empty()) {
                tokens_.push_back(Token{text, tokens_.size()});
            }
            text.clear();
        }
        else if (ch == '\r') {
            // discard CRLF issues
            continue;
        }
        else {
            text.push_back(ch);
        }
    }
    if (!text.empty()) {
        tokens_.push_back(Token{text, tokens_.size()});
    }
}

const Token* TokenStream::peek() const {
    if (cur_ >= tokens_.size()) {
        return nullptr;
    }
    return &tokens_[cur_];
}

const Token* TokenStream::get() {
    const Token* tok = peek();
    if (tok) {
        cur_++;
    }
    return tok;
}

bool TokenStream::empty() const {
    return tokens_.size() > 0;
}

void TokenStream::clear() {
    tokens_.clear();
    cur_ = 0;
}

size_t TokenStream::position() const {
    return cur_;
}

void TokenStream::push(Token&& token) {
    tokens_.push_back(std::move(token));
}

} // namespace sjtu