#include "client/main_window.hpp"

#include <QLayout>
#include <QEventLoop>
#include <QDataStream>
#include <QDateTime>
#include <QDebug>
#include <QHostAddress>
#include <QMessageBox>
#include <QCloseEvent>
#include <QUdpSocket>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>

#include <iostream>

#include "../../../include/command/command.hpp"
#include "../../../include/command/token.hpp"
#include "../../../include/result/result.hpp"
#include "client/widgets/ticket_query_widget.hpp"

namespace {

QString formatDate(const sjtu::date &d) {
    return QString("%1-%2")
        .arg(d.month_, 2, 10, QLatin1Char('0'))
        .arg(d.day_, 2, 10, QLatin1Char('0'));
}

QString formatTime(const sjtu::time &t) {
    QString text = QString("%1:%2")
        .arg(t.hr_, 2, 10, QLatin1Char('0'))
        .arg(t.min_, 2, 10, QLatin1Char('0'));
    if (t.day_offset_ > 0) {
        text += QString(" (+%1d)").arg(t.day_offset_);
    }
    return text;
}

QString formatDateTime(const sjtu::date &d, const sjtu::time &t) {
    return formatDate(d) + " " + formatTime(t);
}

qint64 toMinutesKey(const sjtu::date &d, const sjtu::time &t) {
    return static_cast<qint64>(int(d)) * 1440
         + static_cast<qint64>(t.day_offset_) * 1440
         + static_cast<qint64>(t.hr_) * 60
         + static_cast<qint64>(t.min_);
}

QByteArray serializeCommandInfo(const sjtu::Command &cmd) {
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_2);
    out << cmd.timestamp() << QString::fromStdString(cmd.cmd());
    for (char ch = 'a'; ch <= 'z'; ++ch) {
        out << QString::fromStdString(cmd.arg(ch));
    }
    return payload;
}

bool parseCommandInfo(const QByteArray &payload, sjtu::Command &out) {
    QDataStream in(payload);
    in.setVersion(QDataStream::Qt_6_2);

    QString arg;
    QString cmd;
    int timestamp = 0;
    in >> timestamp >> cmd;
    for (char ch = 'a'; ch <= 'z'; ++ch) {
        in >> arg;
        out.set_arg(ch, arg.toStdString());
    }

    if (in.status() != QDataStream::Ok) {
        return false;
    }

    out.set_timestamp(timestamp);
    out.set_cmd(cmd.toStdString());
    return true;
}

} // namespace

namespace sjtu::client {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
        topBar(nullptr),
        statusBarWidget(nullptr),
        stackedPanel(nullptr),
        homePageWidget(nullptr),
        ticketPageWidget(nullptr),
        orderPageWidget(nullptr),
        managePageWidget(nullptr),
        tcpClient(nullptr),
        udpClient(nullptr),
        discoveryProbeSocket(nullptr),
        discoveryProbeTimer(nullptr),
        connectedViaDiscovery(false),
        discoveryAttempts(0),
        pendingAction(PendingAction::None),
        showProfileDialogOnQuery(false),
        isLoggedIn(false),
        currentPrivilege(0),
        loginDialog(nullptr),
        registerDialog(nullptr),
        profileDialog(nullptr),
        initialized(false),
        isShuttingDown(false),
        importTrainsIndex(0),
        importTrainsSuccess(0),
        importTrainsFail(0),
        batchReleaseIndex(0),
        batchReleaseSuccess(0),
        batchReleaseFail(0) {
    initalizeUI();
    setupNetworkClients();
    startServerDiscovery();
}

void MainWindow::closeEvent(QCloseEvent *event) {
        isShuttingDown = true;
        tryGracefulLogoutBeforeExit();
        QMainWindow::closeEvent(event);
}

void MainWindow::initalizeUI() {
    setFixedSize(1240, 800);

    topBar = new TopBar(this);
    statusBarWidget = new StatusBar(this);
    setCentralWidget(new QWidget(this));
    stackedPanel = new QStackedWidget(centralWidget());
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget());
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(topBar);
    mainLayout->addWidget(stackedPanel, 1);
    mainLayout->addWidget(statusBarWidget);

    connect(topBar, &TopBar::authChanged, this, &MainWindow::handleAuthChanged);
    connect(topBar, &TopBar::loginRequested, this, &MainWindow::onLoginRequested);
    connect(topBar, &TopBar::registerRequested, this, &MainWindow::onRegisterRequested);
    connect(topBar, &TopBar::logoutRequested, this, &MainWindow::onLogoutRequested);
    connect(topBar, &TopBar::profileRequested, this, &MainWindow::onProfileRequested);

    loginDialog = new LoginDialog(this);
    registerDialog = new RegisterDialog(this);
    profileDialog = new ProfileDialog(this);
    applyAuthState();

    initializeComponents();
}

void MainWindow::setupNetworkClients() {
    tcpClient = new sjtu::TCPClient(this);
    udpClient = new sjtu::UDPClient(this);
    discoveryProbeSocket = new QUdpSocket(this);
    discoveryProbeTimer = new QTimer(this);

    tcpClient->registerPacketCodec<sjtu::Command>(
        1001,
        serializeCommandInfo,
        parseCommandInfo,
        [](const sjtu::Command &cmd) {
            qDebug() << "Command packet sent at" << cmd.timestamp();
        }
    );

    tcpClient->registerPacketReceiver<QByteArray>(
        sjtu::TCPClient::kResultMessageType,
        [](const QByteArray &payload, QByteArray &out) {
            out = payload;
            return true;
        },
        [this](const QByteArray &payload) {
            if (payload.size() < static_cast<int>(sizeof(quint32))) {
                qWarning() << "Invalid result payload: too short";
                return;
            }

            QDataStream in(payload);
            in.setVersion(QDataStream::Qt_6_2);
            quint32 rawType = 0;
            in >> rawType;

            if (in.status() != QDataStream::Ok) {
                qWarning() << "Invalid result payload stream";
                return;
            }

            const int headerSize = static_cast<int>(sizeof(quint32));
            const QByteArray body = payload.mid(headerSize);
            auto result = sjtu::Result::deserialize(
                static_cast<sjtu::ResultType>(rawType),
                body.constData(),
                static_cast<uint32_t>(body.size())
            );
            if (!result) {
                qWarning() << "Unknown result type:" << rawType;
                return;
            }
            processServerResult(static_cast<sjtu::ResultType>(rawType), *result);
            result->print(std::cout);
        }
    );

    connect(tcpClient, &sjtu::TCPClient::connected, this, [&]() {
        connectedViaDiscovery = true;
        if (discoveryProbeTimer->isActive()) {
            discoveryProbeTimer->stop();
        }
        if (udpClient) {
            udpClient->stopListening();
        }
        updateConnectionStatus(QString::fromUtf8("已连接"));
    });

    connect(tcpClient, &sjtu::TCPClient::disconnected, this, [&]() {
        updateConnectionStatus(QString::fromUtf8("已断开"));
        connectedViaDiscovery = false;
        pendingAction = PendingAction::None;
        pendingLoginUsername.clear();
        showProfileDialogOnQuery = false;
        resetAuthState();
    });

    connect(tcpClient, &sjtu::TCPClient::error, this, [&](const QString &err) {
        updateConnectionStatus(QString::fromUtf8("连接失败: ") + err);
        pendingAction = PendingAction::None;
        pendingLoginUsername.clear();
        showProfileDialogOnQuery = false;
        resetAuthState();
    });

    connect(udpClient, &sjtu::UDPClient::stringReceived, this, &MainWindow::onServerDiscovered);
    connect(discoveryProbeTimer, &QTimer::timeout, this, &MainWindow::onDiscoveryProbeTimeout);

    updateConnectionStatus(QString::fromUtf8("正在发现服务器..."));
}

void MainWindow::startServerDiscovery() {
    if (!udpClient->startListening(kDiscoveryPort)) {
        updateConnectionStatus(QString::fromUtf8("UDP 监听失败"));
        return;
    }

    discoveryAttempts = 0;
    connectedViaDiscovery = false;
    discoveryProbeTimer->start(300);
    onDiscoveryProbeTimeout();
}

void MainWindow::onDiscoveryProbeTimeout() {
    if (connectedViaDiscovery) {
        discoveryProbeTimer->stop();
        return;
    }

    if (discoveryAttempts >= kMaxDiscoveryAttempts) {
        discoveryProbeTimer->stop();
        updateConnectionStatus(QString::fromUtf8("未发现服务器"));
        return;
    }

    const QByteArray probe = "DISCOVER";
    discoveryProbeSocket->writeDatagram(probe, QHostAddress::Broadcast, kDiscoveryPort);
    ++discoveryAttempts;
}

void MainWindow::onServerDiscovered(const QString &message, const QString &senderIp, quint16) {
    if (connectedViaDiscovery) {
        return;
    }
    if (!message.startsWith("TicketSystem")) {
        return;
    }

    updateConnectionStatus(QString::fromUtf8("发现服务器，正在连接..."));
    tcpClient->connectToServer(senderIp, kServerPort);
}

void MainWindow::updateConnectionStatus(const QString &status) {
    if (statusBarWidget) {
        statusBarWidget->setConnectionStatus(status);
    }
}

void MainWindow::onLoginRequested() {
    if (pendingAction != PendingAction::None) {
        QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("当前有请求正在处理中，请稍后再试。"));
        return;
    }

    if (isLoggedIn) {
        QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("当前已有用户在线，请先退出当前账号再登录。"));
        return;
    }

    if (loginDialog->exec() != QDialog::Accepted) {
        return;
    }

    const QString username = loginDialog->username();
    const QString password = loginDialog->password();

    QString errorMsg;
    if (!LoginDialog::validateUsername(username, errorMsg) || !LoginDialog::validatePassword(password, errorMsg)) {
        QMessageBox::warning(this, QString::fromUtf8("输入错误"), errorMsg);
        return;
    }

    pendingLoginUsername = username;
    const QString command = "login -u " + escapeArg(username) + " -p " + escapeArg(password);
    if (!sendCommandLine(command, PendingAction::Login)) {
        pendingLoginUsername.clear();
        QMessageBox::warning(this, QString::fromUtf8("发送失败"), QString::fromUtf8("无法发送登录请求，请检查网络连接。"));
    }
}

void MainWindow::onRegisterRequested() {
    if (pendingAction != PendingAction::None) {
        QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("当前有请求正在处理中，请稍后再试。"));
        return;
    }

    if (registerDialog->exec() != QDialog::Accepted) {
        return;
    }

    const QString username = registerDialog->username();
    const QString password = registerDialog->password();
    const QString name = registerDialog->displayName();
    const QString email = registerDialog->email();

    QString errorMsg;
    if (!RegisterDialog::validateUsername(username, errorMsg)
        || !RegisterDialog::validatePassword(password, errorMsg)
        || !RegisterDialog::validateName(name, errorMsg)
        || !RegisterDialog::validateEmail(email, errorMsg)) {
        QMessageBox::warning(this, QString::fromUtf8("输入错误"), errorMsg);
        return;
    }

    QString command = "add_user -c root -u " + escapeArg(username)
            + " -p " + escapeArg(password)
            + " -n " + escapeArg(name)
            + " -m " + escapeArg(email)
            + " -g 1";

    if (!sendCommandLine(command, PendingAction::Register)) {
        QMessageBox::warning(this, QString::fromUtf8("发送失败"), QString::fromUtf8("无法发送注册请求，请检查网络连接。"));
    }
}

void MainWindow::onQueryTicketRequested(const QString &fromStation, const QString &toStation, const QString &date) {
    if (pendingAction != PendingAction::None) {
        QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("当前有请求正在处理中，请稍后再试。"));
        return;
    }

    if (fromStation.isEmpty() || toStation.isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8("查询失败"), QString::fromUtf8("出发站和到达站不能为空。"));
        return;
    }
    if (fromStation == toStation) {
        QMessageBox::warning(this, QString::fromUtf8("查询失败"), QString::fromUtf8("出发站和到达站不能相同。"));
        return;
    }

    const bool isTransfer = homePageWidget && homePageWidget->queryWidget
                            && homePageWidget->queryWidget->isTransferMode();
    const QString cmdName = isTransfer ? "query_transfer" : "query_ticket";
    const QString command = cmdName + " -s " + escapeArg(fromStation)
                          + " -t " + escapeArg(toStation)
                          + " -d " + date
                          + " -p time";

    currentTicketDate = date;

    if (!sendCommandLine(command, PendingAction::QueryTicket)) {
        QMessageBox::warning(this, QString::fromUtf8("发送失败"), QString::fromUtf8("无法发送查询请求，请检查网络连接。"));
        return;
    }

    if (ticketPageWidget != nullptr) {
        stackedPanel->setCurrentWidget(ticketPageWidget);
    }
}

void MainWindow::onBuyTicketRequested(const QString &trainName) {
    if (pendingAction != PendingAction::None) {
        QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("当前有请求正在处理中，请稍后再试。"));
        return;
    }

    if (!isLoggedIn) {
        QMessageBox::warning(this, QString::fromUtf8("提示"), QString::fromUtf8("请先登录后再购票。"));
        return;
    }

    if (currentTicketDate.isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8("提示"), QString::fromUtf8("请先查询车票后再购买。"));
        return;
    }

    const TicketListWidget::TicketListItem *ticketItem = nullptr;
    for (const auto &item : currentTicketList) {
        if (item.trainName == trainName) {
            ticketItem = &item;
            break;
        }
    }
    if (ticketItem == nullptr) {
        return;
    }

    BuyTicketDialog::TicketInfo info;
    info.trainName = ticketItem->trainName;
    info.fromStation = ticketItem->startStation;
    info.toStation = ticketItem->endStation;
    info.departureTime = ticketItem->departureTime;
    info.arrivalTime = ticketItem->arrivalTime;
    info.price = ticketItem->price;
    info.remain = ticketItem->remain;
    info.date = currentTicketDate;

    BuyTicketDialog dialog(info, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const int count = dialog.ticketCount();
    const bool useQueue = dialog.useQueue();

    if (ticketItem->isTransfer) {
        pendingTransferCount = count;
        pendingTransferUseQueue = useQueue;
        const QString firstCmd = "buy_ticket -u " + escapeArg(currentUsername)
                    + " -i " + escapeArg(ticketItem->firstTrainId)
                    + " -d " + ticketItem->firstDate
                    + " -n " + QString::number(count)
                    + " -f " + escapeArg(ticketItem->firstFromStation)
                    + " -t " + escapeArg(ticketItem->firstToStation)
                    + " -q " + (useQueue ? "true" : "false");
        if (!sendCommandLine(firstCmd, PendingAction::BuyTransferFirst)) {
            QMessageBox::warning(this, QString::fromUtf8("发送失败"), QString::fromUtf8("无法发送购票请求，请检查网络连接。"));
        }
        return;
    }

    QString command = "buy_ticket -u " + escapeArg(currentUsername)
                    + " -i " + escapeArg(info.trainName)
                    + " -d " + info.date
                    + " -n " + QString::number(count)
                    + " -f " + escapeArg(info.fromStation)
                    + " -t " + escapeArg(info.toStation)
                    + " -q " + (useQueue ? "true" : "false");

    if (!sendCommandLine(command, PendingAction::BuyTicket)) {
        QMessageBox::warning(this, QString::fromUtf8("发送失败"), QString::fromUtf8("无法发送购票请求，请检查网络连接。"));
    }
}

void MainWindow::onRefundRequested(int orderIndex) {
    if (pendingAction != PendingAction::None) {
        QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("当前有请求正在处理中，请稍后再试。"));
        return;
    }

    if (!isLoggedIn) {
        QMessageBox::warning(this, QString::fromUtf8("提示"), QString::fromUtf8("请先登录后退票。"));
        return;
    }

    if (orderPageWidget == nullptr || orderPageWidget->tableWidget == nullptr) {
        return;
    }

    int foundRow = -1;
    for (int row = 0; row < orderPageWidget->tableWidget->rowCount(); ++row) {
        QTableWidgetItem *item = orderPageWidget->tableWidget->item(row, 0);
        if (item && item->text().toInt() == orderIndex) {
            foundRow = row;
            break;
        }
    }
    if (foundRow < 0) {
        return;
    }

    RefundDialog::RefundInfo info;
    info.orderIndex = orderIndex;
    info.trainName = orderPageWidget->tableWidget->item(foundRow, 1)->text();
    info.fromStation = orderPageWidget->tableWidget->item(foundRow, 2)->text();
    info.toStation = orderPageWidget->tableWidget->item(foundRow, 3)->text();
    info.departureTime = orderPageWidget->tableWidget->item(foundRow, 4)->text();
    info.price = orderPageWidget->tableWidget->item(foundRow, 6)->text().toInt();
    info.count = orderPageWidget->tableWidget->item(foundRow, 7)->text().toInt();
    info.statusText = orderPageWidget->tableWidget->item(foundRow, 8)->text();

    RefundDialog dialog(info, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const QString command = "refund_ticket -u " + escapeArg(currentUsername)
                          + " -n " + QString::number(orderIndex);

    if (!sendCommandLine(command, PendingAction::RefundTicket)) {
        QMessageBox::warning(this, QString::fromUtf8("发送失败"), QString::fromUtf8("无法发送退票请求，请检查网络连接。"));
    }
}

void MainWindow::onOrderPageActivated() {
    refreshOrders();
}

void MainWindow::onOrderRefreshRequested() {
    refreshOrders();
}

void MainWindow::refreshOrders() {
    if (pendingAction != PendingAction::None) {
        return;
    }
    if (!isLoggedIn) {
        orderPageWidget->clearOrders();
        return;
    }
    const QString command = "query_order -u " + escapeArg(currentUsername);
    sendCommandLine(command, PendingAction::QueryOrder);
}

void MainWindow::onTrainScheduleRequested(const QString &trainName) {
    if (pendingAction != PendingAction::None) {
        QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("当前有请求正在处理中，请稍后再试。"));
        return;
    }
    if (currentTicketDate.isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8("提示"), QString::fromUtf8("请先查询车票后再查看时刻表。"));
        return;
    }
    const QString command = "query_train -i " + escapeArg(trainName)
                          + " -d " + currentTicketDate;
    if (!sendCommandLine(command, PendingAction::QueryTrainSchedule)) {
        QMessageBox::warning(this, QString::fromUtf8("发送失败"), QString::fromUtf8("无法发送时刻表查询请求。"));
    }
}

void MainWindow::onAdminQueryTrainRequested(const QString &trainId, const QString &date) {
    if (pendingAction != PendingAction::None) {
        QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("当前有请求正在处理中，请稍后再试。"));
        return;
    }
    const QString command = "query_train -i " + escapeArg(trainId)
                          + " -d " + date;
    if (!sendCommandLine(command, PendingAction::QueryTrain)) {
        QMessageBox::warning(this, QString::fromUtf8("发送失败"), QString::fromUtf8("无法发送查询请求。"));
    }
}

void MainWindow::onAdminReleaseTrainRequested(const QString &trainId) {
    if (pendingAction != PendingAction::None) {
        QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("当前有请求正在处理中，请稍后再试。"));
        return;
    }
    const QString command = "release_train -i " + escapeArg(trainId);
    if (!sendCommandLine(command, PendingAction::ReleaseTrain)) {
        QMessageBox::warning(this, QString::fromUtf8("发送失败"), QString::fromUtf8("无法发送发布请求。"));
    }
}

void MainWindow::onAdminDeleteTrainRequested(const QString &trainId) {
    if (pendingAction != PendingAction::None) {
        QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("当前有请求正在处理中，请稍后再试。"));
        return;
    }
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, QString::fromUtf8("确认删除"),
        QString::fromUtf8("确定要删除列车 ") + trainId + QString::fromUtf8(" 吗？"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (reply != QMessageBox::Yes) {
        return;
    }
    const QString command = "delete_train -i " + escapeArg(trainId);
    if (!sendCommandLine(command, PendingAction::DeleteTrain)) {
        QMessageBox::warning(this, QString::fromUtf8("发送失败"), QString::fromUtf8("无法发送删除请求。"));
    }
}

void MainWindow::onAdminAddTrainRequested(const QString &command) {
    if (pendingAction != PendingAction::None) {
        QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("当前有请求正在处理中，请稍后再试。"));
        return;
    }
    if (!sendCommandLine(command, PendingAction::AddTrain)) {
        QMessageBox::warning(this, QString::fromUtf8("发送失败"), QString::fromUtf8("无法发送添加请求。"));
    }
}

void MainWindow::onAdminQueryProfileRequested(const QString &username) {
    if (pendingAction != PendingAction::None) {
        QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("当前有请求正在处理中，请稍后再试。"));
        return;
    }
    const QString command = "query_profile -c " + escapeArg(currentUsername)
                          + " -u " + escapeArg(username);
    if (!sendCommandLine(command, PendingAction::AdminQueryProfile)) {
        QMessageBox::warning(this, QString::fromUtf8("发送失败"), QString::fromUtf8("无法发送查询请求。"));
    }
}

void MainWindow::onAdminAddUserRequested(const QString &username, const QString &password,
                                          const QString &name, const QString &email, int privilege) {
    if (pendingAction != PendingAction::None) {
        QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("当前有请求正在处理中，请稍后再试。"));
        return;
    }
    QString command = "add_user -c " + escapeArg(currentUsername)
                    + " -u " + escapeArg(username)
                    + " -p " + escapeArg(password)
                    + " -n " + escapeArg(name)
                    + " -m " + escapeArg(email)
                    + " -g " + QString::number(privilege);
    if (!sendCommandLine(command, PendingAction::AdminAddUser)) {
        QMessageBox::warning(this, QString::fromUtf8("发送失败"), QString::fromUtf8("无法发送创建请求。"));
    }
}

void MainWindow::onLogoutRequested() {
    if (pendingAction != PendingAction::None) {
        QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("当前有请求正在处理中，请稍后再试。"));
        return;
    }

    if (!isLoggedIn) {
        QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("当前尚未登录。"));
        return;
    }

    if (currentUsername == "root") {
        resetAuthState();
        QMessageBox::information(this, QString::fromUtf8("已退出"), QString::fromUtf8("已退出本地 root 会话。"));
        return;
    }

    const QString command = "logout -u " + escapeArg(currentUsername);
    if (!sendCommandLine(command, PendingAction::Logout)) {
        QMessageBox::warning(this, QString::fromUtf8("发送失败"), QString::fromUtf8("无法发送登出请求，请检查网络连接。"));
    }
}

void MainWindow::onProfileRequested() {
    if (pendingAction == PendingAction::QueryProfile) {
        showProfileDialogOnQuery = true;
        return;
    }

    if (pendingAction != PendingAction::None) {
        QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("当前有请求正在处理中，请稍后再试。"));
        return;
    }

    if (!isLoggedIn) {
        QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("请先登录后再查看个人信息。"));
        return;
    }

    showProfileDialogOnQuery = true;
    const QString command = "query_profile -c " + escapeArg(currentUsername)
                          + " -u " + escapeArg(currentUsername);
    if (!sendCommandLine(command, PendingAction::QueryProfile)) {
        showProfileDialogOnQuery = false;
        QMessageBox::warning(this, QString::fromUtf8("发送失败"), QString::fromUtf8("无法发送查询请求，请检查网络连接。"));
    }
}

bool MainWindow::sendCommandLine(const QString &commandLine, PendingAction action) {
    if (tcpClient == nullptr) {
        qWarning() << "tcpClient is null";
        return false;
    }
    if (!tcpClient->isConnected()) {
        qWarning() << "tcpClient is not connected";
        return false;
    }

    const qint64 timestamp = QDateTime::currentSecsSinceEpoch();
    const QString rawCommand = "[" + QString::number(timestamp) + "] " + commandLine;
    try {
        sjtu::TokenStream stream(rawCommand.toStdString());
        sjtu::Command command(stream);
        if (!tcpClient->sendObject(1001, command)) {
            return false;
        }
        pendingAction = action;
        return true;
    } catch (const std::exception &e) {
        qWarning() << "Failed to build command:" << e.what();
        return false;
    }
}

void MainWindow::processServerResult(sjtu::ResultType type, const sjtu::Result &result) {
    if (pendingAction == PendingAction::None) {
        return;
    }

    if (pendingAction == PendingAction::Login) {
        if (type == sjtu::ResultType::Success) {
            pendingAction = PendingAction::PostLoginQueryProfile;
            const QString command = "query_profile -c " + escapeArg(pendingLoginUsername)
                                  + " -u " + escapeArg(pendingLoginUsername);
            if (!sendCommandLine(command, PendingAction::PostLoginQueryProfile)) {
                pendingAction = PendingAction::None;
                pendingLoginUsername.clear();
                QMessageBox::warning(this, QString::fromUtf8("登录失败"), QString::fromUtf8("无法查询用户信息，请重试。"));
            }
            return;
        }
        pendingAction = PendingAction::None;
        pendingLoginUsername.clear();
        QMessageBox::warning(this, QString::fromUtf8("登录失败"), QString::fromUtf8("用户名或密码错误，或该用户已登录。"));
        return;
    }

    if (pendingAction == PendingAction::PostLoginQueryProfile) {
        pendingAction = PendingAction::None;
        if (type == sjtu::ResultType::Profile) {
            const auto *profile = dynamic_cast<const sjtu::ProfileResult *>(&result);
            if (profile == nullptr) {
                pendingLoginUsername.clear();
                QMessageBox::warning(this, QString::fromUtf8("登录失败"), QString::fromUtf8("用户信息解析失败。"));
                return;
            }
            currentUsername = QString::fromStdString(profile->username());
            currentName = QString::fromStdString(profile->name());
            currentEmail = QString::fromStdString(profile->email());
            currentPrivilege = profile->privilege();
            isLoggedIn = true;
            pendingLoginUsername.clear();
            applyAuthState();
            QMessageBox::information(this, QString::fromUtf8("登录成功"),
                QString::fromUtf8("欢迎，") + currentName + QString::fromUtf8("！"));
            return;
        }
        pendingLoginUsername.clear();
        QMessageBox::warning(this, QString::fromUtf8("登录失败"), QString::fromUtf8("登录凭据无效或查询被拒绝。"));
        return;
    }

    if (pendingAction == PendingAction::Register) {
        pendingAction = PendingAction::None;
        if (type == sjtu::ResultType::Success) {
            QMessageBox::information(this, QString::fromUtf8("注册成功"), QString::fromUtf8("用户注册成功。"));
        } else {
            QMessageBox::warning(this, QString::fromUtf8("注册失败"), QString::fromUtf8("注册请求被服务器拒绝。"));
        }
        return;
    }

    if (pendingAction == PendingAction::Logout) {
        pendingAction = PendingAction::None;
        if (type == sjtu::ResultType::Success) {
            resetAuthState();
            if (!isShuttingDown) {
                QMessageBox::information(this, QString::fromUtf8("已退出"), QString::fromUtf8("当前用户已退出登录。"));
            }
        } else {
            if (!isShuttingDown) {
                QMessageBox::warning(this, QString::fromUtf8("退出失败"), QString::fromUtf8("退出登录失败，请稍后重试。"));
            }
        }
        return;
    }

    if (pendingAction == PendingAction::QueryProfile) {
        pendingAction = PendingAction::None;
        if (type != sjtu::ResultType::Profile) {
            showProfileDialogOnQuery = false;
            QMessageBox::warning(this, QString::fromUtf8("查询失败"), QString::fromUtf8("无法获取个人信息。"));
            return;
        }
        const auto *profile = dynamic_cast<const sjtu::ProfileResult *>(&result);
        if (profile == nullptr) {
            showProfileDialogOnQuery = false;
            QMessageBox::warning(this, QString::fromUtf8("查询失败"), QString::fromUtf8("个人信息解析失败。"));
            return;
        }

        currentUsername = QString::fromStdString(profile->username());
        currentName = QString::fromStdString(profile->name());
        currentEmail = QString::fromStdString(profile->email());
        currentPrivilege = profile->privilege();
        applyAuthState();
        if (showProfileDialogOnQuery) {
            profileDialog->setProfile(currentUsername, currentName, currentEmail, currentPrivilege);
            profileDialog->exec();
            showProfileDialogOnQuery = false;
        }
        return;
    }

    if (pendingAction == PendingAction::QueryTicket) {
        pendingAction = PendingAction::None;

        QVector<TicketListWidget::TicketListItem> displayTickets;

        if (type == sjtu::ResultType::Ticket) {
            const auto *ticketResult = dynamic_cast<const sjtu::TicketResult *>(&result);
            if (ticketResult == nullptr) {
                ticketPageWidget->clearTickets();
                QMessageBox::warning(this, QString::fromUtf8("查询失败"), QString::fromUtf8("车票结果解析失败。"));
                return;
            }
            const auto &tickets = ticketResult->tickets();
            displayTickets.reserve(static_cast<int>(tickets.size()));
            for (size_t i = 0; i < tickets.size(); ++i) {
                const auto &ticket = tickets[i];
                TicketListWidget::TicketListItem item;
                item.trainName = QString::fromStdString(ticket.train_id_.str());
                item.startStation = QString::fromStdString(ticket.start_station_.str());
                item.endStation = QString::fromStdString(ticket.end_station_.str());
                item.departureTime = formatDateTime(ticket.departure_date_, ticket.departure_time_);
                item.arrivalTime = formatDateTime(ticket.arrival_date_, ticket.arrival_time_);
                item.durationMinutes = ticket.duration_;
                item.departureSortKey = toMinutesKey(ticket.departure_date_, ticket.departure_time_);
                item.arrivalSortKey = toMinutesKey(ticket.arrival_date_, ticket.arrival_time_);
                item.price = ticket.price_;
                item.remain = ticket.seat_;
                displayTickets.push_back(item);
            }
        } else if (type == sjtu::ResultType::Transfer) {
            const auto *transferResult = dynamic_cast<const sjtu::TransferResult *>(&result);
            if (transferResult == nullptr) {
                ticketPageWidget->clearTickets();
                QMessageBox::warning(this, QString::fromUtf8("查询失败"), QString::fromUtf8("换乘结果解析失败。"));
                return;
            }
            const auto &transfers = transferResult->tickets();
            displayTickets.reserve(static_cast<int>(transfers.size()));

            for (size_t i = 0; i < transfers.size(); ++i) {
                const auto &tt = transfers[i];
                TicketListWidget::TicketListItem item;
                item.isTransfer = true;
                item.trainName = QString::fromStdString(tt.first_ticket_.train_id_.str())
                               + QString::fromUtf8(" → ") + QString::fromStdString(tt.second_ticket_.train_id_.str());
                item.startStation = QString::fromStdString(tt.first_ticket_.start_station_.str());
                item.endStation = QString::fromStdString(tt.second_ticket_.end_station_.str());
                item.departureTime = formatDateTime(tt.first_ticket_.departure_date_, tt.first_ticket_.departure_time_);
                item.arrivalTime = formatDateTime(tt.second_ticket_.arrival_date_, tt.second_ticket_.arrival_time_);
                item.durationMinutes = tt.duration_;
                item.departureSortKey = toMinutesKey(tt.first_ticket_.departure_date_, tt.first_ticket_.departure_time_);
                item.arrivalSortKey = toMinutesKey(tt.second_ticket_.arrival_date_, tt.second_ticket_.arrival_time_);
                item.price = tt.price_;
                item.remain = qMin(tt.first_ticket_.seat_, tt.second_ticket_.seat_);
                item.firstTrainId = QString::fromStdString(tt.first_ticket_.train_id_.str());
                item.firstFromStation = QString::fromStdString(tt.first_ticket_.start_station_.str());
                item.firstToStation = QString::fromStdString(tt.first_ticket_.end_station_.str());
                item.firstDate = formatDate(tt.first_ticket_.departure_date_);
                item.secondTrainId = QString::fromStdString(tt.second_ticket_.train_id_.str());
                item.secondFromStation = QString::fromStdString(tt.second_ticket_.start_station_.str());
                item.secondToStation = QString::fromStdString(tt.second_ticket_.end_station_.str());
                item.secondDate = formatDate(tt.second_ticket_.departure_date_);
                displayTickets.push_back(item);
            }
        } else {
            ticketPageWidget->clearTickets();
            QMessageBox::warning(this, QString::fromUtf8("查询结果"), QString::fromUtf8("未查询到可用车次。"));
            return;
        }

        if (displayTickets.isEmpty()) {
            ticketPageWidget->clearTickets();
            QMessageBox::information(this, QString::fromUtf8("查询结果"), QString::fromUtf8("未查询到可用车次。"));
            return;
        }

        currentTicketList = displayTickets;
        ticketPageWidget->setTickets(displayTickets);
        stackedPanel->setCurrentWidget(ticketPageWidget);
        return;
    }

    if (pendingAction == PendingAction::BuyTicket) {
        pendingAction = PendingAction::None;
        if (type == sjtu::ResultType::Success) {
            QMessageBox::information(this, QString::fromUtf8("购票请求已处理"),
                QString::fromUtf8("车票购买成功，或已加入候补队列。可查看订单确认状态。"));
            refreshOrders();
        } else {
            QMessageBox::warning(this, QString::fromUtf8("购票失败"), QString::fromUtf8("购票请求被拒绝，可能余票不足或参数有误。"));
        }
        return;
    }

    if (pendingAction == PendingAction::BuyTransferFirst) {
        if (type == sjtu::ResultType::Success) {
            const auto *ticketItem = [&]() -> const TicketListWidget::TicketListItem* {
                for (const auto &item : currentTicketList) {
                    if (item.isTransfer) return &item;
                }
                return nullptr;
            }();
            if (ticketItem == nullptr) {
                pendingAction = PendingAction::None;
                QMessageBox::warning(this, QString::fromUtf8("购票失败"), QString::fromUtf8("换乘信息丢失。"));
                return;
            }
            const QString secondCmd = "buy_ticket -u " + escapeArg(currentUsername)
                        + " -i " + escapeArg(ticketItem->secondTrainId)
                        + " -d " + ticketItem->secondDate
                        + " -n " + QString::number(pendingTransferCount)
                        + " -f " + escapeArg(ticketItem->secondFromStation)
                        + " -t " + escapeArg(ticketItem->secondToStation)
                        + " -q " + (pendingTransferUseQueue ? "true" : "false");
            if (!sendCommandLine(secondCmd, PendingAction::BuyTransferSecond)) {
                pendingAction = PendingAction::None;
                QMessageBox::warning(this, QString::fromUtf8("部分失败"),
                    QString::fromUtf8("第一段购票成功，但第二段发送失败。请检查订单。"));
                refreshOrders();
            }
        } else {
            pendingAction = PendingAction::None;
            QMessageBox::warning(this, QString::fromUtf8("购票失败"),
                QString::fromUtf8("换乘第一段购票被拒绝，可能余票不足或参数有误。"));
        }
        return;
    }

    if (pendingAction == PendingAction::BuyTransferSecond) {
        pendingAction = PendingAction::None;
        if (type == sjtu::ResultType::Success) {
            QMessageBox::information(this, QString::fromUtf8("购票成功"),
                QString::fromUtf8("换乘车票两段均购买成功。可查看订单确认状态。"));
        } else {
            QMessageBox::warning(this, QString::fromUtf8("部分失败"),
                QString::fromUtf8("换乘第一段购买成功，但第二段被拒绝。请查看订单。"));
        }
        refreshOrders();
        return;
    }

    if (pendingAction == PendingAction::RefundTicket) {
        pendingAction = PendingAction::None;
        if (type == sjtu::ResultType::Success) {
            QMessageBox::information(this, QString::fromUtf8("退票成功"), QString::fromUtf8("车票已成功退还。"));
            refreshOrders();
        } else {
            QMessageBox::warning(this, QString::fromUtf8("退票失败"), QString::fromUtf8("退票请求被拒绝。"));
        }
        return;
    }

    if (pendingAction == PendingAction::QueryOrder) {
        pendingAction = PendingAction::None;
        if (type != sjtu::ResultType::Order) {
            orderPageWidget->clearOrders();
            QMessageBox::warning(this, QString::fromUtf8("查询失败"), QString::fromUtf8("无法获取订单信息。"));
            return;
        }
        const auto *orderResult = dynamic_cast<const sjtu::OrderResult *>(&result);
        if (orderResult == nullptr) {
            orderPageWidget->clearOrders();
            return;
        }

        const auto &orders = orderResult->orders();
        QVector<OrdersPageWidget::OrderItem> displayOrders;
        displayOrders.reserve(static_cast<int>(orders.size()));

        for (size_t i = 0; i < orders.size(); ++i) {
            const auto &order = orders[i];
            OrdersPageWidget::OrderItem item;
            item.index = static_cast<int>(i + 1);
            item.trainName = QString::fromStdString(order.ticket_.train_id_.str());
            item.startStation = QString::fromStdString(order.ticket_.start_station_.str());
            item.endStation = QString::fromStdString(order.ticket_.end_station_.str());
            item.departureTime = formatDateTime(order.ticket_.departure_date_, order.ticket_.departure_time_);
            item.arrivalTime = formatDateTime(order.ticket_.arrival_date_, order.ticket_.arrival_time_);
            item.price = order.ticket_.price_;
            item.count = order.ticket_.seat_;
            item.status = static_cast<int>(order.status_);
            displayOrders.push_back(item);
        }

        orderPageWidget->setOrders(displayOrders);
        return;
    }

    if (pendingAction == PendingAction::QueryTrain) {
        pendingAction = PendingAction::None;
        if (type != sjtu::ResultType::Train) {
            managePageWidget->showTrainResult(
                QString::fromUtf8("<span style='color:#dc2626;'>查询失败或列车不存在。</span>"));
            return;
        }
        const auto *trainResult = dynamic_cast<const sjtu::TrainResult *>(&result);
        if (trainResult == nullptr) {
            managePageWidget->showTrainResult(
                QString::fromUtf8("<span style='color:#dc2626;'>结果解析失败。</span>"));
            return;
        }

        const auto &train = trainResult->train();
        QString html = QString::fromUtf8("<b style='color:#16a34a;'>查询成功</b><br>");
        html += QString::fromUtf8("列车编号: <b>") + QString::fromStdString(train.train_id_.str()) + "</b>";
        html += QString::fromUtf8(" &nbsp; 类型: <b>") + QChar(train.type_) + "</b>";
        html += QString::fromUtf8(" &nbsp; 站点数: <b>") + QString::number(train.station_num_) + "</b><br>";
        html += QString::fromUtf8("<hr style='border-color:#e2e8f0;'>");
        html += QString::fromUtf8("站点信息:<br>");
        for (int i = 0; i < train.station_num_; ++i) {
            const auto &st = train.stations_[i];
            html += QString::fromUtf8("&nbsp;&nbsp;") + QString::number(i + 1) + ". "
                  + QString::fromStdString(st.station_name_.str());
            if (st.has_arrival_) {
                html += QString::fromUtf8(" 到:") + formatTime(st.arrival_time_);
            }
            if (st.has_leaving_) {
                html += QString::fromUtf8(" 发:") + formatTime(st.leaving_time_);
            }
            html += QString::fromUtf8(" ¥") + QString::number(st.price_);
            html += QString::fromUtf8(" 余:") + QString::number(st.seat_);
            html += "<br>";
        }
        managePageWidget->showTrainResult(html);
        return;
    }

    if (pendingAction == PendingAction::QueryTrainSchedule) {
        pendingAction = PendingAction::None;
        if (type != sjtu::ResultType::Train) {
            QMessageBox::warning(this, QString::fromUtf8("查询失败"), QString::fromUtf8("无法查询列车时刻表。"));
            return;
        }
        const auto *trainResult = dynamic_cast<const sjtu::TrainResult *>(&result);
        if (trainResult == nullptr) {
            QMessageBox::warning(this, QString::fromUtf8("查询失败"), QString::fromUtf8("时刻表解析失败。"));
            return;
        }

        const auto &train = trainResult->train();
        QString html = QString::fromUtf8("<b>") + QString::fromStdString(train.train_id_.str())
                     + "</b> " + QChar(train.type_) + QString::fromUtf8(" 时刻表<br><br>");
        for (int i = 0; i < train.station_num_; ++i) {
            const auto &st = train.stations_[i];
            html += QString::fromUtf8("&nbsp;&nbsp;") + QString::number(i + 1) + ". "
                  + QString::fromStdString(st.station_name_.str());
            if (st.has_arrival_) {
                html += QString::fromUtf8(" 到:") + formatTime(st.arrival_time_);
            }
            if (st.has_leaving_) {
                html += QString::fromUtf8(" 发:") + formatTime(st.leaving_time_);
            }
            html += QString::fromUtf8(" ¥") + QString::number(st.price_);
            html += QString::fromUtf8(" 余票:") + QString::number(st.seat_);
            html += "<br>";
        }
        QMessageBox::information(this, QString::fromUtf8("列车时刻表"), html);
        return;
    }

    if (pendingAction == PendingAction::ReleaseTrain) {
        pendingAction = PendingAction::None;
        if (type == sjtu::ResultType::Success) {
            managePageWidget->showTrainResult(
                QString::fromUtf8("<span style='color:#16a34a;'>列车发布成功。</span>"));
        } else {
            managePageWidget->showTrainResult(
                QString::fromUtf8("<span style='color:#dc2626;'>发布失败，列车可能不存在或已发布。</span>"));
        }
        return;
    }

    if (pendingAction == PendingAction::DeleteTrain) {
        pendingAction = PendingAction::None;
        if (type == sjtu::ResultType::Success) {
            managePageWidget->showTrainResult(
                QString::fromUtf8("<span style='color:#16a34a;'>列车已成功删除。</span>"));
        } else {
            managePageWidget->showTrainResult(
                QString::fromUtf8("<span style='color:#dc2626;'>删除失败，列车可能已发布或不存在。</span>"));
        }
        return;
    }

    if (pendingAction == PendingAction::AddTrain) {
        pendingAction = PendingAction::None;
        if (type == sjtu::ResultType::Success) {
            managePageWidget->showTrainResult(
                QString::fromUtf8("<span style='color:#16a34a;'>列车添加成功。</span>"));
        } else {
            managePageWidget->showTrainResult(
                QString::fromUtf8("<span style='color:#dc2626;'>添加失败，请检查参数是否正确。</span>"));
        }
        return;
    }

    if (pendingAction == PendingAction::ImportTrain) {
        if (type == sjtu::ResultType::Success) {
            ++importTrainsSuccess;
            const QString cmd = importTrainsList.at(importTrainsIndex);
            QRegularExpression re("-i\\s+(\\S+)");
            QRegularExpressionMatch match = re.match(cmd);
            if (match.hasMatch()) {
                importedTrainIds.append(match.captured(1));
            }
        } else {
            ++importTrainsFail;
        }
        ++importTrainsIndex;
        sendNextImportTrain();
        return;
    }

    if (pendingAction == PendingAction::BatchRelease) {
        if (type == sjtu::ResultType::Success) {
            ++batchReleaseSuccess;
        } else {
            ++batchReleaseFail;
        }
        ++batchReleaseIndex;
        sendNextBatchRelease();
        return;
    }

    if (pendingAction == PendingAction::AdminQueryProfile) {
        pendingAction = PendingAction::None;
        if (type != sjtu::ResultType::Profile) {
            managePageWidget->showUserResult(
                QString::fromUtf8("<span style='color:#dc2626;'>查询失败，用户不存在或权限不足。</span>"));
            return;
        }
        const auto *profile = dynamic_cast<const sjtu::ProfileResult *>(&result);
        if (profile == nullptr) {
            managePageWidget->showUserResult(
                QString::fromUtf8("<span style='color:#dc2626;'>结果解析失败。</span>"));
            return;
        }
        QString html = QString::fromUtf8("<b style='color:#16a34a;'>查询成功</b><br>");
        html += QString::fromUtf8("用户名: <b>") + QString::fromStdString(profile->username()) + "</b><br>";
        html += QString::fromUtf8("姓名: <b>") + QString::fromStdString(profile->name()) + "</b><br>";
        html += QString::fromUtf8("邮箱: <b>") + QString::fromStdString(profile->email()) + "</b><br>";
        html += QString::fromUtf8("权限等级: <b>") + QString::number(profile->privilege()) + "</b>";
        managePageWidget->showUserResult(html);
        return;
    }

    if (pendingAction == PendingAction::AdminAddUser) {
        pendingAction = PendingAction::None;
        if (type == sjtu::ResultType::Success) {
            managePageWidget->showUserResult(
                QString::fromUtf8("<span style='color:#16a34a;'>用户创建成功。</span>"));
        } else {
            managePageWidget->showUserResult(
                QString::fromUtf8("<span style='color:#dc2626;'>创建失败，用户可能已存在或权限不足。</span>"));
        }
        return;
    }
}

void MainWindow::applyAuthState() {
    const bool admin = isLoggedIn && currentPrivilege >= 2;
    topBar->setAuthState(isLoggedIn, currentUsername, admin);
}

void MainWindow::resetAuthState() {
    isLoggedIn = false;
    currentUsername.clear();
    currentName.clear();
    currentEmail.clear();
    currentPrivilege = 0;
    applyAuthState();
}

void MainWindow::tryGracefulLogoutBeforeExit() {
    if (!isLoggedIn || currentUsername == "root") {
        return;
    }
    if (pendingAction != PendingAction::None) {
        return;
    }
    if (tcpClient == nullptr || !tcpClient->isConnected()) {
        return;
    }

    const QString command = "logout -u " + escapeArg(currentUsername);
    if (!sendCommandLine(command, PendingAction::Logout)) {
        pendingAction = PendingAction::None;
        return;
    }

    QEventLoop waitLoop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    connect(&timeoutTimer, &QTimer::timeout, &waitLoop, &QEventLoop::quit);

    QTimer pollTimer;
    pollTimer.setInterval(20);
    connect(&pollTimer, &QTimer::timeout, this, [&]() {
        if (pendingAction == PendingAction::None) {
            waitLoop.quit();
        }
    });

    timeoutTimer.start(1200);
    pollTimer.start();
    waitLoop.exec();

    if (pendingAction == PendingAction::Logout) {
        pendingAction = PendingAction::None;
    }
}

void MainWindow::onAdminImportTrainsRequested(const QString &filePath) {
    if (pendingAction != PendingAction::None) {
        QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("当前有请求正在处理中，请稍后再试。"));
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, QString::fromUtf8("导入失败"), QString::fromUtf8("无法打开文件。"));
        return;
    }

    QTextStream in(&file);
    importTrainsList.clear();
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (!line.isEmpty() && line.startsWith("add_train")) {
            importTrainsList.append(line);
        }
    }
    file.close();

    if (importTrainsList.isEmpty()) {
        QMessageBox::information(this, QString::fromUtf8("导入"), QString::fromUtf8("文件中没有找到 add_train 语句。"));
        return;
    }

    importTrainsIndex = 0;
    importTrainsSuccess = 0;
    importTrainsFail = 0;
    importedTrainIds.clear();
    sendNextImportTrain();
}

void MainWindow::sendNextImportTrain() {
    if (importTrainsIndex >= importTrainsList.size()) {
        const int total = importTrainsList.size();
        managePageWidget->showTrainResult(
            QString::fromUtf8("<span style='color:#16a34a;'>导入完成！</span>")
            + QString::fromUtf8("共 ") + QString::number(total) + QString::fromUtf8(" 条：")
            + QString::fromUtf8("成功 ") + QString::number(importTrainsSuccess)
            + QString::fromUtf8("，失败 ") + QString::number(importTrainsFail));
        pendingAction = PendingAction::None;
        return;
    }

    const QString command = importTrainsList.at(importTrainsIndex);
    if (!sendCommandLine(command, PendingAction::ImportTrain)) {
        QMessageBox::warning(this, QString::fromUtf8("发送失败"),
            QString::fromUtf8("导入中断：无法发送第 ")
            + QString::number(importTrainsIndex + 1) + QString::fromUtf8(" 条命令。"));
        managePageWidget->showTrainResult(
            QString::fromUtf8("<span style='color:#dc2626;'>导入中断：成功 ") + QString::number(importTrainsSuccess)
            + QString::fromUtf8("，失败 ") + QString::number(importTrainsFail + (importTrainsList.size() - importTrainsIndex))
            + QString::fromUtf8("</span>"));
        pendingAction = PendingAction::None;
    }
}

void MainWindow::onAdminBatchReleaseRequested() {
    if (pendingAction != PendingAction::None) {
        QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("当前有请求正在处理中，请稍后再试。"));
        return;
    }

    if (importedTrainIds.isEmpty()) {
        managePageWidget->showTrainResult(
            QString::fromUtf8("<span style='color:#d97706;'>没有可发布的列车。请先导入火车信息。</span>"));
        return;
    }

    batchReleaseIndex = 0;
    batchReleaseSuccess = 0;
    batchReleaseFail = 0;
    sendNextBatchRelease();
}

void MainWindow::sendNextBatchRelease() {
    if (batchReleaseIndex >= importedTrainIds.size()) {
        const int total = importedTrainIds.size();
        managePageWidget->showTrainResult(
            QString::fromUtf8("<span style='color:#16a34a;'>批量发布完成！</span>")
            + QString::fromUtf8("共 ") + QString::number(total) + QString::fromUtf8(" 列：")
            + QString::fromUtf8("成功 ") + QString::number(batchReleaseSuccess)
            + QString::fromUtf8("，失败 ") + QString::number(batchReleaseFail));
        pendingAction = PendingAction::None;
        return;
    }

    const QString command = "release_train -i " + escapeArg(importedTrainIds.at(batchReleaseIndex));
    if (!sendCommandLine(command, PendingAction::BatchRelease)) {
        QMessageBox::warning(this, QString::fromUtf8("发送失败"),
            QString::fromUtf8("批量发布中断：无法发送第 ")
            + QString::number(batchReleaseIndex + 1) + QString::fromUtf8(" 条命令。"));
        managePageWidget->showTrainResult(
            QString::fromUtf8("<span style='color:#dc2626;'>批量发布中断：成功 ") + QString::number(batchReleaseSuccess)
            + QString::fromUtf8("，失败 ") + QString::number(batchReleaseFail + (importedTrainIds.size() - batchReleaseIndex))
            + QString::fromUtf8("</span>"));
        pendingAction = PendingAction::None;
    }
}

QString MainWindow::escapeArg(const QString &arg) {
    if (arg.contains(' ')) {
        qWarning() << "argument contains space, which is not allowed:" << arg;
    }
    QString escaped = arg;
    escaped.replace(' ', '_');
    return escaped;
}

void MainWindow::handleAuthChanged(const QString &msg) {
}

void MainWindow::initializeComponents() {
    homePageWidget = new HomePageWidget(stackedPanel);
    ticketPageWidget = new TicketListWidget(stackedPanel);
    orderPageWidget = new OrdersPageWidget(stackedPanel);
    managePageWidget = new AdminPageWidget(stackedPanel);

    stackedPanel->addWidget(homePageWidget);
    stackedPanel->addWidget(ticketPageWidget);
    stackedPanel->addWidget(orderPageWidget);
    stackedPanel->addWidget(managePageWidget);
    stackedPanel->setCurrentWidget(homePageWidget);

    connect(homePageWidget, &HomePageWidget::queryTicketRequested,
            this, &MainWindow::onQueryTicketRequested);

    connect(ticketPageWidget, &TicketListWidget::trainNameClicked,
            this, &MainWindow::onTrainScheduleRequested);

    connect(ticketPageWidget, &TicketListWidget::purchaseRequested,
            this, &MainWindow::onBuyTicketRequested);

    connect(orderPageWidget, &OrdersPageWidget::refundRequested,
            this, &MainWindow::onRefundRequested);

    connect(orderPageWidget, &OrdersPageWidget::refreshRequested,
            this, &MainWindow::onOrderRefreshRequested);

    connect(managePageWidget, &AdminPageWidget::queryTrainRequested,
            this, &MainWindow::onAdminQueryTrainRequested);
    connect(managePageWidget, &AdminPageWidget::releaseTrainRequested,
            this, &MainWindow::onAdminReleaseTrainRequested);
    connect(managePageWidget, &AdminPageWidget::deleteTrainRequested,
            this, &MainWindow::onAdminDeleteTrainRequested);
    connect(managePageWidget, &AdminPageWidget::addTrainRequested,
            this, &MainWindow::onAdminAddTrainRequested);
    connect(managePageWidget, &AdminPageWidget::importTrainsRequested,
            this, &MainWindow::onAdminImportTrainsRequested);
    connect(managePageWidget, &AdminPageWidget::batchReleaseRequested,
            this, &MainWindow::onAdminBatchReleaseRequested);
    connect(managePageWidget, &AdminPageWidget::queryProfileRequested,
            this, &MainWindow::onAdminQueryProfileRequested);
    connect(managePageWidget, &AdminPageWidget::addUserRequested,
            this, &MainWindow::onAdminAddUserRequested);

    connect(topBar, &TopBar::mainButtonClicked, this, [&]() {
        stackedPanel->setCurrentWidget(homePageWidget);
    });
    connect(topBar, &TopBar::ticketButtonClicked, this, [&]() {
        stackedPanel->setCurrentWidget(ticketPageWidget);
    });
    connect(topBar, &TopBar::orderButtonClicked, this, [&]() {
        stackedPanel->setCurrentWidget(orderPageWidget);
        if (isLoggedIn) {
            onOrderPageActivated();
        }
    });
    connect(topBar, &TopBar::manageButtonClicked, this, [&]() {
        stackedPanel->setCurrentWidget(managePageWidget);
    });

    initialized = true;
}

} // namespace sjtu::client
