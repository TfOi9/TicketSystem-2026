#include "../../../include/web/dispatch/dispatcher.hpp"

namespace sjtu {

bool Dispatcher::dispatch(uint32_t type, const char *data) {
    return dispatch(type, data, 0);
}

bool Dispatcher::dispatch(uint32_t type, const char *data, uint32_t size) {
    if (handlers.contains(type)) {
        return handlers[type]->execute(data, size);
    }
    return false;
}

bool Dispatcher::dispatch(uint32_t type, const QByteArray& payload) {
    return dispatch(type, payload.constData(), static_cast<uint32_t>(payload.size()));
}

bool Dispatcher::dispatch(const TLVPacket& packet) {
    return dispatch(packet.type(), packet.data(), packet.size());
}

} // namespace sjtu