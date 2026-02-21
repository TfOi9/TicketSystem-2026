#ifndef TIME_DATE_HPP
#define TIME_DATE_HPP

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
};

struct date {
    int month_;
    int day_;

    date(int month = 1, int day = 1) : month_(month), day_(day) {}
};

} // namespace sjtu

#endif // TIME_DATE_HPP