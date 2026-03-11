#ifndef TLVPARSER_HPP
#define TLVPARSER_HPP

#include <cstdint>
#include <QByteArray>

#include "tlvpacket.hpp"

namespace sjtu {

class TLVParser {
private:
    QByteArray buffer_;
    enum class TLVParserStatus {
        PendingForHeader = 0,
        PendingForData
    } status_;
    uint32_t type_ = 0;
    uint32_t expected_size_ = 0;

public:
    TLVParser() : status_(TLVParserStatus::PendingForHeader) {}

    void appendData(const QByteArray& data);

    bool hasCompletePacket() const;

    TLVPacket nextPacket();

    void reset();

    size_t bufferSize() const;

};

} // namespace sjtu

#endif // TLVPARSER_HPP