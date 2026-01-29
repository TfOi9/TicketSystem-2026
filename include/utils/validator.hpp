#ifndef VALIDATOR_HPP
#define VALIDATOR_HPP

#include <iostream>
#include <string>
#include <cstdint>
#include <vector>

enum class UnicodeScript {
    Unknown = 0,
    Ascii,
    Latin,
    Han,
    Hiragana,
    Katagana,
    Hangul,
    Greek,
    Cyrillic,
    Hebrew,
    Arabic,
    Devanagari,
    Thai,
    CSpecialSymbol,
    Emoji,
};

UnicodeScript detect_script(char32_t cp);

class Validator {
private:
    std::string str_;
    bool valid_;
public:
    Validator(const std::string& str, bool valid = true);

    ~Validator() = default;

    Validator& max_len(int len);

    Validator& min_len(int len);

    Validator& visible_only();

    Validator& normal_char_only();

    Validator& no_quotes();

    Validator& number_only();

    Validator& number_and_dot_only();

    Validator& only_one_dot();

    Validator& no_pipes();

    operator bool() const;
};

bool is_ascii(char32_t cp);

bool is_visible_ascii(char32_t cp);

bool is_han(char32_t cp);

bool is_cspecial(char32_t cp);

std::vector<char32_t> utf8_to_utf32(const std::string& str);

class UnicodeValidator {
private:
    std::vector<char32_t> utf32_str_;
    bool valid_;
public:
    UnicodeValidator(const std::string& str, bool valid = true);

    ~UnicodeValidator() = default;

    UnicodeValidator& max_len(int len);

    UnicodeValidator& min_len(int len);

    UnicodeValidator& visible_only();

    UnicodeValidator& normal_char_only();

    UnicodeValidator& no_quotes();

    UnicodeValidator& number_only();

    UnicodeValidator& number_and_dot_only();

    UnicodeValidator& only_one_dot();

    UnicodeValidator& no_pipes();

    UnicodeValidator& han();

    operator bool() const;
};

#endif