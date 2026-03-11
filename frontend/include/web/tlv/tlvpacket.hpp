#ifndef TLVPACKET_HPP
#define TLVPACKET_HPP

#include <cstdint>
#include <QByteArray>
#include <cstring>

namespace sjtu {

class TLVPacket {
private:
    uint32_t type_;
    uint32_t size_;
    char *data_;

public:
    TLVPacket() = delete;
    TLVPacket(const TLVPacket& other) = delete;

    TLVPacket(TLVPacket&& other);

    TLVPacket(uint32_t type, uint32_t size, const char *arr);

    template<typename T>
    TLVPacket(uint32_t type, T data);

    template<typename T, typename Serializer>
    TLVPacket(uint32_t type, const T& data, Serializer serializer);

    ~TLVPacket();

    uint32_t type() const;

    uint32_t size() const;

    const char *data() const;

    QByteArray serialize();

};

template<typename T>
TLVPacket::TLVPacket(uint32_t type, T data)
        : type_(type), size_(sizeof(data)) {
    data_ = new char[size_];
    memcpy(data_, &data, size_);
}

template<typename T, typename Serializer>
TLVPacket::TLVPacket(uint32_t type, const T& data, Serializer serializer)
        : type_(type) {
    QByteArray arr = serializer(data);
    size_ = static_cast<uint32_t>(arr.size());
    data_ = new char[size_];
    if (size_ > 0) memcpy(data_, arr.constData(), size_);
}

} // namespace sjtu

#endif // TLVPACKET_HPP