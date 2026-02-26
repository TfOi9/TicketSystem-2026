#ifndef TRAIN_HPP
#define TRAIN_HPP

#include "../../include/utils/fixed_string.hpp"
#include "../../include/utils/time_date.hpp"
#include "../../include/storage/bpt.hpp"
#include "../../include/storage/memory_river.hpp"
#include "../../include/stl/vector.hpp"

namespace sjtu {

struct Train {
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
    bool released_;
};

struct TrainPosition {
    int train_id_;
    int pos_;
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
        trains_(name + "_trains.dat"), stations_(name + "_stations.dat"), train_map_(name + "train_map.dat"),
        station_map_(name + "_station_map.dat"), position_map_(name + "position_map.dat") {}

    int train_id(const std::string& train_name);

    int station_id(const std::string& station_name);

    std::string station_name(int station_id);

    int add_train(Train& train);

    int add_station(const std::string& station_name);

    int delete_train(const std::string& train_name);

    int release_train(const std::string& train_name);

    Train query_train(const std::string& train_name);

    Train query_train(int train_id);

    int query_station(const std::string& station_name, sjtu::vector<TrainPosition>& station_info);

    int query_station(int station_id, sjtu::vector<TrainPosition>& station_info);

    void clear();

};

} // namespace sjtu

#endif // TRAIN_HPP