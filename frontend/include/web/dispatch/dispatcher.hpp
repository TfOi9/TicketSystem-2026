#ifndef DISPATCHER_HPP
#define DISPATCHER_HPP

#include <QMap>
#include <QByteArray>

#include <cstdint>
#include <functional>
#include <utility>
#include <memory>

#include "../tlv/tlvpacket.hpp"

namespace sjtu {

class Dispatcher {

public:
    template<typename T>
    bool registerHandler(uint32_t type, std::function<void(const char*, T&)> anti_serializer, std::function<void(const T&)> handler) {
        if (handlers.contains(type)) {
            return false;
        }
        handlers[type] = std::make_shared<Handler<T>>(
            [anti_serializer](const char* data, uint32_t, T& out) {
                anti_serializer(data, out);
                return true;
            },
            handler
        );
        return true;
    }

    template<typename T>
    bool registerHandlerWithPayload(uint32_t type, std::function<bool(const QByteArray&, T&)> anti_serializer, std::function<void(const T&)> handler) {
        if (handlers.contains(type)) {
            return false;
        }
        handlers[type] = std::make_shared<Handler<T>>(
            [anti_serializer](const char* data, uint32_t size, T& out) {
                return anti_serializer(QByteArray(data, static_cast<qsizetype>(size)), out);
            },
            handler
        );
        return true;
    }

    bool dispatch(uint32_t type, const char *data);

    bool dispatch(uint32_t type, const char *data, uint32_t size);

    bool dispatch(uint32_t type, const QByteArray& payload);

    bool dispatch(const TLVPacket& packet);

private:
    struct HandlerBase {
        virtual ~HandlerBase() = default;

        virtual bool execute(const char* data, uint32_t size) = 0;

    };
    
    template<typename T>
    struct Handler : HandlerBase {
        std::function<bool(const char*, uint32_t, T&)> anti_serializer;
        std::function<void(const T&)> processor;

        Handler(std::function<bool(const char*, uint32_t, T&)> a, std::function<void(const T&)> p)
            : anti_serializer(a), processor(p) {}

        bool execute(const char* data, uint32_t size) override {
            T obj;
            if (!anti_serializer(data, size, obj)) {
                return false;
            }
            processor(obj);
            return true;
        }
    };
    
    QMap<uint32_t, std::shared_ptr<HandlerBase>> handlers;

};

} // namespace sjtu

#endif // DISPATCHER_HPP