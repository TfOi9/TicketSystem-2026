#include <QCoreApplication>
#include "../../include/web/tcpserver.hpp"
#include <QTimer>
#include <string>
#include <iostream>

int main(int argc, char **argv) {
    QCoreApplication a(argc, argv);
    
    sjtu::TCPServer server;
    server.start(1145);

    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [&]() {
        server.broadcast("Heartbeat from server");
    });
    timer.start(5000);

    QTimer inputTimer;
    QTextStream cinStream(stdin);
    
    QObject::connect(&inputTimer, &QTimer::timeout, [&]() {
        if (cinStream.atEnd()) {
            return;
        }
        
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
    
    inputTimer.start(500);
    
    return a.exec();
}