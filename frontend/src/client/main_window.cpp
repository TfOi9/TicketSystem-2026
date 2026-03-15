#include "../../include/client/main_window.hpp"

#include <QLayout>

namespace sjtu::client {

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), initialized(false) {
    initalizeUI();
}

void MainWindow::initalizeUI() {
    topBar = new TopBar(this);
    setCentralWidget(new QWidget(this));
    stackedPanel = new QStackedWidget(centralWidget());
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget());
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(topBar);
    mainLayout->addWidget(stackedPanel);

    connect(topBar, &TopBar::authChanged, this, &MainWindow::handleAuthChanged);

    initializeComponents();
}

void MainWindow::handleAuthChanged(const QString &msg) {
    // setStatusMessage(msg);
}

void MainWindow::initializeComponents() {
    // todo: initialize different panels and add them to stackedPanel
    initialized = true;
}

} // namespace sjtu::client