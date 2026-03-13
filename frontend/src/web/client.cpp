#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <QtCore/qstring.h>
#include <string>
#include <iostream>
#include <QSocketNotifier>
#include <QTextStream>
#include <QDateTime>

#include "../../include/web/tcp/tcpclient.hpp"
#include "../../include/web/udp/udpclient.hpp"

#include "../../../include/command/command.hpp"
#include "../../../include/command/token.hpp"

QByteArray serializeCommandInfo(const sjtu::Command& cmd) {
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_2);
    out << cmd.timestamp() << QString::fromStdString(cmd.cmd());
    for (char ch = 'a'; ch <= 'z'; ch++) {
        out << QString::fromStdString(cmd.arg(ch));
    }
    return payload;
}

bool parseCommandInfo(const QByteArray& payload, sjtu::Command& out) {
    QDataStream in(payload);
    in.setVersion(QDataStream::Qt_6_2);
    QString arg, cmd;
    int timestamp;
    in >> timestamp >> cmd;
    for (char ch = 'a'; ch <= 'z'; ch++) {
        in >> arg;
        out.set_arg(ch, arg.toStdString());
    }
    out.set_timestamp(timestamp);
    out.set_cmd(cmd.toStdString());
    return in.status() == QDataStream::Ok;
}

int main(int argc, char **argv) {
    QCoreApplication a(argc, argv);
    
    sjtu::TCPClient client;
    sjtu::UDPClient udpClient;
    const quint16 discoveryPort = 45454;
    bool connectedViaDiscovery = false;

    client.registerPacketCodec<sjtu::Command>(
        1001,
        serializeCommandInfo,
        parseCommandInfo,
        [&](const sjtu::Command& cmd) {
            std::cout << "Package sent at timestamp " << cmd.timestamp() << std::endl;
        }
    );

    // Start UDP discovery listener
    if (udpClient.startListening(discoveryPort)) {
        QObject::connect(&udpClient, &sjtu::UDPClient::stringReceived,
                         [&](const QString &str, const QString &senderIp, quint16 /*senderPort*/) {
            if (connectedViaDiscovery) return;
            if (str.startsWith("TicketSystem")) {
                qDebug() << "Discovered server at" << senderIp << ", connecting TCP...";
                std::cout << "Discovered server at " << senderIp.toStdString() << std::endl;
                client.connectToServer(senderIp, 1145);
                connectedViaDiscovery = true;
                udpClient.stopListening();
            }
        });
    }

    // Active probe: send a few DISCOVER broadcasts to prompt a reply from server
    QUdpSocket *probeSocket = new QUdpSocket(&a);
    QTimer probeTimer;
    int *attempts = new int(0);
    QObject::connect(&probeTimer, &QTimer::timeout, [&]() {
        if (*attempts >= 3) {
            probeTimer.stop();
            delete attempts;
            probeSocket->close();
            probeSocket->deleteLater();
            return;
        }
        QByteArray probe = "DISCOVER";
        probeSocket->writeDatagram(probe, QHostAddress::Broadcast, discoveryPort);
        qDebug() << "Sent DISCOVER probe" << *attempts+1;
        (*attempts)++;
    });
    probeTimer.start(300); // send three probes every 300ms
    
    QObject::connect(&client, &sjtu::TCPClient::connected, [&]() {
        qDebug() << "Connected, sending message...";
        client.send("Hello server!");
    });
    
    QObject::connect(&client, &sjtu::TCPClient::received, 
                     [](const QString &msg) {
        qDebug() << "Received:" << msg;
        std::cout << "Received: " << msg.toStdString() << std::endl;
    });
    
    // If you want to skip discovery and connect directly, uncomment below:
    // client.connectToServer("127.0.0.1", 1145);

    QTextStream cinStream(stdin);
    QSocketNotifier stdinNotifier(fileno(stdin), QSocketNotifier::Read, &a);
    QObject::connect(&stdinNotifier, &QSocketNotifier::activated, [&](int) {
        if (cinStream.atEnd()) return;
        QString line = cinStream.readLine();
        if (line.startsWith("send ")) {
            QString msg = line.mid(5);
            client.send(msg);
            std::cout << "Send: " << msg.toStdString() << std::endl;
        }
        else if (line == "exit") {
            a.quit();
        }
        else {
            qint64 timestamp = QDateTime::currentSecsSinceEpoch();
            QString cmdstr = "[" + QString::number(timestamp) + "] " + line;
            sjtu::Command *cmd;
            try {
                sjtu::TokenStream stream(cmdstr.toStdString());
                cmd = new sjtu::Command(stream);
            }
            catch(const std::exception& e) {
                std::cout << e.what() << std::endl;
                return;
            }
            std::cout << "Now sending to server...\n";
            client.sendObject(1001, *cmd);
            delete cmd;
        }
    });

    return a.exec();
}