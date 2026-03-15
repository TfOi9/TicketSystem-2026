#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include "top_bar.hpp"

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QMainWindow>
#include <QStackedWidget>

#include <functional>

namespace sjtu {
namespace client {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

    void setStatusMessage(const QString &message) {
        // statusBar()->showMessage(message);
    }

private slots:
    void initializeComponents();

private:
    void initalizeUI();
    void handleAuthChanged(const QString &msg);

    TopBar *topBar;

    QStackedWidget *stackedPanel;

    bool initialized;

};

} // namespace client
} // namespace sjtu

#endif // MAIN_WINDOW_HPP