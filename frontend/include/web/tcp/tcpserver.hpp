#ifndef TCPSERVER_HPP
#define TCPSERVER_HPP

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QHash>
#include <QByteArray>
#include <QString>

#include <functional>

#include "../dispatch/dispatcher.hpp"
#include "../dispatch/sender.hpp"
#include "../tlv/tlvparser.hpp"

namespace sjtu {

class TCPServer : public QObject {
    Q_OBJECT

public:
    static constexpr uint32_t kTextMessageType = 1;
    static constexpr uint32_t kResultMessageType = 2;

    explicit TCPServer(QObject *parent = nullptr);
    ~TCPServer();

    bool start(quint16 port);
    void broadcast(const QString& msg);
    void broadcastPacket(uint32_t type, const QByteArray& payload);
    void send(const QString& msg, QTcpSocket *client);
    void sendPacket(uint32_t type, const QByteArray& payload, QTcpSocket *client);

    template<typename T>
    bool registerPacketSender(uint32_t type, std::function<QByteArray(const T&)> serializer) {
        return sender_.registerSender<T>(type, serializer);
    }

    template<typename T>
    bool registerPacketSenderWithPayload(uint32_t type, std::function<bool(const T&, QByteArray&)> serializer) {
        return sender_.registerSenderWithPayload<T>(type, serializer);
    }

    template<typename T>
    bool registerPacketReceiver(uint32_t type,
                                std::function<bool(const QByteArray&, T&)> antiSerializer,
                                std::function<void(QTcpSocket*, const T&)> handler) {
        return dispatcher_.registerHandlerWithPayload<T>(
            type,
            antiSerializer,
            [this, handler](const T& obj) {
                if (currentDispatchClient_ != nullptr) {
                    handler(currentDispatchClient_, obj);
                }
            }
        );
    }

    template<typename T>
    bool registerPacketCodec(uint32_t type,
                             std::function<QByteArray(const T&)> serializer,
                             std::function<bool(const QByteArray&, T&)> antiSerializer,
                             std::function<void(QTcpSocket*, const T&)> handler) {
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
    bool sendObject(uint32_t type, const T& obj, QTcpSocket* client) {
        QByteArray payload;
        if (!sender_.send<T>(type, obj, payload)) {
            return false;
        }
        sendPacket(type, payload, client);
        return true;
    }

    template<typename T>
    bool broadcastObject(uint32_t type, const T& obj) {
        QByteArray payload;
        if (!sender_.send<T>(type, obj, payload)) {
            return false;
        }
        broadcastPacket(type, payload);
        return true;
    }

signals:
    void packetReceived(QTcpSocket *client, uint32_t type, const QByteArray& payload);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onClientDisconnected();

private:
    QTcpServer *server;
    QList<QTcpSocket*> clients;
    QHash<QTcpSocket*, TLVParser> parsers_;
    Dispatcher dispatcher_;
    Sender sender_;
    QTcpSocket* currentDispatchClient_ = nullptr;

};

} // namespace sjtu

#endif // TCPSERVER_HPP