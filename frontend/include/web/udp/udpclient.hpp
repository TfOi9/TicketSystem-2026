#ifndef UDPCLIENT_HPP
#define UDPCLIENT_HPP

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QNetworkInterface>

namespace sjtu {

class UDPClient : public QObject {
    Q_OBJECT

public:
    explicit UDPClient(QObject *parent = nullptr);

    ~UDPClient();

    bool startListening(quint16 port);

    void stopListening();

signals:
    void stringReceived(const QString &str, const QString &senderIp, quint16 senderPort);
    
private slots:
    void readPendingDatagrams();
    
private:
    QUdpSocket *socket;
    bool isListening;

};

} // namespace sjtu

#endif // UDPCLIENT_HPP