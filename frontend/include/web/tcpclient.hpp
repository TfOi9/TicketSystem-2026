#ifndef TCPCLIENT_HPP
#define TCPCLIENT_HPP

#include <QObject>
#include <QTcpSocket>
#include <QtCore/qtmetamacros.h>
#include <QtNetwork/qabstractsocket.h>

namespace sjtu {

class TCPClient : public QObject {
    Q_OBJECT

public:
    explicit TCPClient(QObject *parent = nullptr);

    void connectToServer(const QString& host, quint16 port);
    void disconnect();
    void send(const QString& msg);
    bool isConnected() const;

signals:
    void connected();
    void disconnected();
    void received(const QString& msg);
    void error(const QString& err);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError err);

private:
    QTcpSocket *socket;
    
};

} // namespace sjtu

#endif // TCPCLIENT_HPP