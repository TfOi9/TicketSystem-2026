#include <QCoreApplication>
#include "../../include/web/tcpclient.hpp"
#include <QDebug>
#include <QTimer>
#include <string>
#include <iostream>
#include <QSocketNotifier>
#include <QTextStream>

int main(int argc, char **argv) {
    QCoreApplication a(argc, argv);
    
    sjtu::TCPClient client;
    
    QObject::connect(&client, &sjtu::TCPClient::connected, [&]() {
        qDebug() << "Connected, sending message...";
        client.send("Hello server!");
    });
    
    QObject::connect(&client, &sjtu::TCPClient::received, 
                     [](const QString &msg) {
        qDebug() << "Received:" << msg;
        std::cout << "Received: " << msg.toStdString() << std::endl;
    });
    
    client.connectToServer("127.0.0.1", 1145);

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
    });

    return a.exec();
}