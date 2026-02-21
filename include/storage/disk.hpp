#ifndef DISK_HPP
#define DISK_HPP

#include <string>
#include <fstream>
#include <iostream>

#include "../config.hpp"

namespace sjtu {
#define DISKMANAGER_TYPE DiskManager<FixedType, FixedInfoType, info_len, reuse>
#define DISKMANAGER_TEMPLATE_ARGS template<typename FixedType, typename FixedInfoType, int info_len, bool reuse>

template<typename FixedType, typename FixedInfoType = diskpos_t, int info_len = 12, bool reuse = true>
class DiskManager {
private:
    std::fstream file_;
    std::string file_name_;
    constexpr static diskpos_t sizeofT = sizeof(FixedType);
    constexpr static diskpos_t sizeofInfo = sizeof(FixedInfoType);
    constexpr static diskpos_t info_offset = info_len * sizeofInfo;
    constexpr static int free_index = 3;

    /*
        Current info area distribution:

            [1. root of BPT] [2. size of free list] [3. free pos #1] ... [(n + 2). free pos #n]

    */

    bool open_file();

    class FreeList {
        friend class DISKMANAGER_TYPE;
    private:
        struct FreeListNode {
            diskpos_t pos_;
            FreeListNode *next_;

            FreeListNode(diskpos_t pos = 0) : pos_(pos), next_(nullptr) {}
        };

        FreeListNode *head_;
        size_t size_;

    public:
        FreeList() : size_(0) {
            head_ = new FreeListNode();
        }

        bool empty() const {
            return size_ == 0;
        }

        size_t size() const {
            return size_;
        }

        void push(diskpos_t pos) {
            FreeListNode *new_node = new FreeListNode(pos);
            new_node->next_ = head_->next_;
            head_->next_ = new_node;
            size_++;
        }

        diskpos_t pop() {
            FreeListNode *del = head_->next_;
            head_->next_ = del->next_;
            diskpos_t ret = del->pos_;
            delete del;
            size_--;
            return ret;
        }

        void clear() {
            FreeListNode *cur = head_->next_;
            while (cur) {
                FreeListNode *del = cur;
                cur = cur->next_;
                delete del;
            }
            size_ = 0;
        }

        ~FreeList() {
            FreeListNode *cur = head_;
            while (cur) {
                FreeListNode *del = cur;
                cur = cur->next_;
                delete del;
            }
        }
    };

    FreeList free_;

    void flush_list();

    void restore_list();

public:
    DiskManager() = default;

    ~DiskManager();

    bool initialise(const std::string& file_name = "default.dat");

    void get_info(FixedInfoType& info, int idx);

    void write_info(FixedInfoType& info, int idx);

    void read(FixedType& t, const diskpos_t pos);

    void update(FixedType& t, const diskpos_t pos);

    diskpos_t write(FixedType& t);

    void erase(diskpos_t pos);

    void clear();

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
void DISKMANAGER_TYPE::flush_list() {
    diskpos_t size = static_cast<diskpos_t>(free_.size());
    write_info(size, 1);
    constexpr int free_capacity = info_len - free_index + 1;
    if (free_capacity > 0 && static_cast<int>(free_.size()) <= free_capacity) {
        typename FreeList::FreeListNode *cur = free_.head_->next_;
        for (int i = 0; i < static_cast<int>(free_.size()); i++) {
            write_info(cur->pos_, i + free_index);
            cur = cur->next_;
        }
    }
    else {
        std::fstream list_file(file_name_ + ".free_list.dat", std::ios::out | std::ios::binary);
        typename FreeList::FreeListNode *cur = free_.head_->next_;
        for (int i = 0; i < static_cast<int>(free_.size()); i++) {
            list_file.write(reinterpret_cast<char *>(&(cur->pos_)), sizeof(diskpos_t));
            cur = cur->next_;
        }
    }
}

DISKMANAGER_TEMPLATE_ARGS
void DISKMANAGER_TYPE::restore_list() {
    diskpos_t size;
    get_info(size, 1);
    constexpr int free_capacity = info_len - free_index + 1;
    if (free_capacity > 0 && static_cast<int>(size) <= free_capacity) {
        for (int i = free_index; i < static_cast<int>(size) + free_index; i++) {
            diskpos_t pos;
            get_info(pos, i);
            free_.push(pos);
        }
    }
    else {
        std::fstream list_file(file_name_ + ".free_list.dat", std::ios::in | std::ios::out | std::ios::binary);
        if (!list_file) {
            // file open faliure, discarding free pages
            return;
        }
        list_file.seekg(0);
        for (int i = 0; i < size; i++) {
            diskpos_t pos;
            list_file.read(reinterpret_cast<char *>(&pos), sizeof(diskpos_t));
            free_.push(pos);
        }
    }
}

DISKMANAGER_TEMPLATE_ARGS
DISKMANAGER_TYPE::~DiskManager() {
    if (reuse) {
        flush_list();
    }
    if (file_.is_open()) {
        file_.close();
    }
}

DISKMANAGER_TEMPLATE_ARGS
bool DISKMANAGER_TYPE::initialise(const std::string& file_name) {
    file_name_ = file_name;
    bool f = open_file();
    if (f && reuse) {
        restore_list();
    }
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
    if (!free_.empty()) {
        diskpos_t pos = free_.pop();
        file_.seekp(pos);
        file_.write(reinterpret_cast<char *>(&t), sizeofT);
        return pos;
    }
    file_.seekp(0, std::ios::end);
    diskpos_t pos = file_.tellp();
    file_.write(reinterpret_cast<char *>(&t), sizeofT);
    return pos;
}

DISKMANAGER_TEMPLATE_ARGS
void DISKMANAGER_TYPE::erase(diskpos_t pos) {
    if (reuse) {
        free_.push(pos);
    }
}

DISKMANAGER_TEMPLATE_ARGS
void DISKMANAGER_TYPE::clear() {
    if (file_.is_open()) {
        file_.close();
    }
    file_.open(file_name_, std::ios::binary | std::ios::trunc);
}

} // namespace sjtu

#endif // DISK_HPP