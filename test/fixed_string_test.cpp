#include <cassert>
#include <sstream>
#include <stdexcept>

#include "../include/utils/fixed_string.hpp"

using sjtu::FixedString;

int main() {
    {
        FixedString<5> s;
        for (size_t i = 0; i < FixedString<5>::size(); ++i) {
            assert(s[i] == '\0');
        }
    }

    {
        FixedString<4> s("abc");
        assert(s[0] == 'a');
        assert(s[1] == 'b');
        assert(s[2] == 'c');

        FixedString<4> t(std::string("abcd"));
        std::ostringstream os;
        os << t;
        assert(os.str() == "abcd");
    }

    {
        FixedString<3> s("abcdef");
        std::ostringstream os;
        os << s;
        assert(os.str() == "abc");
    }

    {
        FixedString<6> a("alpha");
        FixedString<6> b(a);
        FixedString<6> c;
        c = b;
        assert(a == b);
        assert(b == c);
    }

    {
        FixedString<5> a("abc");
        FixedString<5> b("abd");
        FixedString<5> c("abc");
        assert(a < b);
        assert(b > a);
        assert(a == c);
        assert(a != b);
    }

    {
        FixedString<4> s("xxxx");
        bool threw = false;
        try {
            (void)s[4];
        } catch (const std::runtime_error&) {
            threw = true;
        }
        assert(threw);
    }

    {
        bool threw = false;
        try {
            FixedString<3> s(nullptr);
            (void)s;
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        assert(threw);
    }

    {
        FixedString<5> s;
        std::istringstream iss("hello-world");
        iss >> s;
        std::ostringstream os;
        os << s;
        assert(os.str() == "hello");
    }

    return 0;
}
