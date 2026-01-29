#ifndef FIXED_STRING_HPP
#define FIXED_STRING_HPP

#include <cstddef>
#include <iostream>
#include <cstring>
#include <ostream>
#include <stdexcept>
#include <string>

namespace sjtu {
#define FIXEDSTRING_TYPE FixedString<length>
#define FIXEDSTRING_TEMPLATE_ARGS template<size_t length>

FIXEDSTRING_TEMPLATE_ARGS
class FixedString {
private:
    char data_[length + 1];
    constexpr static size_t size_ = length;
public:
    FixedString();

    FixedString(const FixedString& oth);

    explicit FixedString(const char *str);

    explicit FixedString(const std::string& str);

    ~FixedString() = default;

    FixedString& operator=(const FixedString& oth);

    char& operator[](size_t index);

    char operator[](size_t index) const;

    inline static size_t size() { return size_; }

    template<size_t len>
    friend bool operator==(const FixedString<len>& a, const FixedString<len>& b);

    template<size_t len>
    friend bool operator!=(const FixedString<len>& a, const FixedString<len>& b);
    
    template<size_t len>
    friend bool operator>(const FixedString<len>& a, const FixedString<len>& b);
    
    template<size_t len>
    friend bool operator<(const FixedString<len>& a, const FixedString<len>& b);

    template<size_t len>
    friend bool operator>=(const FixedString<len>& a, const FixedString<len>& b);

    template<size_t len>
    friend bool operator<=(const FixedString<len>& a, const FixedString<len>& b);

    template<size_t len>
    friend std::istream& operator>>(std::istream& is, FixedString<len>& fstr);

    template<size_t len>
    friend std::ostream& operator<<(std::ostream& os, const FixedString<len>& fstr);

};

FIXEDSTRING_TEMPLATE_ARGS
FIXEDSTRING_TYPE::FixedString() {
    memset(data_, 0, sizeof(data_));
}

FIXEDSTRING_TEMPLATE_ARGS
FIXEDSTRING_TYPE::FixedString(const FIXEDSTRING_TYPE& oth) {
    memcpy(data_, oth.data_, sizeof(data_));
}

FIXEDSTRING_TEMPLATE_ARGS
FIXEDSTRING_TYPE::FixedString(const char *str) {
    if (str == nullptr) {
        throw std::invalid_argument("null pointer passed to FixedString");
    }
    std::memset(data_, 0, sizeof(data_));
	std::strncpy(data_, str, sizeof(data_) - sizeof(char));
}

FIXEDSTRING_TEMPLATE_ARGS
FIXEDSTRING_TYPE::FixedString(const std::string& str) : FixedString(str.c_str()) {}

FIXEDSTRING_TEMPLATE_ARGS
FIXEDSTRING_TYPE& FIXEDSTRING_TYPE::operator=(const FIXEDSTRING_TYPE& oth) {
    if (this == &oth) {
        return *this;
    }
    memcpy(data_, oth.data_, sizeof(data_));
    return *this;
}

FIXEDSTRING_TEMPLATE_ARGS
char& FIXEDSTRING_TYPE::operator[](size_t index) {
    if (index >= 0 && index < length) {
        return data_[index];
    }
    else {
        throw std::runtime_error("index out of bound");
    }
}

FIXEDSTRING_TEMPLATE_ARGS
char FIXEDSTRING_TYPE::operator[](size_t index) const {
    if (index >= 0 && index < length) {
        return data_[index];
    }
    else {
        throw std::runtime_error("index out of bound");
    }
}

FIXEDSTRING_TEMPLATE_ARGS
bool operator==(const FIXEDSTRING_TYPE& a, const FIXEDSTRING_TYPE& b) {
	return std::strcmp(a.data_, b.data_) == 0;
}

FIXEDSTRING_TEMPLATE_ARGS
bool operator!=(const FIXEDSTRING_TYPE& a, const FIXEDSTRING_TYPE& b) {
	return !(a == b);
}

FIXEDSTRING_TEMPLATE_ARGS
bool operator<(const FIXEDSTRING_TYPE& a, const FIXEDSTRING_TYPE& b) {
	return std::strcmp(a.data_, b.data_) < 0;
}

FIXEDSTRING_TEMPLATE_ARGS
bool operator>(const FIXEDSTRING_TYPE& a, const FIXEDSTRING_TYPE& b) {
	return std::strcmp(a.data_, b.data_) > 0;
}

FIXEDSTRING_TEMPLATE_ARGS
bool operator<=(const FIXEDSTRING_TYPE& a, const FIXEDSTRING_TYPE& b) {
	return std::strcmp(a.data_, b.data_) <= 0;
}

FIXEDSTRING_TEMPLATE_ARGS
bool operator>=(const FIXEDSTRING_TYPE& a, const FIXEDSTRING_TYPE& b) {
	return std::strcmp(a.data_, b.data_) >= 0;
}

FIXEDSTRING_TEMPLATE_ARGS
std::istream& operator>>(std::istream& is, FIXEDSTRING_TYPE& fstr) {
    std::string str;
    is >> str;
    fstr = FixedString<length>(str);
    return is;
}

FIXEDSTRING_TEMPLATE_ARGS
std::ostream& operator<<(std::ostream& os, const FIXEDSTRING_TYPE& fstr) {
    for (size_t i = 0; i < length; i++) {
        if (!fstr.data_[i]) {
            break;
        }
        os << fstr.data_[i];
    }
    return os;
}

} // namespace sjtu

#endif // FIXED_STRING_HPP