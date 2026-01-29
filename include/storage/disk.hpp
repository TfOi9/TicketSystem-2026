#ifndef DISK_HPP
#define DISK_HPP

#include <string>
#include <fstream>

#include "../config.hpp"
#include "../utils/type_helper.hpp"

namespace sjtu {
#define DISKMANAGER_TYPE DiskManager<FixedType, FixedInfoType, info_len>
#define DISKMANAGER_TEMPLATE_ARGS template<typename FixedType, typename FixedInfoType, int info_len>

template<typename FixedType, typename FixedInfoType = diskpos_t, int info_len = 4>
class DiskManager {
private:
    std::fstream file_;
    std::string file_name_;
    constexpr static diskpos_t sizeofT = sizeof(FixedType);
    constexpr static diskpos_t sizeofInfo = sizeof(FixedInfoType);
    constexpr static diskpos_t info_offset = info_len * sizeofInfo;
    static_assert(is_pod_v<FixedType>, "DiskManager requires POD FixedType");

    bool open_file();

public:
    DiskManager() = default;

    ~DiskManager();

    bool initialise(const std::string& file_name = "default.dat");

    void get_info(FixedInfoType& info, int idx);

    void write_info(FixedInfoType& info, int idx);

    void read(FixedType& t, const diskpos_t pos);

    void update(FixedType& t, const diskpos_t pos);

    diskpos_t write(FixedType& t);
};

DISKMANAGER_TEMPLATE_ARGS
bool DISKMANAGER_TYPE::open_file() {
    file_.open(file_name_, std::ios::in | std::ios::out | std::ios::binary);
    if (!file_) {
        file_.open(file_name_, std::ios::out | std::ios::binary);
        file_.close();
        file_.open(file_name_, std::ios::in | std::ios::out | std::ios::binary);
        int temp = 0;
        for (int i = 0; i < info_len; i++) {
            file_.write(reinterpret_cast<char *>(&temp), sizeof(int));
        }
        return false;
    }
    return true;
}

DISKMANAGER_TEMPLATE_ARGS
DISKMANAGER_TYPE::~DiskManager() {
    if (file_.is_open()) {
        file_.close();
    }
}

DISKMANAGER_TEMPLATE_ARGS
bool DISKMANAGER_TYPE::initialise(const std::string& file_name) {
    file_name_ = file_name;
    bool f = open_file();
    return f;
}

DISKMANAGER_TEMPLATE_ARGS
void DISKMANAGER_TYPE::get_info(FixedInfoType &info, int idx) {
    if (idx < 1 || idx > info_len) {
        return;
    }
    if (!file_.is_open()) {
        open_file();
    }
    if (!file_) {
        return;
    }
    file_.seekg((idx - 1) * sizeofInfo);
    file_.read(reinterpret_cast<char *>(&info), sizeofInfo);
}

DISKMANAGER_TEMPLATE_ARGS
void DISKMANAGER_TYPE::write_info(FixedInfoType& info, int idx) {
    if (idx < 1 || idx > info_len) {
        return;
    }
    if (!file_.is_open()) {
        open_file();
    }
    if (!file_) {
        return;
    }
    file_.seekp((idx - 1) * sizeofInfo);
    file_.write(reinterpret_cast<char *>(&info), sizeofInfo);
}

DISKMANAGER_TEMPLATE_ARGS
void DISKMANAGER_TYPE::read(FixedType& t, const diskpos_t pos) {
    file_.seekg(pos);
    file_.read(reinterpret_cast<char *>(&t), sizeofT);
}

DISKMANAGER_TEMPLATE_ARGS
void DISKMANAGER_TYPE::update(FixedType &t, const diskpos_t pos) {
    file_.seekp(pos);
    file_.write(reinterpret_cast<char *>(&t), sizeofT);
}

DISKMANAGER_TEMPLATE_ARGS
diskpos_t DISKMANAGER_TYPE::write(FixedType& t) {
    file_.seekp(0, std::ios::end);
    diskpos_t pos = file_.tellp();
    file_.write(reinterpret_cast<char *>(&t), sizeofT);
    return pos;
}

} // namespace sjtu

#endif // DISK_HPP