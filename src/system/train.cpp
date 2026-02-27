#include "../../include/system/train.hpp"

#include <optional>
#include <cstring>

namespace sjtu {
int TrainSystem::train_id(const std::string& train_name) {
    auto ans = train_map_.find(FixedString<20>(train_name));
    return ans.has_value() ? ans.value() : -1;
}

int TrainSystem::station_id(const std::string &station_name) {
    auto ans = station_map_.find(FixedString<40>(station_name));
    return ans.has_value() ? ans.value() : -1;
}

std::string TrainSystem::station_name(int station_id) {
    FixedString<40> ans;
    stations_.read(ans, station_id);
    return ans.str();
}

int TrainSystem::add_train(Train& train) {
    auto ans = train_map_.find(train.trainID_);
    if (ans.has_value()) {
        return -1;
    }
    diskpos_t train_id = trains_.write(train);
    train_map_.insert(train.trainID_, train_id);
    return 0;
}

int TrainSystem::add_station(const std::string &station_name) {
    FixedString<40> str(station_name);
    auto ans = station_map_.find(str);
    if (ans.has_value()) {
        return ans.value();
    }
    int station_id = stations_.write(str);
    station_map_.insert(str, station_id);
    return station_id;
}

int TrainSystem::delete_train(const std::string &train_name) {
    auto ans = train_map_.find(FixedString<20>(train_name));
    if (!ans.has_value()) {
        return -1;
    }
    Train train;
    trains_.read(train, ans.value());
    if (train.released_) {
        return -1;
    }
    train_map_.erase(FixedString<20>(train_name), ans.value());
    return 0;
}

int TrainSystem::release_train(const std::string &train_name) {
    auto ans = train_map_.find(FixedString<20>(train_name));
    if (!ans.has_value()) {
        return -1;
    }
    Train train;
    trains_.read(train, ans.value());
    if (train.released_) {
        return -1;
    }
    train.released_ = true;
    trains_.update(train, ans.value());
    for (int i = 0; i < train.stationNum_; i++) {
        position_map_.insert(train.stations_[i], {ans.value(), i});
    }
    return 0;
}

std::optional<Train> TrainSystem::query_train(const std::string& train_name) {
    auto ans = train_map_.find(FixedString<20>(train_name));
    if (!ans.has_value()) {
        return std::nullopt;
    }
    Train train;
    trains_.read(train, ans.value());
    return train;
}

std::optional<Train> TrainSystem::query_train(int train_id) {
    Train train;
    if (trains_.size() <= train_id) {
        return std::nullopt;
    }
    trains_.read(train, train_id);
    return train;
}

int TrainSystem::query_station(const std::string& station_name, sjtu::vector<TrainPosition>& station_info) {
    auto ans = station_map_.find(FixedString<40>(station_name));
    if (!ans.has_value()) {
        return -1;
    }
    station_info.clear();
    position_map_.find_all(ans.value(), station_info);
    return 0;
}

int TrainSystem::query_station(int station_id, sjtu::vector<TrainPosition>& station_info) {
    position_map_.find_all(station_id, station_info);
    return 0;
}

void TrainSystem::flush() {
    train_map_.flush();
    station_map_.flush();
    position_map_.flush();
}

void TrainSystem::clear() {
    trains_.clear();
    stations_.clear();
    train_map_.clear();
    station_map_.clear();
    position_map_.clear();
}

} // namespace sjtu