#ifndef TCPSERVER_HPP
#define TCPSERVER_HPP

#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QString>

namespace sjtu {

class TCPServer : public QObject {
    Q_OBJECT

public:
    explicit TCPServer(QObject *parent = nullptr);
    ~TCPServer();

    bool start(quint16 port);
    void broadcast(const QString& msg);
    void send(const QString& msg, QTcpSocket *client);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onClientDisconnected();

private:
    QTcpServer *server;
    QList<QTcpSocket*> clients;

};

} // namespace sjtu

#endif // TCPSERVER_HPP