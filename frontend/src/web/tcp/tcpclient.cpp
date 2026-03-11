#include "../../../include/web/tcp/tcpclient.hpp"
#include "../../../include/web/tlv/tlvpacket.hpp"
#include <QtCore/qlogging.h>
#include <QtCore/qobject.h>
#include <QtCore/qoverload.h>
#include <QtNetwork/qabstractsocket.h>

namespace sjtu {

TCPClient::TCPClient(QObject *parent) : QObject(parent) {
    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::connected, this, &TCPClient::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &TCPClient::onDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &TCPClient::onReadyRead);
    connect(socket, &QTcpSocket::errorOccurred, this, &TCPClient::onError);
}

void TCPClient::connectToServer(const QString &host, quint16 port) {
    socket->connectToHost(host, port);
}

void TCPClient::disconnect() {
    socket->disconnectFromHost();
}

void TCPClient::send(const QString &msg) {
    sendPacket(kTextMessageType, msg.toUtf8());
}

void TCPClient::sendPacket(uint32_t type, const QByteArray& payload) {
    if (!socket->isOpen()) {
        return;
    }
    TLVPacket packet(type, static_cast<uint32_t>(payload.size()), payload.constData());
    socket->write(packet.serialize());
}

bool TCPClient::isConnected() const {
    return socket->isOpen();
}

void TCPClient::onConnected() {
    emit connected();
    qDebug() << "Connected to server.";
}

void TCPClient::onDisconnected() {
    emit disconnected();
    qDebug() << "Disconnected from server.";
}

void TCPClient::onReadyRead() {
    parser_.appendData(socket->readAll());

    while (parser_.hasCompletePacket()) {
        TLVPacket packet = parser_.nextPacket();
        QByteArray payload(packet.data(), static_cast<qsizetype>(packet.size()));

        emit packetReceived(packet.type(), payload);
        dispatcher_.dispatch(packet.type(), payload);
        if (packet.type() == kTextMessageType) {
            emit received(QString::fromUtf8(payload));
        }
    }
}

void TCPClient::onError(QAbstractSocket::SocketError socketError) {
    QString msg = socket->errorString();
    emit error(msg);
    qDebug() << "Socket error:" << msg;
}

} // namespace sjtu