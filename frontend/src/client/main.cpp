#include "client/main_window.hpp"

#include <QApplication>

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    sjtu::client::MainWindow window;
    window.show();
    return app.exec();
}