#include "../../../include/web/tcp/tcpserver.hpp"
#include "../../../include/web/tlv/tlvpacket.hpp"
#include <QtCore/qbytearray.h>
#include <QtCore/qlogging.h>
#include <QtNetwork/qhostaddress.h>
#include <iostream>

namespace sjtu {

TCPServer::TCPServer(QObject *parent) {
    server = new QTcpServer(this);
    connect(server, &QTcpServer::newConnection, this, &TCPServer::onNewConnection);
}

TCPServer::~TCPServer() {
    for (QTcpSocket *client : clients) {
        client->close();
        client->deleteLater();
    }
    server->close();
}

bool TCPServer::start(quint16 port) {
    bool result = server->listen(QHostAddress::Any, port);
    if (result) {
        qDebug() << "Server started on port " << port;
    }
    else {
        qDebug() << "Failed to start server: " << server->errorString();
    }
    return result;
}

void TCPServer::onNewConnection() {
    while (server->hasPendingConnections()) {
        QTcpSocket *socket = server->nextPendingConnection();
        clients.append(socket);
        parsers_.insert(socket, TLVParser());
        qDebug() << "New client connected: " << socket->peerAddress().toString();
        connect(socket, &QTcpSocket::readyRead, this, &TCPServer::onReadyRead);
        connect(socket, &QTcpSocket::disconnected, this, &TCPServer::onClientDisconnected);
        send("Welcome! You are connected to server.", socket);
    }
}

void TCPServer::onReadyRead() {
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    TLVParser& parser = parsers_[socket];
    parser.appendData(socket->readAll());

    while (parser.hasCompletePacket()) {
        TLVPacket packet = parser.nextPacket();
        QByteArray payload(packet.data(), static_cast<qsizetype>(packet.size()));

        emit packetReceived(socket, packet.type(), payload);
        currentDispatchClient_ = socket;
        dispatcher_.dispatch(packet.type(), payload);
        currentDispatchClient_ = nullptr;
        if (packet.type() == kTextMessageType) {
            QString msg = QString::fromUtf8(payload);
            qDebug() << "Text data received from client:" << msg;
            std::cout << "Received from " << socket->peerAddress().toString().toStdString() << ": " << msg.toStdString() << std::endl;
        }
    }
}

void TCPServer::onClientDisconnected() {
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    clients.removeOne(socket);
    parsers_.remove(socket);
    qDebug() << "Cliend disconnected: " << socket->peerAddress().toString();
    socket->deleteLater();
}

void TCPServer::broadcast(const QString &msg) {
    broadcastPacket(kTextMessageType, msg.toUtf8());
}

void TCPServer::broadcastPacket(uint32_t type, const QByteArray& payload) {
    for (QTcpSocket *client : clients) {
        sendPacket(type, payload, client);
    }
}

void TCPServer::send(const QString &msg, QTcpSocket *client) {
    sendPacket(kTextMessageType, msg.toUtf8(), client);
}

void TCPServer::sendPacket(uint32_t type, const QByteArray& payload, QTcpSocket *client) {
    if (client && client->isOpen()) {
        TLVPacket packet(type, static_cast<uint32_t>(payload.size()), payload.constData());
        client->write(packet.serialize());
    }
}

} // namespace sjtu