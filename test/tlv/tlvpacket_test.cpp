#include <cassert>
#include <iostream>
#include <cstring>
#include "../frontend/include/web/tlv/tlvpacket.hpp"

using sjtu::TLVPacket;

int main() {
    {
        uint32_t t = 0x12345678;
        uint32_t v = 0xCAFEBABE;
        TLVPacket p(t, v);
        assert(p.type() == t);
        assert(p.size() == sizeof(v));
        uint32_t read = 0;
        memcpy(&read, p.data(), sizeof(read));
        assert(read == v);
    }

    {
        struct S { int a; double b; } s{42, 3.14};
        TLVPacket p(2, s);
        assert(p.type() == 2);
        assert(p.size() == sizeof(s));
        S r;
        memcpy(&r, p.data(), sizeof(r));
        assert(r.a == 42);
        // move
        TLVPacket m = std::move(p);
        assert(m.type() == 2);
    }

    return 0;
}
