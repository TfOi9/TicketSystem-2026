#include <cassert>
#include <iostream>
#include "../frontend/include/web/dispatch/dispatcher.hpp"
#include "../frontend/include/web/tlv/tlvpacket.hpp"

using sjtu::Dispatcher;
using sjtu::TLVPacket;

int main() {
    Dispatcher d;
    bool called = false;

    auto anti = [](const char* data, int& out) {
        std::memcpy(&out, data, sizeof(out));
    };

    auto handler = [&](const int& v) {
        if (v == 0x42) called = true;
    };

    bool ok = d.registerHandler<int>(1, anti, handler);
    assert(ok);

    int payload = 0x42;
    TLVPacket p(1, payload);
    bool disp = d.dispatch(p);
    assert(disp);
    assert(called);

    return 0;
}
