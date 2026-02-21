#ifndef TRAIN_HPP
#define TRAIN_HPP

#include "../../include/utils/fixed_string.hpp"
#include "../../include/utils/time_date.hpp"

namespace sjtu {

class Train {
private:
    FixedString<20> trainID_;
    int stationNum_;
    int stations_[100];
    int seatNum_;
    int prices_[100];
    time startTime_;
    int travelTimes_[100];
    int stopoverTimes_[100];
    date startSaleDate_;
    date endSaleDate_;
    char type_;
};

} // namespace sjtu

#endif // TRAIN_HPP