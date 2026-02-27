#ifndef TIME_DATE_HPP
#define TIME_DATE_HPP

#include <stdexcept>
#include <string>
#include <exception>
#include <iostream>

namespace sjtu {

struct time {
    int hr_;
    int min_;
    int day_offset_;

    time(int hr = 0, int min = 0, int day_offset = 0) : hr_(hr), min_(min), day_offset_(day_offset) {}

    time operator+(int min) const {
        int new_min = min_, new_hr = hr_, new_day = day_offset_;
        new_min += min;
        if (new_min >= 60) {
            new_hr += new_min / 60;
            new_min %= 60;
            if (new_hr >= 24) {
                new_day += new_hr / 24;
                new_hr %= 24;
            }
        }
        return time(new_hr, new_min, new_day);
    }

    explicit operator int() const {
        return day_offset_ * 1440 + hr_ * 60 + min_;
    }

};

struct date {
    int month_;
    int day_;

    date() {}

    date(int month, int day) : month_(month), day_(day) {}

    date(int d) {
        if (d >= 0 && d <= 29) {
            month_ = 6;
            day_ = d + 1;
        }
        else if (d > 29 && d <= 60) {
            month_ = 7;
            day_ = d - 29;
        }
        else if (d > 60 && d <= 91) {
            month_ = 8;
            day_ = d - 60;
        }
        else if (d > 91 && d <= 121) {
            month_ = 9;
            day_ = d - 91;
        }
        else {
            throw std::invalid_argument("Invalid day number");
        }
    }

    explicit operator int() const {
        switch (month_) {
            case 6:
                return day_ - 1;
            case 7:
                return day_ + 29;
            case 8:
                return day_ + 60;
            case 9:
                return day_ + 91;
            default:
                return -1;
        }
    }

    date operator+(int d) const {
        return date(int(*this) + d);
    }

};

time parse_time(const std::string& str);

date parse_date(const std::string& str);

void print_time_date(const date& d, const time& t, std::ostream& os);

} // namespace sjtu

#endif // TIME_DATE_HPP