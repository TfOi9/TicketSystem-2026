#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include "bars/top_bar.hpp"
#include "bars/status_bar.hpp"
#include "dialogs/login_dialog.hpp"
#include "dialogs/register_dialog.hpp"
#include "dialogs/profile_dialog.hpp"
#include "dialogs/buy_ticket_dialog.hpp"
#include "dialogs/refund_dialog.hpp"
#include "widgets/home_page_widget.hpp"
#include "widgets/ticket_list_widget.hpp"
#include "widgets/orders_page_widget.hpp"
#include "widgets/admin_page_widget.hpp"

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QMainWindow>
#include <QStackedWidget>
#include <QTimer>

class QCloseEvent;

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
    }

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void initializeComponents();
    void startServerDiscovery();
    void onDiscoveryProbeTimeout();
    void onServerDiscovered(const QString &message, const QString &senderIp, quint16 senderPort);
    void onLoginRequested();
    void onRegisterRequested();
    void onLogoutRequested();
    void onProfileRequested();
    void onQueryTicketRequested(const QString &fromStation, const QString &toStation, const QString &date);
    void onBuyTicketRequested(const QString &trainName);
    void onRefundRequested(int orderIndex);
    void onTrainScheduleRequested(const QString &trainName);
    void onOrderPageActivated();
    void onOrderRefreshRequested();
    void onAdminQueryTrainRequested(const QString &trainId, const QString &date);
    void onAdminReleaseTrainRequested(const QString &trainId);
    void onAdminDeleteTrainRequested(const QString &trainId);
    void onAdminAddTrainRequested(const QString &command);
    void onAdminQueryProfileRequested(const QString &username);
    void onAdminAddUserRequested(const QString &username, const QString &password,
                                 const QString &name, const QString &email, int privilege);
    void onAdminImportTrainsRequested(const QString &filePath);

private:
    enum class PendingAction {
        None,
        Login,
        PostLoginQueryProfile,
        Register,
        Logout,
        QueryProfile,
        QueryTicket,
        BuyTicket,
        RefundTicket,
        QueryOrder,
        QueryTrain,
        QueryTrainSchedule,
        ReleaseTrain,
        DeleteTrain,
        AddTrain,
        ImportTrain,
        AdminQueryProfile,
        AdminAddUser
    };

    void initalizeUI();
    void handleAuthChanged(const QString &msg);
    void setupNetworkClients();
    void updateConnectionStatus(const QString &status);
    bool sendCommandLine(const QString &commandLine, PendingAction action);
    void processServerResult(sjtu::ResultType type, const sjtu::Result &result);
    void applyAuthState();
    void resetAuthState();
    void tryGracefulLogoutBeforeExit();
    static QString escapeArg(const QString &arg);
    void refreshOrders();
    void sendNextImportTrain();

    TopBar *topBar;
    StatusBar *statusBarWidget;

    QStackedWidget *stackedPanel;
    HomePageWidget *homePageWidget;
    TicketListWidget *ticketPageWidget;
    OrdersPageWidget *orderPageWidget;
    AdminPageWidget *managePageWidget;

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

    TicketListWidget::TicketListItem currentTicketContext;
    QString currentTicketDate;
    OrdersPageWidget::OrderItem currentRefundContext;

    QStringList importTrainsList;
    int importTrainsIndex;
    int importTrainsSuccess;
    int importTrainsFail;

    static constexpr quint16 kDiscoveryPort = 45454;
    static constexpr quint16 kServerPort = 1145;
    static constexpr int kMaxDiscoveryAttempts = 3;

    bool initialized;
    bool isShuttingDown;

};

} // namespace client
} // namespace sjtu

#endif // MAIN_WINDOW_HPP
