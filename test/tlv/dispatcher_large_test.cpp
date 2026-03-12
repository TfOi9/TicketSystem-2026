#include <cassert>
#include <iostream>
#include <chrono>
#include <atomic>
#include "../frontend/include/web/dispatch/dispatcher.hpp"
#include "../frontend/include/web/tlv/tlvpacket.hpp"

using sjtu::Dispatcher;
using sjtu::TLVPacket;

int main() {
    Dispatcher d;
    const uint32_t N = 10000;
    std::atomic<uint32_t> count{0};

    auto anti = [](const char* data, uint32_t& out) {
        std::memcpy(&out, data, sizeof(out));
    };

    for (uint32_t i = 1; i <= N; ++i) {
        bool ok = d.registerHandler<uint32_t>(i, anti, [&count, i](const uint32_t& v) {
            if (v == i) ++count;
        });
        assert(ok);
    }

    auto start = std::chrono::steady_clock::now();
    for (uint32_t i = 1; i <= N; ++i) {
        TLVPacket p(i, i);
        bool disp = d.dispatch(p);
        assert(disp);
    }
    auto end = std::chrono::steady_clock::now();

    assert(count == N);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Dispatched " << N << " messages in " << ms << " ms\n";
    return 0;
}
