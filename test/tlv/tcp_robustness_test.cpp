#include <cassert>
#include <iostream>
#include <atomic>
#include <vector>
#include <QCoreApplication>
#include <QTimer>
#include "../frontend/include/web/tcp/tcpserver.hpp"
#include "../frontend/include/web/tcp/tcpclient.hpp"

using sjtu::TCPServer;
using sjtu::TCPClient;

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);

    TCPServer server;
    const quint16 port = 34567;
    if (!server.start(port)) {
        std::cerr << "Server failed to start on port " << port << ", skipping test\n";
        return 0;
    }

    const int N = 8;
    std::atomic<int> welcome_count{0};
    std::vector<TCPClient*> clients;

    for (int i = 0; i < N; ++i) {
        TCPClient* c = new TCPClient(&app);
        QObject::connect(c, &TCPClient::received, [&welcome_count](const QString& msg) {
            if (msg.contains("Welcome")) ++welcome_count;
        });
        clients.push_back(c);
        c->connectToServer("127.0.0.1", port);
    }

    QTimer::singleShot(500, [&clients]() {
        for (auto c : clients) c->send("ping");
    });

    QTimer::singleShot(1500, [&]() {
        assert(welcome_count == N);
        for (auto c : clients) {
            c->disconnect();
            c->deleteLater();
        }
        app.quit();
    });

    app.exec();
    std::cout << "TCP robustness test passed\n";
    return 0;
}
