#include "../../include/utils/time_date.hpp"
#include <stdexcept>

namespace sjtu {

time parse_time(const std::string &str) {
    if (str.size() != 5 || str[2] != ':') {
        throw std::invalid_argument("Invalid time");
    }
    if (str[0] < '0' || str[0] > '9' || str[1] < '0' || str[1] > '9' ||
         str[3] < '0' || str[3] > '9' || str[4] < '0' || str[4] > '9') {
        throw std::invalid_argument("Invalid time");
    }
    int h = (str[0] - '0') * 10 + (str[1] - '0');
    int m = (str[3] - '0') * 10 + (str[4] - '0');
    if (h >= 24 || m >= 60) {
        throw std::invalid_argument("Invalid time");
    }
    return time(h, m);
}

date parse_date(const std::string &str) {
    if (str.size() != 5 || str[2] != '-') {
        throw std::invalid_argument("Invalid date");
    }
    if (str[0] < '0' || str[0] > '9' || str[1] < '0' || str[1] > '9' ||
         str[3] < '0' || str[3] > '9' || str[4] < '0' || str[4] > '9') {
        throw std::invalid_argument("Invalid date");
    }
    int m = (str[0] - '0') * 10 + (str[1] - '0');
    int d = (str[3] - '0') * 10 + (str[4] - '0');
    if (m > 12 || m < 1 || d < 1) {
        throw std::invalid_argument("Invalid date");
    }
    int maxd = 31;
    if (m == 2) maxd = 28;
    else if (m < 7 && m % 2 == 0 || m > 7 && m % 2 == 1) maxd = 30;
    if (d > maxd) {
        throw std::invalid_argument("Invalid date");
    }
    return date(m, d);
}

void print_time_date(const date& d, const time& t, std::ostream& os, bool discard_offset) {
    date p = d + (discard_offset ? 0 : t.day_offset_);
    os << ((p.month_ < 10) ? "0" : "") << p.month_ << "-" << ((p.day_ < 10) ? "0" : "") << p.day_
        << " " << ((t.hr_ < 10) ? "0" : "") << t.hr_ << ":" << ((t.min_ < 10) ? "0" : "") << t.min_;
}

} // namespace sjtu