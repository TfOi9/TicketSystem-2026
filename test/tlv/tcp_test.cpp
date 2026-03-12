#include <cassert>
#include <iostream>
#include <QCoreApplication>
#include "../frontend/include/web/tcp/tcpserver.hpp"
#include "../frontend/include/web/tcp/tcpclient.hpp"

using sjtu::TCPServer;
using sjtu::TCPClient;

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);

    TCPServer server;
    // don't rely on listen success on all platforms; just ensure methods are callable
    server.broadcast("hello");
    server.send("x", nullptr);

    TCPClient client;
    assert(!client.isConnected());
    client.send("test");
    client.disconnect();

    return 0;
}
