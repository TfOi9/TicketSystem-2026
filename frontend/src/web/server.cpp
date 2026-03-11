#include <QCoreApplication>
#include "../../include/web/tcp/tcpserver.hpp"
#include <QTimer>
#include <string>
#include <iostream>
#include <QSocketNotifier>
#include <QTextStream>

int main(int argc, char **argv) {
    QCoreApplication a(argc, argv);
    
    sjtu::TCPServer server;
    server.start(1145);

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