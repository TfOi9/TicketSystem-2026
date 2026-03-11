#ifndef SENDER_HPP
#define SENDER_HPP

#include <QByteArray>
#include <QMap>

#include <cstdint>
#include <functional>
#include <memory>

namespace sjtu {

class Sender {
public:
    template<typename T>
    bool registerSender(uint32_t type, std::function<QByteArray(const T&)> serializer) {
        if (senders_.contains(type)) {
            return false;
        }
        senders_[type] = std::make_shared<SenderImpl<T>>(
            [serializer](const T& obj, QByteArray& payload) {
                payload = serializer(obj);
                return true;
            }
        );
        return true;
    }

    template<typename T>
    bool registerSenderWithPayload(uint32_t type, std::function<bool(const T&, QByteArray&)> serializer) {
        if (senders_.contains(type)) {
            return false;
        }
        senders_[type] = std::make_shared<SenderImpl<T>>(serializer);
        return true;
    }

    bool removeSender(uint32_t type) {
        return senders_.remove(type) > 0;
    }

    template<typename T>
    bool send(uint32_t type, const T& obj, QByteArray& outPayload) const {
        auto it = senders_.find(type);
        if (it == senders_.end()) {
            return false;
        }
        return it.value()->serialize(static_cast<const void*>(&obj), outPayload);
    }

private:
    struct SenderBase {
        virtual ~SenderBase() = default;

        virtual bool serialize(const void* data, QByteArray& payload) const = 0;
    };

    template<typename T>
    struct SenderImpl : SenderBase {
        std::function<bool(const T&, QByteArray&)> serializer;

        explicit SenderImpl(std::function<bool(const T&, QByteArray&)> s) : serializer(std::move(s)) {}

        bool serialize(const void* data, QByteArray& payload) const override {
            if (data == nullptr) {
                return false;
            }
            return serializer(*static_cast<const T*>(data), payload);
        }
    };

    QMap<uint32_t, std::shared_ptr<SenderBase>> senders_;
};

} // namespace sjtu

#endif // SENDER_HPP
