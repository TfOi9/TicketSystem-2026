#include "../../../include/web/udp/udpserver.hpp"
#include <QDebug>
#include <QByteArray>
#include <QtCore/qstring.h>

namespace sjtu {

UDPServer::UDPServer(QObject *parent) : QObject(parent) {
    socket = new QUdpSocket(this);
    isListening = false;
    connect(socket, &QUdpSocket::readyRead, this, &UDPServer::readPendingDatagrams);
}

bool UDPServer::broadcast(const QString &msg, quint16 port) {
    QHostAddress broadcastAddress = QHostAddress::Broadcast;
    QByteArray data = msg.toUtf8();
    qint64 sent = socket->writeDatagram(data, broadcastAddress, port);
    if (sent == -1) {
        qDebug() << "Broadcast failed:" << socket->errorString();
        return false;
    }
    qDebug() << "Broadcast sent to port " << port;
    return true;
}

QString UDPServer::getLocalIPv4Address() {
    for (const QHostAddress &address : QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && 
            address != QHostAddress::LocalHost) {
            return address.toString();
        }
    }
    return "";
}

QString UDPServer::getBroadcastAddress() {
    return "255.255.255.255";
}

bool UDPServer::startListening(quint16 port) {
    if (isListening) stopListening();
    isListening = socket->bind(QHostAddress::AnyIPv4, port,
                              QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    if (isListening) qDebug() << "UDP server listening on port:" << port;
    else qDebug() << "Failed to bind UDP server on port:" << port << "," << socket->errorString();
    return isListening;
}

void UDPServer::stopListening() {
    if (isListening) {
        socket->close();
        isListening = false;
        qDebug() << "UDP server stopped listening";
    }
}

void UDPServer::readPendingDatagrams() {
    while (socket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(socket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        socket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        QString strData = QString::fromUtf8(datagram);
        qDebug() << "UDP server received from" << sender.toString() << ":" << senderPort << "->" << strData;
        if (strData.startsWith("DISCOVER")) {
            QString resp = "TicketSystem:" + getLocalIPv4Address();
            socket->writeDatagram(resp.toUtf8(), sender, senderPort);
            qDebug() << "Replied to discovery from" << sender.toString();
        }
    }
}

} // namespace sjtu