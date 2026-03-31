#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include "top_bar.hpp"
#include "status_bar.hpp"
#include "login_dialog.hpp"
#include "register_dialog.hpp"
#include "profile_dialog.hpp"

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QMainWindow>
#include <QStackedWidget>
#include <QTimer>

#include "../web/tcp/tcpclient.hpp"
#include "../web/udp/udpclient.hpp"

#include "result/result.hpp"

#include <functional>

class QUdpSocket;
class QMessageBox;

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
    void startServerDiscovery();
    void onDiscoveryProbeTimeout();
    void onServerDiscovered(const QString &message, const QString &senderIp, quint16 senderPort);
    void onLoginRequested();
    void onRegisterRequested();
    void onLogoutRequested();
    void onProfileRequested();

private:
    enum class PendingAction {
        None,
        Login,
        Register,
        Logout,
        QueryProfile
    };

    void initalizeUI();
    void handleAuthChanged(const QString &msg);
    void setupNetworkClients();
    void updateConnectionStatus(const QString &status);
    bool sendCommandLine(const QString &commandLine, PendingAction action);
    void processServerResult(sjtu::ResultType type, const sjtu::Result &result);
    void applyAuthState();
    void resetAuthState();
    static QString escapeArg(const QString &arg);

    TopBar *topBar;
    StatusBar *statusBarWidget;

    QStackedWidget *stackedPanel;

    sjtu::TCPClient *tcpClient;
    sjtu::UDPClient *udpClient;
    QUdpSocket *discoveryProbeSocket;
    QTimer *discoveryProbeTimer;

    bool connectedViaDiscovery;
    int discoveryAttempts;

    PendingAction pendingAction;
    QString pendingLoginUsername;
    bool showProfileDialogOnQuery;

    bool isLoggedIn;
    QString currentUsername;
    QString currentName;
    QString currentEmail;
    int currentPrivilege;

    LoginDialog *loginDialog;
    RegisterDialog *registerDialog;
    ProfileDialog *profileDialog;

    static constexpr quint16 kDiscoveryPort = 45454;
    static constexpr quint16 kServerPort = 1145;
    static constexpr int kMaxDiscoveryAttempts = 3;

    bool initialized;

};

} // namespace client
} // namespace sjtu

#endif // MAIN_WINDOW_HPP