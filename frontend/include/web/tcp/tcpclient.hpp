#ifndef TCPCLIENT_HPP
#define TCPCLIENT_HPP

#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include <QtCore/qtmetamacros.h>
#include <QtNetwork/qabstractsocket.h>

#include <functional>

#include "../dispatch/dispatcher.hpp"
#include "../dispatch/sender.hpp"
#include "../tlv/tlvparser.hpp"

namespace sjtu {

class TCPClient : public QObject {
    Q_OBJECT

public:
    static constexpr uint32_t kTextMessageType = 1;

    explicit TCPClient(QObject *parent = nullptr);

    void connectToServer(const QString& host, quint16 port);
    void disconnect();
    void send(const QString& msg);
    void sendPacket(uint32_t type, const QByteArray& payload);

    template<typename T>
    bool registerPacketSender(uint32_t type, std::function<QByteArray(const T&)> serializer) {
        return sender_.registerSender<T>(type, serializer);
    }

    template<typename T>
    bool registerPacketSenderWithPayload(uint32_t type, std::function<bool(const T&, QByteArray&)> serializer) {
        return sender_.registerSenderWithPayload<T>(type, serializer);
    }

    template<typename T>
    bool registerPacketReceiver(uint32_t type, std::function<bool(const QByteArray&, T&)> antiSerializer, std::function<void(const T&)> handler) {
        return dispatcher_.registerHandlerWithPayload<T>(type, antiSerializer, handler);
    }

    template<typename T>
    bool registerPacketCodec(uint32_t type,
                             std::function<QByteArray(const T&)> serializer,
                             std::function<bool(const QByteArray&, T&)> antiSerializer,
                             std::function<void(const T&)> handler) {
        bool sendOK = registerPacketSender<T>(type, serializer);
        if (!sendOK) {
            return false;
        }
        bool recvOK = registerPacketReceiver<T>(type, antiSerializer, handler);
        if (!recvOK) {
            sender_.removeSender(type);
            return false;
        }
        return true;
    }

    template<typename T>
    bool sendObject(uint32_t type, const T& obj) {
        QByteArray payload;
        if (!sender_.send<T>(type, obj, payload)) {
            return false;
        }
        sendPacket(type, payload);
        return true;
    }

    bool isConnected() const;

signals:
    void connected();
    void disconnected();
    void received(const QString& msg);
    void packetReceived(uint32_t type, const QByteArray& payload);
    void error(const QString& err);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError err);

private:
    QTcpSocket *socket;
    TLVParser parser_;
    Dispatcher dispatcher_;
    Sender sender_;
    
};

} // namespace sjtu

#endif // TCPCLIENT_HPP