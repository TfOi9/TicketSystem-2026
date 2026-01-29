#ifndef BPT_HPP
#define BPT_HPP

#include <optional>
#include <string>

#include "../config.hpp"
#include "page.hpp"
#include "buffer.hpp"

namespace sjtu {
#define BPT_TYPE BPlusTree<KeyType, ValueType>
#define BPT_TEMPLATE_ARGS template<typename KeyType, typename ValueType>

BPT_TEMPLATE_ARGS
class BPlusTree {
private:
    BUFFER_MANAGER_TYPE buffer_;
    std::shared_ptr<const PAGE_TYPE> cur_;
    diskpos_t pos_;
    diskpos_t root_ = 0;

    void split();

    bool borrowl();

    bool borrowr();

    void merge();

    void balance();

public:
    BPlusTree(const std::string file_name = "bpt.dat");

    ~BPlusTree();

    std::optional<ValueType> find(const KeyType& key);

    void find_all(const KeyType& key, std::vector<ValueType>& vec);

    void insert(const KeyType& key, const ValueType& val);

    void erase(const KeyType& key, const ValueType& val);

};

BPT_TEMPLATE_ARGS
BPT_TYPE::BPlusTree(const std::string file_name) : buffer_(CACHE_CAPACITY, file_name) {
    root_ = buffer_.get_root_pos();
}

BPT_TEMPLATE_ARGS
BPT_TYPE::~BPlusTree() {
    buffer_.set_root_pos(root_);
}

BPT_TEMPLATE_ARGS
std::optional<ValueType> BPT_TYPE::find(const KeyType& key) {
    if (root_ == 0) {
        return std::nullopt;
    }
    pos_ = root_;
    cur_ = buffer_.get_page(pos_);
    while (cur_->type_ != PageType::Leaf) {
        int k = cur_->lower_bound(key);
        pos_ = cur_->ch_[k];
        cur_ = buffer_.get_page(pos_);
    }
    int k = cur_->lower_bound(key);
    if (cur_->data_[k].key_ != key) {
        return std::nullopt;
    }
    return cur_->data_[k].val_;
}

BPT_TEMPLATE_ARGS
void BPT_TYPE::find_all(const KeyType& key, std::vector<ValueType>& vec) {
    vec.clear();
    if (root_ == 0) {
        return;
    }
    pos_ = root_;
    cur_ = buffer_.get_page(pos_);
    while (cur_->type_ != PageType::Leaf) {
        int k = cur_->lower_bound(key);
        pos_ = cur_->ch_[k];
        cur_ = buffer_.get_page(pos_);
    }
    int k = cur_->lower_bound(key);
    if (cur_->data_[k].key_ != key) {
        return;
    }
    int curk = k;
    while (cur_->data_[curk].key_ == key) {
        vec.push_back(cur_->data_[curk].val_);
        if (curk < cur_->size_ - 1) {
            curk++;
        }
        else {
            if (cur_->right_ == -1) {
                break;
            }
            else {
                pos_ = cur_->right_;
                cur_ = buffer_.get_page(pos_);
                curk = 0;
            }
        }
    }
}

BPT_TEMPLATE_ARGS
void BPT_TYPE::split() {
    PAGE_TYPE newp;
    newp.size_ = PAGE_SLOT_COUNT / 2;
    auto cur_mut = buffer_.get_page_mutable(pos_);
    diskpos_t cur_pos = pos_;
    diskpos_t parent_pos = cur_mut->fa_;
    cur_mut->size_ = PAGE_SLOT_COUNT / 2;
    if (cur_mut->type_ == PageType::Leaf) {
        newp.type_ = PageType::Leaf;
    }
    else {
        newp.type_ = PageType::Internal;
    }
    newp.fa_ = parent_pos;
    newp.left_ = cur_pos;
    newp.right_ = cur_mut->right_;
    if (cur_mut->type_ == PageType::Leaf) {
        for (int i = 0; i < newp.size_; i++) {
            newp.data_[i] = cur_mut->data_[i + newp.size_];
        }
        KEYPAIR_TYPE split_at = cur_mut->back();
        KEYPAIR_TYPE max_pair = newp.back();
        if (parent_pos != -1) {
            auto f = buffer_.get_page_mutable(parent_pos);
            diskpos_t fa_pos = f->lower_bound(max_pair);
            for (int i = f->size_ - 1; i >= fa_pos; i--) {
                f->data_[i + 1] = f->data_[i];
                f->ch_[i + 1] = f->ch_[i];
            }
            diskpos_t newp_pos = buffer_.insert_page(newp);
            f->data_[fa_pos] = split_at;
            f->data_[fa_pos + 1] = max_pair;
            f->ch_[fa_pos] = cur_pos;
            f->ch_[fa_pos + 1] = newp_pos;
            f->size_++;
            if (cur_mut->right_ != -1) {
                auto rp = buffer_.get_page_mutable(cur_mut->right_);
                rp->left_ = newp_pos;
                buffer_.finish_use(cur_mut->right_);
            }
            cur_mut->right_ = newp_pos;
            bool need_split_parent = (f->size_ == PAGE_SLOT_COUNT);
            buffer_.finish_use(parent_pos);
            buffer_.finish_use(cur_pos);
            if (need_split_parent) {
                pos_ = parent_pos;
                split();
            }
        }
        else {
            PAGE_TYPE newr;
            newr.type_ = PageType::Internal;
            newr.size_ = 2;
            newr.data_[0] = split_at;
            newr.data_[1] = max_pair;
            newr.ch_[0] = cur_pos;
            diskpos_t newp_pos = buffer_.insert_page(newp);
            newr.ch_[1] = newp_pos;
            cur_mut->right_ = newp_pos;
            root_ = buffer_.insert_page(newr);
            cur_mut->fa_ = root_;
            auto newp_mut = buffer_.get_page_mutable(newp_pos);
            newp_mut->fa_ = root_;
            buffer_.finish_use(newp_pos);
            buffer_.finish_use(cur_pos);
        }
        return;
    }

    diskpos_t newp_pos = buffer_.insert_page(newp);
    auto newp_mut = buffer_.get_page_mutable(newp_pos);
    for (int i = 0; i < newp_mut->size_; i++) {
        newp_mut->data_[i] = cur_mut->data_[i + newp_mut->size_];
        newp_mut->ch_[i] = cur_mut->ch_[i + newp_mut->size_];
    }
    for (int i = 0; i < newp_mut->size_; i++) {
        auto ch = buffer_.get_page_mutable(newp_mut->ch_[i]);
        ch->fa_ = newp_pos;
        buffer_.finish_use(newp_mut->ch_[i]);
    }
    KEYPAIR_TYPE split_at = cur_mut->back();
    KEYPAIR_TYPE max_pair = newp_mut->back();
    if (parent_pos != -1) {
        auto f = buffer_.get_page_mutable(parent_pos);
        diskpos_t fa_pos = f->lower_bound(max_pair);
        for (int i = f->size_ - 1; i >= fa_pos; i--) {
            f->data_[i + 1] = f->data_[i];
            f->ch_[i + 1] = f->ch_[i];
        }
        f->data_[fa_pos] = split_at;
        f->data_[fa_pos + 1] = max_pair;
        f->ch_[fa_pos] = cur_pos;
        f->ch_[fa_pos + 1] = newp_pos;
        f->size_++;
        if (cur_mut->right_ != -1) {
            auto rp = buffer_.get_page_mutable(cur_mut->right_);
            rp->left_ = newp_pos;
            buffer_.finish_use(cur_mut->right_);
        }
        cur_mut->right_ = newp_pos;
        bool need_split_parent = (f->size_ == PAGE_SLOT_COUNT);
        buffer_.finish_use(parent_pos);
        buffer_.finish_use(cur_pos);
        buffer_.finish_use(newp_pos);
        if (need_split_parent) {
            pos_ = parent_pos;
            split();
        }
    }
    else {
        PAGE_TYPE newr;
        newr.type_ = PageType::Internal;
        newr.size_ = 2;
        newr.data_[0] = split_at;
        newr.data_[1] = max_pair;
        newr.ch_[0] = cur_pos;
        newr.ch_[1] = newp_pos;
        root_ = buffer_.insert_page(newr);
        cur_mut->fa_ = root_;
        newp_mut->fa_ = root_;
        buffer_.finish_use(cur_pos);
        buffer_.finish_use(newp_pos);
    }
}

BPT_TEMPLATE_ARGS
void BPT_TYPE::insert(const KeyType& key, const ValueType& val) {
    KEYPAIR_TYPE kp(key, val);
    if (root_ == 0) {
        PAGE_TYPE newr;
        newr.size_ = 1;
        newr.type_ = PageType::Leaf;
        newr.data_[0] = kp;
        root_ = buffer_.insert_page(newr);
        return;
    }
    pos_ = root_;
    cur_ = buffer_.get_page(pos_);
    while (cur_->type_ != PageType::Leaf) {
        auto cur_mut = buffer_.get_page_mutable(pos_);
        int k = cur_mut->lower_bound(kp);
        if (cur_mut->data_[k] < kp) {
            cur_mut->data_[k] = kp;
        }
        diskpos_t child = cur_mut->ch_[k];
        buffer_.finish_use(pos_);
        pos_ = child;
        cur_ = buffer_.get_page(pos_);
    }
    auto cur_mut = buffer_.get_page_mutable(pos_);
    int k = cur_mut->lower_bound(kp);
    if (cur_mut->data_[k] == kp) {
        buffer_.finish_use(pos_);
        return;
    }
    if (cur_mut->data_[k] < kp) {
        cur_mut->data_[k + 1] = kp;
        cur_mut->size_++;
    }
    else {
        for (int i = static_cast<int>(cur_mut->size_) - 1; i >= k; i--) {
            cur_mut->data_[i + 1] = cur_mut->data_[i];
        }
        cur_mut->data_[k] = kp;
        cur_mut->size_++;
    }
    bool need_split = (cur_mut->size_ == PAGE_SLOT_COUNT);
    buffer_.finish_use(pos_);
    if (need_split) {
        split();
    }
}

BPT_TEMPLATE_ARGS
void BPT_TYPE::erase(const KeyType& key, const ValueType& val) {
    if (root_ == 0) {
        return;
    }
    KEYPAIR_TYPE kp(key, val);
    pos_ = root_;
    cur_ = buffer_.get_page(pos_);
    while (cur_->type_ != PageType::Leaf) {
        int k = cur_->lower_bound(kp);
        pos_ = cur_->ch_[k];
        cur_ = buffer_.get_page(pos_);
    }
    auto cur_mut = buffer_.get_page_mutable(pos_);
    int k = cur_mut->lower_bound(kp);
    if (cur_mut->data_[k] != kp) {
        buffer_.finish_use(pos_);
        return;
    }
    for (int i = k; i < static_cast<int>(cur_mut->size_) - 1; i++) {
        cur_mut->data_[i] = cur_mut->data_[i + 1];
    }
    cur_mut->size_--;
    KEYPAIR_TYPE max_pair = cur_mut->back();
    diskpos_t cur_pos = pos_;
    diskpos_t fpos = cur_mut->fa_;
    buffer_.finish_use(cur_pos);
    while (fpos != -1) {
        auto f = buffer_.get_page_mutable(fpos);
        int p = f->lower_bound(kp);
        diskpos_t next_parent = f->fa_;
        if (f->data_[p] == kp) {
            f->data_[p] = max_pair;
            buffer_.finish_use(fpos);
            fpos = next_parent;
        }
        else {
            buffer_.finish_use(fpos);
            break;
        }
    }
    auto check_cur = buffer_.get_page(pos_);
    bool need_balance = (check_cur->size_ < PAGE_SLOT_COUNT / 2);
    if (need_balance) {
        balance();
    }
}

BPT_TEMPLATE_ARGS
bool BPT_TYPE::borrowl() {
    auto cur_mut = buffer_.get_page_mutable(pos_);
    diskpos_t cur_pos = pos_;
    if (cur_mut->fa_ == -1 || cur_mut->size_ == 0) {
        buffer_.finish_use(cur_pos);
        return false;
    }
    diskpos_t fpos = cur_mut->fa_;
    KEYPAIR_TYPE max_pair = cur_mut->back();
    auto f = buffer_.get_page_mutable(fpos);
    int k = f->lower_bound(max_pair);
    if (k == 0) {
        buffer_.finish_use(fpos);
        buffer_.finish_use(cur_pos);
        return false;
    }
    diskpos_t bpos = f->ch_[k - 1];
    auto bro = buffer_.get_page_mutable(bpos);
    if (bro->size_ <= PAGE_SLOT_COUNT / 2) {
        buffer_.finish_use(bpos);
        buffer_.finish_use(fpos);
        buffer_.finish_use(cur_pos);
        return false;
    }
    for (int i = static_cast<int>(cur_mut->size_) - 1; i >= 0; i--) {
        cur_mut->data_[i + 1] = cur_mut->data_[i];
        cur_mut->ch_[i + 1] = cur_mut->ch_[i];
    }
    cur_mut->data_[0] = bro->back();
    cur_mut->ch_[0] = bro->ch_[bro->size_ - 1];
    cur_mut->size_++;
    bro->size_--;
    if (cur_mut->type_ == PageType::Internal) {
        auto son = buffer_.get_page_mutable(cur_mut->ch_[0]);
        son->fa_ = cur_pos;
        buffer_.finish_use(cur_mut->ch_[0]);
    }
    f->data_[k - 1] = bro->back();
    buffer_.finish_use(bpos);
    buffer_.finish_use(fpos);
    buffer_.finish_use(cur_pos);
    return true;
}

BPT_TEMPLATE_ARGS
bool BPT_TYPE::borrowr() {
    auto cur_mut = buffer_.get_page_mutable(pos_);
    diskpos_t cur_pos = pos_;
    if (cur_mut->fa_ == -1 || cur_mut->size_ == 0) {
        buffer_.finish_use(cur_pos);
        return false;
    }
    diskpos_t fpos = cur_mut->fa_;
    KEYPAIR_TYPE max_pair = cur_mut->back();
    auto f = buffer_.get_page_mutable(fpos);
    int k = f->lower_bound(max_pair);
    if (k == static_cast<int>(f->size_) - 1) {
        buffer_.finish_use(fpos);
        buffer_.finish_use(cur_pos);
        return false;
    }
    diskpos_t bpos = f->ch_[k + 1];
    auto bro = buffer_.get_page_mutable(bpos);
    if (bro->size_ <= PAGE_SLOT_COUNT / 2) {
        buffer_.finish_use(bpos);
        buffer_.finish_use(fpos);
        buffer_.finish_use(cur_pos);
        return false;
    }
    cur_mut->data_[cur_mut->size_] = bro->data_[0];
    cur_mut->ch_[cur_mut->size_] = bro->ch_[0];
    cur_mut->size_++;
    for (int i = 0; i < static_cast<int>(bro->size_) - 1; i++) {
        bro->data_[i] = bro->data_[i + 1];
        bro->ch_[i] = bro->ch_[i + 1];
    }
    bro->size_--;
    if (cur_mut->type_ == PageType::Internal) {
        auto son = buffer_.get_page_mutable(cur_mut->ch_[cur_mut->size_ - 1]);
        son->fa_ = cur_pos;
        buffer_.finish_use(cur_mut->ch_[cur_mut->size_ - 1]);
    }
    f->data_[k] = cur_mut->back();
    buffer_.finish_use(bpos);
    buffer_.finish_use(fpos);
    buffer_.finish_use(cur_pos);
    return true;
}

BPT_TEMPLATE_ARGS
void BPT_TYPE::merge() {
    auto cur_mut = buffer_.get_page_mutable(pos_);
    diskpos_t cur_pos = pos_;
    if (cur_mut->fa_ == -1) {
        buffer_.finish_use(cur_pos);
        return;
    }
    KEYPAIR_TYPE max_pair = cur_mut->back();
    diskpos_t fpos = cur_mut->fa_;
    auto f = buffer_.get_page_mutable(fpos);
    int k = f->lower_bound(max_pair);
    if (k) {
        diskpos_t bpos = f->ch_[k - 1];
        auto bro = buffer_.get_page_mutable(bpos);
        if (cur_mut->type_ == PageType::Internal) {
            for (int i = 0; i < static_cast<int>(cur_mut->size_); i++) {
                auto son = buffer_.get_page_mutable(cur_mut->ch_[i]);
                son->fa_ = bpos;
                buffer_.finish_use(cur_mut->ch_[i]);
            }
        }
        for (int i = 0; i < static_cast<int>(cur_mut->size_); i++) {
            bro->data_[bro->size_ + i] = cur_mut->data_[i];
            bro->ch_[bro->size_ + i] = cur_mut->ch_[i];
        }
        bro->size_ += cur_mut->size_;
        cur_mut->size_ = 0;
        bro->right_ = cur_mut->right_;
        if (cur_mut->right_ != -1) {
            auto rp = buffer_.get_page_mutable(cur_mut->right_);
            rp->left_ = bpos;
            buffer_.finish_use(cur_mut->right_);
        }
        for (int i = k; i < static_cast<int>(f->size_) - 1; i++) {
            f->data_[i] = f->data_[i + 1];
            f->ch_[i] = f->ch_[i + 1];
        }
        f->size_--;
        f->data_[k - 1] = bro->back();
        bool need_balance = (f->size_ < PAGE_SLOT_COUNT / 2);
        buffer_.finish_use(bpos);
        buffer_.finish_use(fpos);
        buffer_.finish_use(cur_pos);
        if (need_balance) {
            pos_ = fpos;
            balance();
        }
    }
    else if (k != static_cast<int>(f->size_) - 1) {
        diskpos_t bpos = f->ch_[k + 1];
        auto bro = buffer_.get_page_mutable(bpos);
        if (cur_mut->type_ == PageType::Internal) {
            for (int i = 0; i < static_cast<int>(bro->size_); i++) {
                auto son = buffer_.get_page_mutable(bro->ch_[i]);
                son->fa_ = cur_pos;
                buffer_.finish_use(bro->ch_[i]);
            }
        }
        for (int i = 0; i < static_cast<int>(bro->size_); i++) {
            cur_mut->data_[cur_mut->size_ + i] = bro->data_[i];
            cur_mut->ch_[cur_mut->size_ + i] = bro->ch_[i];
        }
        cur_mut->size_ += bro->size_;
        bro->size_ = 0;
        cur_mut->right_ = bro->right_;
        if (bro->right_ != -1) {
            auto rp = buffer_.get_page_mutable(bro->right_);
            rp->left_ = cur_pos;
            buffer_.finish_use(bro->right_);
        }
        for (int i = k + 1; i < static_cast<int>(f->size_) - 1; i++) {
            f->data_[i] = f->data_[i + 1];
            f->ch_[i] = f->ch_[i + 1];
        }
        f->size_--;
        f->data_[k] = cur_mut->back();
        bool need_balance = (f->size_ < PAGE_SLOT_COUNT / 2);
        buffer_.finish_use(bpos);
        buffer_.finish_use(fpos);
        buffer_.finish_use(cur_pos);
        if (need_balance) {
            pos_ = fpos;
            balance();
        }
    }
    else {
        buffer_.finish_use(fpos);
        buffer_.finish_use(cur_pos);
    }
}

BPT_TEMPLATE_ARGS
void BPT_TYPE::balance() {
    auto cur_mut = buffer_.get_page_mutable(pos_);
    diskpos_t cur_pos = pos_;
    if (cur_mut->fa_ == -1) {
        if (cur_mut->size_ == 0) {
            root_ = 0;
        }
        if (cur_mut->type_ == PageType::Internal && cur_mut->size_ == 1) {
            diskpos_t child = cur_mut->ch_[0];
            auto son = buffer_.get_page_mutable(child);
            son->fa_ = -1;
            buffer_.finish_use(child);
            root_ = child;
        }
        buffer_.finish_use(cur_pos);
        return;
    }
    buffer_.finish_use(cur_pos);
    if (borrowl()) {
        return;
    }
    if (borrowr()) {
        return;
    }
    merge();
}

} // namespace sjtu

#endif // BPT_HPP