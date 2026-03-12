#include "../../../include/web/tlv/tlvparser.hpp"
#include <stdexcept>

namespace sjtu {

void TLVParser::appendData(const QByteArray& data) {
    buffer_.append(data);
    if (status_ == TLVParserStatus::PendingForHeader) {
        if (buffer_.size() >= sizeof(uint32_t) * 2) {
            status_ = TLVParserStatus::PendingForData;
            memcpy(&type_, buffer_.constData(), sizeof(uint32_t));
            memcpy(&expected_size_, buffer_.constData() + sizeof(uint32_t), sizeof(uint32_t));
        }
    }
}

bool TLVParser::hasCompletePacket() const {
    return status_ == TLVParserStatus::PendingForData
        && buffer_.size() >= sizeof(uint32_t) * 2 + expected_size_;
}

TLVPacket TLVParser::nextPacket() {
    if (!hasCompletePacket()) {
        throw std::runtime_error("Truncated packet");
    }
    TLVPacket packet(type_, expected_size_, buffer_.constData() + sizeof(uint32_t) * 2);
    buffer_.remove(0, expected_size_ + sizeof(uint32_t) * 2);
    if (buffer_.size() >= sizeof(uint32_t) * 2) {
        memcpy(&type_, buffer_.constData(), sizeof(uint32_t));
        memcpy(&expected_size_, buffer_.constData() + sizeof(uint32_t), sizeof(uint32_t));
    }
    else {
        type_ = 0;
        expected_size_ = 0;
        status_ = TLVParserStatus::PendingForHeader;
    }
    return packet;
}

void TLVParser::reset() {
    buffer_.clear();
    status_ = TLVParserStatus::PendingForHeader;
    type_ = 0;
    expected_size_ = 0;
}

size_t TLVParser::bufferSize() const {
    return buffer_.size();
}

} // namespace sjtu