#include "../../include/utils/validator.hpp"

UnicodeScript detect_script(char32_t cp) {
    if (is_ascii(cp)) {
        return UnicodeScript::Ascii;
    }
    else if ((cp >= 0x00C0 && cp <= 0x024F) || (cp >= 0x1E00 && cp <= 0x1EFF)) {
        return UnicodeScript::Latin;
    }
    else if (is_han(cp)) {
        return UnicodeScript::Han;
    }
    else if (cp >= 0x3040 && cp <= 0x309F) {
        return UnicodeScript::Hiragana;
    }
    else if (cp >= 0x30A0 && cp <= 0x30FF) {
        return UnicodeScript::Katagana;
    }
    else if (cp >= 0xAC00 && cp <= 0xD7AF) {
        return UnicodeScript::Hangul;
    }
    else if (cp >= 0x0370 && cp <= 0x03FF) {
        return UnicodeScript::Greek;
    }
    else if (cp >= 0x0400 && cp <= 0x04FF) {
        return UnicodeScript::Cyrillic;
    }
    else if (cp >= 0x0590 && cp <= 0x05FF) {
        return UnicodeScript::Hebrew;
    }
    else if (cp >= 0x0600 && cp <= 0x06FF) {
        return UnicodeScript::Arabic;
    }
    else if (cp >= 0x0900 && cp <= 0x097F) {
        return UnicodeScript::Devanagari;
    }
    else if (cp >= 0x0E00 && cp <= 0x0E7F) {
        return UnicodeScript::Thai;
    }
    else if (is_cspecial(cp)) {
        return UnicodeScript::CSpecialSymbol;
    }
    else if (cp >= 0x1F300 && cp <= 0x1F5FF) {
        return UnicodeScript::Emoji;
    }
    else {
        return UnicodeScript::Unknown;
    }
}

Validator::Validator(const std::string& str, bool valid) : str_(str), valid_(valid) {}

Validator& Validator::max_len(int len) {
    if (str_.size() > len) {
        valid_ = false;
    }
    return *this;
}

Validator& Validator::min_len(int len) {
    if (str_.size() < len) {
        valid_ = false;
    }
    return *this;
}

Validator& Validator::visible_only() {
    if (!valid_) {
        return *this;
    }
    for (int i = 0; i < str_.size(); i++) {
        if (str_[i] < 33 || str_[i] > 126) {
            valid_ = false;
            break;
        }
    }
    return *this;
}

Validator& Validator::normal_char_only() {
    if (!valid_) {
        return *this;
    }
    for (int i = 0; i < str_.size(); i++) {
        char ch = str_[i];
        bool is_number = (ch >= '0' && ch <= '9');
        bool is_letter = (ch >= 'A' && ch <= 'Z' || ch >= 'a' && ch <= 'z');
        bool is_underline = (ch == '_');
        if (is_number == 0 && is_letter == 0 && is_underline == 0) {
            valid_ = false;
            break;
        }
    }
    return *this;
}

Validator& Validator::no_quotes() {
    if (!valid_) {
        return *this;
    }
    for (int i = 0; i < str_.size(); i++) {
        if (str_[i] == '"') {
            valid_ = false;
            break;
        }
    }
    return *this;
}

Validator& Validator::number_only() {
    if (!valid_) {
        return *this;
    }
    for (int i = 0; i < str_.size(); i++) {
        if (str_[i] < '0' || str_[i] > '9') {
            valid_ = false;
            break;
        }
    }
    return *this;
}

Validator& Validator::number_and_dot_only() {
    if (!valid_) {
        return *this;
    }
    for (int i = 0; i < str_.size(); i++) {
        if ((str_[i] < '0' || str_[i] > '9') && str_[i] != '.') {
            valid_ = false;
            break;
        }
    }
    return *this;
}

Validator& Validator::only_one_dot() {
    if (!valid_) {
        return *this;
    }
    bool flag = 0;
    for (int i = 0; i < str_.size(); i++) {
        if (str_[i] == '.') {
            if (flag) {
                valid_ = 0;
                break;
            }
            else {
                flag = 1;
            }
        }
    }
    return *this;
}

Validator& Validator::no_pipes() {
    if (!valid_) {
        return *this;
    }
    for (int i = 0; i < str_.size(); i++) {
        if (str_[i] == '|') {
            valid_ = 0;
            break;
        }
    }
    return *this;
}

Validator::operator bool() const {
    return valid_;
}

bool is_ascii(char32_t cp) {
    return cp <= 0x7F;
}

bool is_visible_ascii(char32_t cp) {
    return cp >= 33 && cp <= 126;
}

bool is_han(char32_t cp) {
    return
        (cp >= 0x4E00 && cp <= 0x9FFF) ||       // CJK Unified Ideographs
        (cp >= 0x3400 && cp <= 0x4DBF) ||       // CJK Unified Ideographs Extension A
        (cp >= 0x20000 && cp <= 0x2A6DF) ||     // Extension B
        (cp >= 0x2A700 && cp <= 0x2B73F) ||     // Extension C
        (cp >= 0x2B740 && cp <= 0x2B81F) ||     // Extension D
        (cp >= 0x2B820 && cp <= 0x2CEAF) ||     // Extension E
        (cp >= 0x2CEB0 && cp <= 0x2EBEF) ||     // Extension F
        (cp >= 0x30000 && cp <= 0x3134F);       // Extension G
}

bool is_cspecial(char32_t cp) {
    return
        (cp == 0x00B7) ||       // 「·」
        (cp == 0x2014) ||       // 「—」
        (cp == 0xFF08) ||       // 「（」
        (cp == 0xFF09) ||       // 「）」
        (cp == 0xFF1A);         // 「：」
}

sjtu::vector<char32_t> utf8_to_utf32(const std::string& str) {
    sjtu::vector<char32_t> result;
    int i = 0;
    while (i < str.size()) {
        unsigned char ch = str[i];
        char32_t cp = 0;
        int len = 0;
        if (ch <= 0x7F) {
            cp = ch;
            len = 1;
        }
        else if ((ch & 0xE0) == 0xC0) {
            cp = ch & 0x1F;
            len = 2;
        }
        else if ((ch & 0xF0) == 0xE0) {
            cp = ch & 0x0F;
            len = 3;
        }
        else if ((ch & 0xF8) == 0xF0) {
            cp = ch & 0x07;
            len = 4;
        }
        else {
            throw "Invalid UTF-8 leading byte";
        }
        if (i + len > str.size()) {
            throw "Truncated UTF-8";
        }
        for (int j = 1; j < len; j++) {
            unsigned char cc = str[i + j];
            if ((cc & 0xC0) != 0x80) {
                throw "Invalid UTF-8 continuation byte";
            }
            cp = (cp << 6) | (cc & 0x3F);
        }
        result.push_back(cp);
        i += len;
    }
    return result;
}

UnicodeValidator::UnicodeValidator(const std::string& str, bool valid) : utf32_str_(utf8_to_utf32(str)), valid_(valid) {}

UnicodeValidator& UnicodeValidator::max_len(int len) {
    if (utf32_str_.size() > len) {
        valid_ = false;
    }
    return *this;
}

UnicodeValidator& UnicodeValidator::min_len(int len) {
    if (utf32_str_.size() < len) {
        valid_ = false;
    }
    return *this;
}

UnicodeValidator& UnicodeValidator::visible_only() {
    if (!valid_) {
        return *this;
    }
    for (int i = 0; i < utf32_str_.size(); i++) {
        char32_t cp = utf32_str_[i];
        if (cp < 33 || cp > 126) {
            valid_ = false;
            break;
        }
    }
    return *this;
}

UnicodeValidator& UnicodeValidator::normal_char_only() {
    if (!valid_) {
        return *this;
    }
    for (int i = 0; i < utf32_str_.size(); i++) {
        char32_t cp = utf32_str_[i];
        bool is_number = (cp >= '0' && cp <= '9');
        bool is_letter = (cp >= 'A' && cp <= 'Z' || cp >= 'a' && cp <= 'z');
        bool is_underline = (cp == '_');
        if (is_number == 0 && is_letter == 0 && is_underline == 0) {
            valid_ = false;
            break;
        }
    }
    return *this;
}

UnicodeValidator& UnicodeValidator::no_quotes() {
    if (!valid_) {
        return *this;
    }
    for (int i = 0; i < utf32_str_.size(); i++) {
        if (utf32_str_[i] == '"') {
            valid_ = false;
            break;
        }
    }
    return *this;
}

UnicodeValidator& UnicodeValidator::number_only() {
    if (!valid_) {
        return *this;
    }
    for (int i = 0; i < utf32_str_.size(); i++) {
        if (utf32_str_[i] < '0' || utf32_str_[i] > '9') {
            valid_ = false;
            break;
        }
    }
    return *this;
}

UnicodeValidator& UnicodeValidator::number_and_dot_only() {
    if (!valid_) {
        return *this;
    }
    for (int i = 0; i < utf32_str_.size(); i++) {
        if ((utf32_str_[i] < '0' || utf32_str_[i] > '9') && utf32_str_[i] != '.') {
            valid_ = false;
            break;
        }
    }
    return *this;
}

UnicodeValidator& UnicodeValidator::only_one_dot() {
    if (!valid_) {
        return *this;
    }
    bool flag = 0;
    for (int i = 0; i < utf32_str_.size(); i++) {
        if (utf32_str_[i] == '.') {
            if (flag) {
                valid_ = 0;
                break;
            }
            else {
                flag = 1;
            }
        }
    }
    return *this;
}

UnicodeValidator& UnicodeValidator::no_pipes() {
    if (!valid_) {
        return *this;
    }
    for (int i = 0; i < utf32_str_.size(); i++) {
        if (utf32_str_[i] == '|') {
            valid_ = 0;
            break;
        }
    }
    return *this;
}

UnicodeValidator& UnicodeValidator::han() {
    if (!valid_) {
        return *this;
    }
    for (int i = 0; i < utf32_str_.size(); i++) {
        UnicodeScript script = detect_script(utf32_str_[i]);
        if (script != UnicodeScript::Ascii && script != UnicodeScript::Han &&
            !is_cspecial(utf32_str_[i])) {
            valid_ = false;
            break;
        }
    }
    return *this;
}

UnicodeValidator::operator bool() const {
    return valid_;
}