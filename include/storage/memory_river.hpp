#ifndef MEMORY_RIVER_HPP
#define MEMORY_RIVER_HPP

#include <iostream>
#include <fstream>

using std::string;
using std::fstream;
using std::ifstream;
using std::ofstream;

template<class T, int info_len = 4>
class MemoryRiver {
private:
    fstream file;
    string file_name;
    int sizeofT = sizeof(T);
    int info_offset = info_len * sizeof(int);

    int size_;
public:
    MemoryRiver() : size_(1) {}

    MemoryRiver(const string& file_name) : file_name(file_name), size_(1) {}

    ~MemoryRiver() {
        write_info(size_, 1);
        if (file.is_open()) {
            file.close();
        }
    }

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

    bool initialise(string FN = "") {
        if (FN != "") file_name = FN;
        bool f = open_file();
        if (f) {
            get_info(size_, 1);
        }
        else {
            size_ = 1;
        }
        // std::cerr << file_name << " " << size_ << std::endl;
        return f;
    }

    void get_info(int &tmp, int n) {
        if (n > info_len) return;
        if (!file.is_open()) {
            open_file();
        }
        if (!file) {
            return;
        }
        file.seekg((n - 1) * sizeof(int));
        file.read(reinterpret_cast<char *>(&tmp), sizeof(int));
    }

    void write_info(int tmp, int n) {
        if (n > info_len) return;
        if (!file.is_open()) {
            open_file();
        }
        if (!file) {
            return;
        }
        file.seekp((n - 1) * sizeof(int));
        file.write(reinterpret_cast<char *>(&tmp), sizeof(int));
    }

    int write(T &t) {
        int siz = size_;
        size_++;
        file.seekp(info_offset + siz * sizeofT);
        file.write(reinterpret_cast<char *>(&t), sizeofT);
        return siz;
    }

    void update(T &t, const int pos) {
        file.seekp(info_offset + pos * sizeofT);
        file.write(reinterpret_cast<char *>(&t), sizeofT);
    }

    void read(T &t, const int pos) {
        file.seekg(info_offset + pos * sizeofT);
        file.read(reinterpret_cast<char *>(&t), sizeofT);
    }

    int size() const {
        return size_;
    }
};

#endif