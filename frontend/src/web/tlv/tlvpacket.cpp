#include "../../../include/web/tlv/tlvpacket.hpp"
#include <stdexcept>

namespace sjtu {

TLVPacket::TLVPacket(TLVPacket&& other)
        : type_(other.type_), size_(other.size_) {
    data_ = other.data_;
    other.data_ = nullptr;
}

TLVPacket::TLVPacket(uint32_t type, uint32_t size, const char *arr)
        : type_(type), size_(size) {
    data_ = new char[size_];
    memcpy(data_, arr, size_);
}

TLVPacket::~TLVPacket() {
    delete []data_;
}

uint32_t TLVPacket::type() const {
    return type_;
}

uint32_t TLVPacket::size() const {
    return size_;
}

const char* TLVPacket::data() const {
    return data_;
}

QByteArray TLVPacket::serialize() {
    QByteArray arr;
    arr.reserve(sizeof(uint32_t) * 2 + size_);
    arr.append(reinterpret_cast<const char *>(&type_), sizeof(uint32_t));
    arr.append(reinterpret_cast<const char *>(&size_), sizeof(uint32_t));
    arr.append(data_, size_);
    return arr;
}

} // namespace sjtu