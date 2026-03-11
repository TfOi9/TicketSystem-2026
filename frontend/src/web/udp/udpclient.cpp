#include "../../../include/web/udp/udpclient.hpp"
#include <QDebug>

namespace sjtu {

UDPClient::UDPClient(QObject *parent) : QObject(parent) {
    socket = new QUdpSocket(this);
    isListening = false;
    connect(socket, &QUdpSocket::readyRead, this, &UDPClient::readPendingDatagrams);
}

UDPClient::~UDPClient() {
    stopListening();
}

bool UDPClient::startListening(quint16 port) {
    if (isListening) {
        stopListening();
    }
    // Bind to IPv4 Any and allow address reuse/share so multiple listeners can work.
    isListening = socket->bind(QHostAddress::AnyIPv4, port,
                              QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    if (isListening) {
        qDebug() << "UDP client started on port:" << port;
    } else {
        qDebug() << "Failed to start UDP client on port:" << port << ", error:" << socket->errorString();
    }
    return isListening;
}

void UDPClient::stopListening() {
    if (isListening) {
        socket->close();
        isListening = false;
        qDebug() << "UDP client stopped";
    }
}

void UDPClient::readPendingDatagrams() {
    while (socket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(socket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        socket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        QString strData = QString::fromUtf8(datagram);
        emit stringReceived(strData, sender.toString(), senderPort);
        qDebug() << "Received from" << sender.toString() << ":" << senderPort << "-" << datagram.size() << "bytes";
    }
}

} // namespace sjtu