#include <cassert>
#include <iostream>

#include <QCoreApplication>
#include <QDataStream>
#include <QTimer>

#include "web/tcp/tcpclient.hpp"
#include "web/tcp/tcpserver.hpp"

namespace {

constexpr uint32_t kTypeText = sjtu::TCPClient::kTextMessageType;
constexpr uint32_t kTypeUser = 1001;
constexpr uint32_t kTypeOrder = 1002;

struct UserInfo {
    qint32 userId;
    QString name;
    bool vip;
};

struct OrderInfo {
    qint64 orderId;
    QString train;
    qint32 seatCount;
};

QByteArray serializeUserInfo(const UserInfo& user) {
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_2);
    out << user.userId << user.name << user.vip;
    return payload;
}

bool parseUserInfo(const QByteArray& payload, UserInfo& out) {
    QDataStream in(payload);
    in.setVersion(QDataStream::Qt_6_2);
    in >> out.userId >> out.name >> out.vip;
    return in.status() == QDataStream::Ok;
}

QByteArray serializeOrderInfo(const OrderInfo& order) {
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_2);
    out << order.orderId << order.train << order.seatCount;
    return payload;
}

bool parseOrderInfo(const QByteArray& payload, OrderInfo& out) {
    QDataStream in(payload);
    in.setVersion(QDataStream::Qt_6_2);
    in >> out.orderId >> out.train >> out.seatCount;
    return in.status() == QDataStream::Ok;
}

} // namespace

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);

    sjtu::TCPServer server;
    const quint16 port = 34789;
    if (!server.start(port)) {
        std::cerr << "Server failed to start on port " << port << ", skipping test\n";
        return 0;
    }

    sjtu::TCPClient client;

    const UserInfo userToServer{42, "Alice", true};
    const OrderInfo orderToServer{778899, "G123", 3};
    const UserInfo userFromServer{7, "Bob", false};
    const OrderInfo orderFromServer{998877, "D456", 2};

    int textAckCount = 0;
    bool gotUserFromServer = false;
    bool gotOrderFromServer = false;
    bool serverGotUser = false;
    bool serverGotOrder = false;

    bool serverUserCodecOK = server.registerPacketCodec<UserInfo>(
        kTypeUser,
        serializeUserInfo,
        parseUserInfo,
        [&](QTcpSocket* sock, const UserInfo& user) {
            assert(user.userId == userToServer.userId);
            assert(user.name == userToServer.name);
            assert(user.vip == userToServer.vip);
            serverGotUser = true;
            bool sent = server.sendObject(kTypeOrder, orderFromServer, sock);
            assert(sent);
        }
    );
    assert(serverUserCodecOK);

    bool serverOrderCodecOK = server.registerPacketCodec<OrderInfo>(
        kTypeOrder,
        serializeOrderInfo,
        parseOrderInfo,
        [&](QTcpSocket* sock, const OrderInfo& order) {
            assert(order.orderId == orderToServer.orderId);
            assert(order.train == orderToServer.train);
            assert(order.seatCount == orderToServer.seatCount);
            serverGotOrder = true;
            bool sent = server.sendObject(kTypeUser, userFromServer, sock);
            assert(sent);
        }
    );
    assert(serverOrderCodecOK);

    bool serverTextReceiverOK = server.registerPacketReceiver<QString>(
        kTypeText,
        [](const QByteArray& payload, QString& out) {
            out = QString::fromUtf8(payload);
            return true;
        },
        [&](QTcpSocket* sock, const QString& text) {
            server.send(QString("ack:%1").arg(text), sock);
        }
    );
    assert(serverTextReceiverOK);

    bool clientUserCodecOK = client.registerPacketCodec<UserInfo>(
        kTypeUser,
        serializeUserInfo,
        parseUserInfo,
        [&](const UserInfo& user) {
            assert(user.userId == userFromServer.userId);
            assert(user.name == userFromServer.name);
            assert(user.vip == userFromServer.vip);
            gotUserFromServer = true;
        }
    );
    assert(clientUserCodecOK);

    bool clientOrderCodecOK = client.registerPacketCodec<OrderInfo>(
        kTypeOrder,
        serializeOrderInfo,
        parseOrderInfo,
        [&](const OrderInfo& order) {
            assert(order.orderId == orderFromServer.orderId);
            assert(order.train == orderFromServer.train);
            assert(order.seatCount == orderFromServer.seatCount);
            gotOrderFromServer = true;
        }
    );
    assert(clientOrderCodecOK);

    QObject::connect(&client, &sjtu::TCPClient::connected, [&]() {
        // Send mixed packet types continuously to test sticky-packet safety.
        client.send("hello");
        bool sentUser = client.sendObject(kTypeUser, userToServer);
        bool sentOrder = client.sendObject(kTypeOrder, orderToServer);
        assert(sentUser);
        assert(sentOrder);
        client.send("bye");
    });

    QObject::connect(&client, &sjtu::TCPClient::received, [&](const QString& msg) {
        if (msg == "ack:hello" || msg == "ack:bye") {
            ++textAckCount;
        }
    });

    QTimer::singleShot(3000, [&]() {
        assert(textAckCount == 2);
        assert(serverGotUser);
        assert(serverGotOrder);
        assert(gotUserFromServer);
        assert(gotOrderFromServer);
        client.disconnect();
        app.quit();
    });

    QTimer::singleShot(8000, [&]() {
        assert(false && "tcp_tlv_mixed_test timeout");
    });

    client.connectToServer("127.0.0.1", port);

    app.exec();
    std::cout << "TCP TLV mixed test passed\n";
    return 0;
}
