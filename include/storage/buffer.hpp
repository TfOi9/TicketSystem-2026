#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <list>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "../config.hpp"
#include "page.hpp"
#include "disk.hpp"

namespace sjtu {
#define BUFFER_MANAGER_TYPE BufferManager<KeyType, ValueType>
#define BUFFER_MANAGER_TEMPLATE_ARGS template<typename KeyType, typename ValueType>

BUFFER_MANAGER_TEMPLATE_ARGS
class BufferManager {
private:
    struct CacheEntry {
        diskpos_t pos_;
        std::shared_ptr<PAGE_TYPE> page_;
        bool dirty_;
        typename std::list<diskpos_t>::iterator lru_it_;
    };
    DiskManager<PAGE_TYPE> disk_;
    std::unordered_map<diskpos_t, CacheEntry> cache_;
    std::unordered_set<diskpos_t> cache_in_use_;
    std::list<diskpos_t> lru_list_;
    size_t cache_capacity_;

    void evict();

    void promote(diskpos_t pos);

    void load(diskpos_t pos);

public:
    BufferManager(size_t cache_capacity = CACHE_CAPACITY, const std::string& file_name = "default.dat");

    BufferManager(const BufferManager& oth) = delete;

    ~BufferManager();

    BufferManager& operator=(const BufferManager& oth) = delete;

    std::shared_ptr<const PAGE_TYPE> get_page(diskpos_t pos);

    std::shared_ptr<PAGE_TYPE> get_page_mutable(diskpos_t pos);

    void mark_dirty(diskpos_t pos);

    diskpos_t insert_page(PAGE_TYPE& page);

    void flush();

    diskpos_t get_root_pos();

    void set_root_pos(diskpos_t pos);

    void finish_use(diskpos_t pos);

};

BUFFER_MANAGER_TEMPLATE_ARGS
BUFFER_MANAGER_TYPE::BufferManager(size_t cache_capacity, const std::string& file_name) : cache_capacity_(cache_capacity) {
    disk_.initialise(file_name);
}

BUFFER_MANAGER_TEMPLATE_ARGS
BUFFER_MANAGER_TYPE::~BufferManager() {
    flush();
}

BUFFER_MANAGER_TEMPLATE_ARGS
void BUFFER_MANAGER_TYPE::evict() {
    if (lru_list_.empty()) {
        return;
    }
    for (auto rit = lru_list_.rbegin(); rit != lru_list_.rend(); rit++) {
        diskpos_t cand = *rit;
        if (cache_in_use_.find(cand) == cache_in_use_.end()) {
            auto it = cache_.find(cand);
            if (it != cache_.end()) {
                if (it->second.dirty_) {
                    disk_.update(*(it->second.page_), cand);
                }
                lru_list_.erase(std::next(rit).base());
                cache_.erase(it);
                return;
            }
        }
    }
}

BUFFER_MANAGER_TEMPLATE_ARGS
void BUFFER_MANAGER_TYPE::promote(diskpos_t pos) {
    auto it = cache_.find(pos);
    if (it != cache_.end()) {
        lru_list_.erase(it->second.lru_it_);
        lru_list_.push_front(pos);
        it->second.lru_it_ = lru_list_.begin();
    }
}

BUFFER_MANAGER_TEMPLATE_ARGS
void BUFFER_MANAGER_TYPE::load(diskpos_t pos) {
    auto page_ptr = std::make_shared<PAGE_TYPE>();
    disk_.read(*page_ptr, pos);
    CacheEntry entry;
    entry.pos_ = pos;
    entry.page_ = page_ptr;
    entry.dirty_ = false;
    lru_list_.push_front(pos);
    entry.lru_it_ = lru_list_.begin();
    cache_[pos] = entry;
}

BUFFER_MANAGER_TEMPLATE_ARGS
std::shared_ptr<const PAGE_TYPE> BUFFER_MANAGER_TYPE::get_page(diskpos_t pos) {
    auto it = cache_.find(pos);
    if (it != cache_.end()) {
        promote(pos);
        return std::const_pointer_cast<const PAGE_TYPE>(it->second.page_);
    }
    if (cache_.size() >= cache_capacity_) {
        evict();
    }
    load(pos);
    return std::const_pointer_cast<const PAGE_TYPE>(cache_[pos].page_);
}

BUFFER_MANAGER_TEMPLATE_ARGS
std::shared_ptr<PAGE_TYPE> BUFFER_MANAGER_TYPE::get_page_mutable(diskpos_t pos) {
    auto it = cache_.find(pos);
    if (it != cache_.end()) {
        promote(pos);
        mark_dirty(pos);
        cache_in_use_.insert(pos);
        return it->second.page_;
    }
    if (cache_.size() >= cache_capacity_) {
        evict();
    }
    load(pos);
    mark_dirty(pos);
    cache_in_use_.insert(pos);
    return cache_[pos].page_;
}

BUFFER_MANAGER_TEMPLATE_ARGS
void BUFFER_MANAGER_TYPE::mark_dirty(diskpos_t pos) {
    auto it = cache_.find(pos);
    if (it != cache_.end()) {
        it->second.dirty_ = true;
    }
}

BUFFER_MANAGER_TEMPLATE_ARGS
diskpos_t BUFFER_MANAGER_TYPE::insert_page(Page<KeyType, ValueType> &page) {
    if (cache_.size() >= cache_capacity_) {
        evict();
    }
    diskpos_t pos = disk_.write(page);
    std::shared_ptr<PAGE_TYPE> page_ptr = std::make_shared<PAGE_TYPE>(page);
    CacheEntry entry;
    entry.pos_ = pos;
    entry.page_ = page_ptr;
    entry.dirty_ = false;
    lru_list_.push_front(pos);
    entry.lru_it_ = lru_list_.begin();
    cache_[pos] = entry;
    return pos;
}

BUFFER_MANAGER_TEMPLATE_ARGS
void BUFFER_MANAGER_TYPE::flush() {
    for (auto& pair : cache_) {
        if (pair.second.dirty_) {
            disk_.update(*(pair.second.page_), pair.first);
            pair.second.dirty_ = false;
        }
    }
    cache_.clear();
    lru_list_.clear();
    cache_in_use_.clear();
}

BUFFER_MANAGER_TEMPLATE_ARGS
diskpos_t BUFFER_MANAGER_TYPE::get_root_pos() {
    diskpos_t root_pos;
    disk_.get_info(root_pos, 2);
    return root_pos;
}

BUFFER_MANAGER_TEMPLATE_ARGS
void BUFFER_MANAGER_TYPE::set_root_pos(diskpos_t pos) {
    disk_.write_info(pos, 2);
}

BUFFER_MANAGER_TEMPLATE_ARGS
void BUFFER_MANAGER_TYPE::finish_use(diskpos_t pos) {
    cache_in_use_.erase(pos);
}

} // namespace sjtu

#endif // BUFFER_HPP