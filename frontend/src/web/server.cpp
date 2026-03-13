#include <QCoreApplication>
#include <QTimer>
#include <string>
#include <iostream>
#include <QSocketNotifier>
#include <QTextStream>
#include <QDateTime>

#include "../../include/web/tcp/tcpserver.hpp"
#include "../../include/web/udp/udpserver.hpp"

#include "../../../include/system/ticket.hpp"
#include "../../../include/command/command.hpp"

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
    std::cerr << timestamp << " " << cmd.toStdString() << std::endl;
    for (char ch = 'a'; ch <= 'z'; ch++) {
        in >> arg;
        std::cerr << int(in.status() == QDataStream::Ok) << " " << arg.toStdString() << std::endl;
        out.set_arg(ch, arg.toStdString());
    }
    out.set_timestamp(timestamp);
    out.set_cmd(cmd.toStdString());
    std::cerr << "the cmd is:";
    std::cerr << out.cmd() << std::endl;
    return in.status() == QDataStream::Ok;
}

int main(int argc, char **argv) {
    QCoreApplication a(argc, argv);

    sjtu::TicketSystem system("server");
    
    sjtu::TCPServer server;
    server.start(1145);

    server.registerPacketCodec<sjtu::Command>(
        1001,
        serializeCommandInfo,
        parseCommandInfo,
        [&](QTcpSocket* sock, const sjtu::Command& cmd) {
            std::cout << "Received command from client: cmd = " << cmd.cmd() << std::endl;
            std::string res = system.handle(cmd);
            std::cout << "The result is: " << res << std::endl;
            QString msg = "[" + QString::number(cmd.timestamp()) + "] " + QString::fromStdString(res);
            server.send(msg, sock);
        }
    );

    sjtu::UDPServer udpServer;
    udpServer.startListening(45454);
    QTimer udpTimer;
    QObject::connect(&udpTimer, &QTimer::timeout, [&]() {
        QString tag = "TicketSystem";
        udpServer.broadcast(tag, 45454);
    });
    udpTimer.start(2000);

    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [&]() {
        QString hb = "Heartbeat from server";
        server.broadcast(hb);
        std::cout << "Broadcast: " << hb.toStdString() << std::endl;
    });
    timer.start(60000);
    
    QTextStream cinStream(stdin);
    QSocketNotifier stdinNotifier(fileno(stdin), QSocketNotifier::Read, &a);
    QObject::connect(&stdinNotifier, &QSocketNotifier::activated, [&](int) {
        if (cinStream.atEnd()) return;
        QString line = cinStream.readLine();
        if (line.startsWith("broadcast ")) {
            QString msg = line.mid(10);
            server.broadcast(msg);
            std::cout << "Broadcast: " << msg.toStdString() << std::endl;
        }
        else if (line == "exit") {
            a.quit();
        }
    });

    return a.exec();
}