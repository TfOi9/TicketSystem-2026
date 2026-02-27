#ifndef DYNAMIC_RIVER_HPP
#define DYNAMIC_RIVER_HPP

#include <cstddef>
#include <fstream>
#include <string>

#include "../config.hpp"

namespace sjtu {
template<typename T, typename Stringifier, typename AntiStringifier, typename SizeCalculator>
class DynamicRiver {
private:
    std::fstream file;
    std::string file_name;
    Stringifier str_;
    AntiStringifier astr_;
    SizeCalculator calc_;

    bool open_file() {
        file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
        if (!file) {
            file.open(file_name, std::ios::out | std::ios::binary);
            file.close();
            file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
            return 0;
        }
        return 1;
    }

public:
    DynamicRiver(const std::string& file_name) : file_name(file_name), str_(), astr_() {
        open_file();
    }

    ~DynamicRiver() {
        if (file.is_open()) {
            file.close();
        }
    }

    void clear() {
        if (file.is_open()) {
            file.close();
        }
        file.open(file_name, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!file) {
            return;
        }
        file.close();
        file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
    }

    diskpos_t write(T& t) {
        int len = 0;
        char *data = str_(t, len);
        file.seekp(0, std::ios::end);
        diskpos_t pos = file.tellp();
        file.write(data, len);
        delete []data;
        return pos;
    }

    void update(T& t, diskpos_t pos) {
        int len = 0;
        char *data = str_(t, len);
        file.seekp(pos);
        file.write(data, len);
        delete []data;
    }

    void read(T& t, diskpos_t pos) {
        int siz = 0;
        file.seekg(pos);
        file.read(reinterpret_cast<char *>(&siz), 4);
        int len = calc_(siz);
        char *data = new char[len];
        file.seekg(pos);
        file.read(data, len);
        t = astr_(data);
        delete []data;
    }

};

} // namespace sjtu

#endif // DYNAMIC_RIVER_HPP