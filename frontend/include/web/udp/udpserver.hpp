#ifndef UDPSERVER_HPP
#define UDPSERVER_HPP

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QNetworkInterface>

namespace sjtu {

class UDPServer : public QObject {
    Q_OBJECT

public:
    explicit UDPServer(QObject *parent = nullptr);

    bool broadcast(const QString& msg, quint16 port);

    bool startListening(quint16 port);

    void stopListening();

    QString getLocalIPv4Address();

    QString getBroadcastAddress();

private:
    QUdpSocket *socket;
    bool isListening;

private slots:
    void readPendingDatagrams();

};

} // namespace sjtu

#endif // UDPSERVER_HPP