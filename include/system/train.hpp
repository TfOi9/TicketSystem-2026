#ifndef TRAIN_HPP
#define TRAIN_HPP

#include "../../include/utils/fixed_string.hpp"
#include "../../include/utils/time_date.hpp"
#include "../../include/storage/bpt.hpp"
#include "../../include/storage/memory_river.hpp"
#include "../../include/stl/vector.hpp"

namespace sjtu {

struct Train {
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
    bool released_;
};

struct TrainPosition {
    int train_id_;
    int pos_;
};

struct TrainInStation {
    int station_id_;
    int train_id_;
};

class TrainManager {
private:
    MemoryRiver<Train> trains_;
    MemoryRiver<FixedString<40>> stations_;
    BPlusTree<FixedString<20>, int> train_map_;
    BPlusTree<FixedString<40>, int> station_map_;
    BPlusTree<TrainInStation, TrainPosition> position_map_;

public:
    TrainManager(const std::string& name = "train_manager") :
        trains_(name + "_trains.dat"), stations_(name + "_stations.dat"), train_map_(name + "train_map.dat"),
        station_map_(name + "_station_map.dat"), position_map_(name + "position_map.dat") {}

    int train_id(const std::string& train_name);

    int station_id(const std::string& station_name);

    std::string train_name(int train_id);

    std::string station_name(int station_id);

    int add_train(Train& train);

    int delete_train(const std::string& train_name);

    int release_train(const std::string& train_name);

    Train query_train(const std::string& train_name);

    Train query_train(int train_id);

    int update_train(const std::string& train_name, Train& modified_train);

    int query_station(const std::string& station_name, sjtu::vector<TrainInStation>& station_info);

    int query_station(int station_id, sjtu::vector<TrainInStation>& station_info);

    void clear();

};

} // namespace sjtu

#endif // TRAIN_HPP