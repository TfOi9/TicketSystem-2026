#ifndef TRAIN_HPP
#define TRAIN_HPP

#include "../../include/utils/fixed_string.hpp"
#include "../../include/utils/time_date.hpp"
#include "../../include/storage/bpt.hpp"
#include "../../include/storage/dynamic_river.hpp"
#include "../../include/storage/memory_river.hpp"
#include "../../include/stl/vector.hpp"

namespace sjtu {

constexpr int max_stations = 100;

struct Train {
    int stationNum_;
    FixedString<20> trainID_;
    int stations_[max_stations];
    int seatNum_;
    int prices_[max_stations];
    time startTime_;
    int travelTimes_[max_stations];
    int stopoverTimes_[max_stations];
    time arrivalTimes_[max_stations];
    int seats_[92][max_stations];
    date startSaleDate_;
    date endSaleDate_;
    char type_;
    bool released_;
};

struct TrainStringifier {
    char *operator()(Train& t, int& len) const {
        char *data = new char[60 + t.stationNum_ * 396];
        memcpy(data, reinterpret_cast<char *>(&(t.stationNum_)), 4);
        for (int i = 0; i < 20; i++) {
            data[i + 4] = t.trainID_[i];
        }
        memcpy(data + 24, reinterpret_cast<char *>(t.stations_), t.stationNum_ * 4);
        memcpy(data + 24 + 4 * t.stationNum_, reinterpret_cast<char *>(&(t.seatNum_)), 4);
        memcpy(data + 28 + 4 * t.stationNum_, reinterpret_cast<char *>(t.prices_), t.stationNum_ * 4);
        memcpy(data + 28 + 8 * t.stationNum_, reinterpret_cast<char *>(&(t.startTime_)), 12);
        memcpy(data + 40 + 8 * t.stationNum_, reinterpret_cast<char *>(t.travelTimes_), t.stationNum_ * 4);
        memcpy(data + 40 + 12 * t.stationNum_, reinterpret_cast<char *>(t.stopoverTimes_), t.stationNum_ * 4);
        memcpy(data + 40 + 16 * t.stationNum_, reinterpret_cast<char *>(t.arrivalTimes_), t.stationNum_ * 12);
        for (int i = 0; i < 92; i++) {
            memcpy(data + 40 + (28 + i * 4) * t.stationNum_, reinterpret_cast<char *>(t.seats_[i]), t.stationNum_ * 4);
        }
        memcpy(data + 40 + 396 * t.stationNum_, reinterpret_cast<char *>(&(t.startSaleDate_)), 8);
        memcpy(data + 48 + 396 * t.stationNum_, reinterpret_cast<char *>(&(t.endSaleDate_)), 8);
        data[56 + 396 * t.stationNum_] = t.type_;
        data[57 + 396 * t.stationNum_] = t.released_;
        return data;
    }
};

struct TrainAntiStringifier {
    Train operator()(const char *data) const {
        Train t;
        memcpy(reinterpret_cast<char *>(&(t.stationNum_)), data, 4);
        for (int i = 0; i < 20; i++) {
            t.trainID_[i] = data[i + 4];
        }
        memset(t.stations_, 0, sizeof(t.stations_));
        memset(t.prices_, 0, sizeof(t.prices_));
        memset(t.travelTimes_, 0, sizeof(t.travelTimes_));
        memset(t.stopoverTimes_, 0, sizeof(t.stopoverTimes_));
        memset(t.arrivalTimes_, 0, sizeof(t.arrivalTimes_));
        memset(t.seats_, 0, sizeof(t.seats_));
        memcpy(reinterpret_cast<char *>(t.stations_), data + 24, t.stationNum_ * 4);
        memcpy(reinterpret_cast<char *>(&(t.seatNum_)), data + 24 + 4 * t.stationNum_, 4);
        memcpy(reinterpret_cast<char *>(t.prices_), data + 28 + 4 * t.stationNum_, t.stationNum_ * 4);
        memcpy(reinterpret_cast<char *>(&(t.startTime_)), data + 28 + 8 * t.stationNum_, 12);
        memcpy(reinterpret_cast<char *>(t.travelTimes_), data + 40 + 8 * t.stationNum_, t.stationNum_ * 4);
        memcpy(reinterpret_cast<char *>(t.stopoverTimes_), data + 40 + 12 * t.stationNum_, t.stationNum_ * 4);
        memcpy(reinterpret_cast<char *>(t.arrivalTimes_), data + 40 + 16 * t.stationNum_, t.stationNum_ * 12);
        for (int i = 0; i < 92; i++) {
            memcpy(reinterpret_cast<char *>(t.seats_[i]), data + 40 + (28 + i * 4) * t.stationNum_, t.stationNum_ * 4);
        }
        memcpy(reinterpret_cast<char *>(&(t.startSaleDate_)), data + 40 + 396 * t.stationNum_, 8);
        memcpy(reinterpret_cast<char *>(&(t.endSaleDate_)), data + 48 + 396 * t.stationNum_, 8);
        t.type_ = data[56 + 396 * t.stationNum_];
        t.released_ = data[57 + 396 * t.stationNum_] != 0;
        return t;
    }
};

struct TrainPosition {
    int train_id_;
    int pos_;
};

inline bool operator==(const TrainPosition& a, const TrainPosition& b) {
    return a.train_id_ == b.train_id_ && a.pos_ == b.pos_;
}

inline bool operator!=(const TrainPosition& a, const TrainPosition& b) {
    return !(a == b);
}

inline bool operator<(const TrainPosition& a, const TrainPosition& b) {
    if (a.train_id_ == b.train_id_) {
        return a.pos_ < b.pos_;
    }
    return a.train_id_ < b.train_id_;
}

struct TrainPositionCompare {
    bool operator()(const TrainPosition& a, const TrainPosition& b) const {
        if (a.train_id_ == b.train_id_) {
            return a.pos_ < b.pos_;
        }
        return a.train_id_ < b.train_id_;
    }
};

class TrainSystem {
private:
    MemoryRiver<Train> trains_;
    
    MemoryRiver<FixedString<40>> stations_;
    BPlusTree<FixedString<20>, int> train_map_;
    BPlusTree<FixedString<40>, int> station_map_;
    BPlusTree<int, TrainPosition> position_map_;

public:
    TrainSystem(const std::string& name = "train") :
        trains_(name + "_trains.dat"), stations_(name + "_stations.dat"), train_map_(name + "_train_map.dat"),
        station_map_(name + "_station_map.dat"), position_map_(name + "_position_map.dat") {}

    int train_id(const std::string& train_name);

    int station_id(const std::string& station_name);

    std::string station_name(int station_id);

    int add_train(Train& train);

    int add_station(const std::string& station_name);

    int delete_train(const std::string& train_name);

    int release_train(const std::string& train_name);

    std::optional<Train> query_train(const std::string& train_name);

    std::optional<Train> query_train(int train_id);

    int query_station(const std::string& station_name, sjtu::vector<TrainPosition>& station_info);

    int query_station(int station_id, sjtu::vector<TrainPosition>& station_info);

    void flush();

    void clear();

};

} // namespace sjtu

#endif // TRAIN_HPP