#include "../../include/web/tcpserver.hpp"
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
        qDebug() << "New client connected: " << socket->peerAddress().toString();
        connect(socket, &QTcpSocket::readyRead, this, &TCPServer::onReadyRead);
        connect(socket, &QTcpSocket::disconnected, this, &TCPServer::onClientDisconnected);
        socket->write("Welcome! You are connected to server.");
    }
}

void TCPServer::onReadyRead() {
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    QByteArray data = socket->readAll();
    QString msg = QString::fromUtf8(data);
    qDebug() << "Data received from client:" << msg;
    std::cout << "Received from " << socket->peerAddress().toString().toStdString() << ": " << msg.toStdString() << std::endl;
}

void TCPServer::onClientDisconnected() {
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    clients.removeOne(socket);
    qDebug() << "Cliend disconnected: " << socket->peerAddress().toString();
    socket->deleteLater();
}

void TCPServer::broadcast(const QString &msg) {
    QByteArray data = msg.toUtf8();
    for (QTcpSocket *client : clients) {
        client->write(data);
    }
}

void TCPServer::send(const QString &msg, QTcpSocket *client) {
    if (client && client->isOpen()) {
        client->write(msg.toUtf8());
    }
}

} // namespace sjtu