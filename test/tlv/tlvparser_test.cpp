#include <cassert>
#include <iostream>
#include <QByteArray>
#include "../frontend/include/web/tlv/tlvparser.hpp"
#include "../frontend/include/web/tlv/tlvpacket.hpp"

using sjtu::TLVParser;
using sjtu::TLVPacket;

void checkPacketEquals(const TLVPacket& a, uint32_t type, uint32_t size, const char* data) {
    assert(a.type() == type);
    assert(a.size() == size);
    if (size > 0) {
        assert(std::memcmp(a.data(), data, size) == 0);
    }
}

int main() {
    // single complete packet
    {
        TLVPacket p(0x10, uint32_t(4), "ABCD");
        TLVParser parser;
        parser.reset();
        parser.appendData(p.serialize());
        assert(parser.hasCompletePacket());
        TLVPacket out = parser.nextPacket();
        checkPacketEquals(out, 0x10, 4, "ABCD");
        assert(parser.bufferSize() == 0);
    }

    // fragmented header then data
    {
        TLVPacket p(0x11, uint32_t(3), "xyz");
        QByteArray serialized = p.serialize();
        TLVParser parser;
        parser.reset();
        // append only first 5 bytes (less than header 8)
        parser.appendData(serialized.left(5));
        assert(!parser.hasCompletePacket());
        // append remaining
        parser.appendData(serialized.mid(5));
        assert(parser.hasCompletePacket());
        TLVPacket out = parser.nextPacket();
        checkPacketEquals(out, 0x11, 3, "xyz");
    }

    // fragmented data
    {
        TLVPacket p(0x12, uint32_t(6), "hello!");
        QByteArray serialized = p.serialize();
        TLVParser parser;
        parser.reset();
        // append full header
        parser.appendData(serialized.left(8));
        assert(!parser.hasCompletePacket());
        // append partial data
        parser.appendData(serialized.mid(8, 3));
        assert(!parser.hasCompletePacket());
        parser.appendData(serialized.mid(11));
        assert(parser.hasCompletePacket());
        TLVPacket out = parser.nextPacket();
        checkPacketEquals(out, 0x12, 6, "hello!");
    }

    // multiple packets in buffer
    {
        TLVPacket a(0x20, uint32_t(2), "OK");
        TLVPacket b(0x21, uint32_t(5), "World");
        QByteArray combined = a.serialize() + b.serialize();
        TLVParser parser;
        parser.reset();
        parser.appendData(combined);
        assert(parser.hasCompletePacket());
        TLVPacket out1 = parser.nextPacket();
        checkPacketEquals(out1, 0x20, 2, "OK");
        assert(parser.hasCompletePacket());
        TLVPacket out2 = parser.nextPacket();
        checkPacketEquals(out2, 0x21, 5, "World");
        assert(parser.bufferSize() == 0);
    }

    // reset clears buffer
    {
        TLVPacket p(0x30, uint32_t(1), "x");
        TLVParser parser;
        parser.reset();
        parser.appendData(p.serialize());
        assert(parser.hasCompletePacket());
        parser.reset();
        assert(!parser.hasCompletePacket());
        assert(parser.bufferSize() == 0);
    }

    std::cout << "TLVParser tests passed\n";
    return 0;
}
