#include "../../include/web/tcpclient.hpp"
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
    if (socket->isOpen()) {
        socket->write(msg.toUtf8());
    }
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
    QByteArray data = socket->readAll();
    emit received(QString::fromUtf8(data));
}

void TCPClient::onError(QAbstractSocket::SocketError socketError) {
    QString msg = socket->errorString();
    emit error(msg);
    qDebug() << "Socket error:" << msg;
}

} // namespace sjtu