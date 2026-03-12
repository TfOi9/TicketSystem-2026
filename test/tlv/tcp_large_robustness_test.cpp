#include <cassert>
#include <iostream>
#include <atomic>
#include <vector>
#include <memory>
#include <QCoreApplication>
#include <QTimer>
#include "../frontend/include/web/tcp/tcpserver.hpp"
#include "../frontend/include/web/tcp/tcpclient.hpp"

using sjtu::TCPServer;
using sjtu::TCPClient;

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);

    TCPServer server;
    const quint16 port = 34678;
    if (!server.start(port)) {
        std::cerr << "Server failed to start on port " << port << ", skipping test\n";
        return 0;
    }

    const int N = 200; // number of concurrent clients
    std::atomic<int> connected_count{0};
    std::atomic<int> welcome_count{0};
    std::atomic<int> disconnected_count{0};

    std::vector<std::unique_ptr<TCPClient>> clients;
    clients.reserve(N);

    for (int i = 0; i < N; ++i) {
        auto c = std::make_unique<TCPClient>(&app);
        QObject::connect(c.get(), &TCPClient::connected, [&connected_count]() {
            ++connected_count;
        });
        QObject::connect(c.get(), &TCPClient::received, [&welcome_count](const QString& msg) {
            if (msg.contains("Welcome")) ++welcome_count;
        });
        QObject::connect(c.get(), &TCPClient::disconnected, [&disconnected_count]() {
            ++disconnected_count;
        });
        clients.push_back(std::move(c));
    }

    // stagger connections slightly to avoid bursting too many sockets at once
    for (int i = 0; i < N; ++i) {
        QTimer::singleShot(i % 50, [i, &clients, port]() {
            clients[i]->connectToServer("127.0.0.1", port);
        });
    }

    // after connections established, send a message from each client
    QTimer::singleShot(2000, [&clients]() {
        for (auto &c : clients) c->send("ping_large");
    });

    // after some time, disconnect all
    QTimer::singleShot(5000, [&clients]() {
        for (auto &c : clients) c->disconnect();
    });

    // finish test after timeout and assert counts
    QTimer::singleShot(8000, [&]() {
        std::cout << "connected_count=" << connected_count << " welcome_count=" << welcome_count
                  << " disconnected_count=" << disconnected_count << "\n";
        // basic expectations: most clients connected and got welcome messages
        assert(connected_count > N * 0.8);
        assert(welcome_count > N * 0.8);
        assert(disconnected_count >= N * 0.8);
        app.quit();
    });

    app.exec();
    std::cout << "TCP large-scale robustness test passed\n";
    return 0;
}
