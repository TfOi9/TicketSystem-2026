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

#include <iostream>

#include "../../../include/command/command.hpp"
#include "../../../include/command/token.hpp"
#include "../../../include/result/result.hpp"

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
        isShuttingDown(false) {
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
        updateConnectionStatus("已连接");
    });

    connect(tcpClient, &sjtu::TCPClient::disconnected, this, [&]() {
        updateConnectionStatus("已断开");
        connectedViaDiscovery = false;
        pendingAction = PendingAction::None;
        pendingLoginUsername.clear();
        showProfileDialogOnQuery = false;
    });

    connect(tcpClient, &sjtu::TCPClient::error, this, [&](const QString &err) {
        updateConnectionStatus("连接失败: " + err);
        pendingAction = PendingAction::None;
        pendingLoginUsername.clear();
        showProfileDialogOnQuery = false;
    });

    connect(udpClient, &sjtu::UDPClient::stringReceived, this, &MainWindow::onServerDiscovered);
    connect(discoveryProbeTimer, &QTimer::timeout, this, &MainWindow::onDiscoveryProbeTimeout);

    updateConnectionStatus("正在发现服务器...");
}

void MainWindow::startServerDiscovery() {
    if (!udpClient->startListening(kDiscoveryPort)) {
        updateConnectionStatus("UDP 监听失败");
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
        updateConnectionStatus("未发现服务器");
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

    updateConnectionStatus("发现服务器，正在连接...");
    tcpClient->connectToServer(senderIp, kServerPort);
}

void MainWindow::updateConnectionStatus(const QString &status) {
    if (statusBarWidget) {
        statusBarWidget->setConnectionStatus(status);
    }
}

void MainWindow::onLoginRequested() {
    if (pendingAction != PendingAction::None) {
        QMessageBox::information(this, "提示", "当前有请求正在处理中，请稍后再试。");
        return;
    }

    if (isLoggedIn) {
        QMessageBox::information(this, "提示", "当前已有用户在线，请先退出当前账号再登录。");
        return;
    }

    if (loginDialog->exec() != QDialog::Accepted) {
        return;
    }

    const QString username = loginDialog->username();
    const QString password = loginDialog->password();
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "登录失败", "用户名和密码不能为空。");
        return;
    }

    pendingLoginUsername = username;
    const QString command = "login -u " + escapeArg(username) + " -p " + escapeArg(password);
    if (!sendCommandLine(command, PendingAction::Login)) {
        pendingLoginUsername.clear();
        QMessageBox::warning(this, "发送失败", "无法发送登录请求，请检查网络连接。");
    }
}

void MainWindow::onRegisterRequested() {
    if (pendingAction != PendingAction::None) {
        QMessageBox::information(this, "提示", "当前有请求正在处理中，请稍后再试。");
        return;
    }

    if (registerDialog->exec() != QDialog::Accepted) {
        return;
    }

    const QString username = registerDialog->username();
    const QString password = registerDialog->password();
    const QString name = registerDialog->displayName();
    const QString email = registerDialog->email();
    if (username.isEmpty() || password.isEmpty() || name.isEmpty() || email.isEmpty()) {
        QMessageBox::warning(this, "注册失败", "用户名、密码、姓名和邮箱不能为空。");
        return;
    }

    QString command = "add_user -c root -u " + escapeArg(username)
            + " -p " + escapeArg(password)
            + " -n " + escapeArg(name)
            + " -m " + escapeArg(email)
            + " -g 1";

    if (!sendCommandLine(command, PendingAction::Register)) {
        QMessageBox::warning(this, "发送失败", "无法发送注册请求，请检查网络连接。");
    }
}

void MainWindow::onQueryTicketRequested(const QString &fromStation, const QString &toStation, const QString &date) {
    if (pendingAction != PendingAction::None) {
        QMessageBox::information(this, "提示", "当前有请求正在处理中，请稍后再试。");
        return;
    }

    if (fromStation.isEmpty() || toStation.isEmpty()) {
        QMessageBox::warning(this, "查询失败", "出发站和到达站不能为空。");
        return;
    }
    if (fromStation == toStation) {
        QMessageBox::warning(this, "查询失败", "出发站和到达站不能相同。");
        return;
    }

    const QString command = "query_ticket -s " + escapeArg(fromStation)
                          + " -t " + escapeArg(toStation)
                          + " -d " + date
                          + " -p time";
    if (!sendCommandLine(command, PendingAction::QueryTicket)) {
        QMessageBox::warning(this, "发送失败", "无法发送查询请求，请检查网络连接。");
        return;
    }

    if (ticketPageWidget != nullptr) {
        stackedPanel->setCurrentWidget(ticketPageWidget);
    }
}

void MainWindow::onLogoutRequested() {
    if (pendingAction != PendingAction::None) {
        QMessageBox::information(this, "提示", "当前有请求正在处理中，请稍后再试。");
        return;
    }

    if (!isLoggedIn) {
        QMessageBox::information(this, "提示", "当前尚未登录。");
        return;
    }

    // Root 由服务端常驻维护，前端仅清理本地会话状态。
    if (currentUsername == "root") {
        resetAuthState();
        QMessageBox::information(this, "已退出", "已退出本地 root 会话。");
        return;
    }

    const QString command = "logout -u " + escapeArg(currentUsername);
    if (!sendCommandLine(command, PendingAction::Logout)) {
        QMessageBox::warning(this, "发送失败", "无法发送登出请求，请检查网络连接。");
    }
}

void MainWindow::onProfileRequested() {
    if (pendingAction == PendingAction::QueryProfile) {
        // 背景资料刷新未完成时，标记为完成后弹窗，避免阻塞用户操作。
        showProfileDialogOnQuery = true;
        return;
    }

    if (pendingAction != PendingAction::None) {
        QMessageBox::information(this, "提示", "当前有请求正在处理中，请稍后再试。");
        return;
    }

    if (!isLoggedIn) {
        QMessageBox::information(this, "提示", "请先登录后再查看个人信息。");
        return;
    }

    showProfileDialogOnQuery = true;
    const QString command = "query_profile -c root -u " + escapeArg(currentUsername);
    if (!sendCommandLine(command, PendingAction::QueryProfile)) {
        showProfileDialogOnQuery = false;
        QMessageBox::warning(this, "发送失败", "无法发送查询请求，请检查网络连接。");
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
            currentUsername = pendingLoginUsername;
            isLoggedIn = true;
            currentPrivilege = (currentUsername == "root") ? 10 : 1;
            pendingLoginUsername.clear();
            applyAuthState();
            pendingAction = PendingAction::None;
            showProfileDialogOnQuery = false;
            QMessageBox::information(this, "登录成功", "登录成功。");
            return;
        }
        pendingAction = PendingAction::None;
        pendingLoginUsername.clear();
        QMessageBox::warning(this, "登录失败", "用户名或密码错误，或该用户已登录。");
        return;
    }

    if (pendingAction == PendingAction::Register) {
        pendingAction = PendingAction::None;
        if (type == sjtu::ResultType::Success) {
            QMessageBox::information(this, "注册成功", "用户注册成功。");
        } else {
            QMessageBox::warning(this, "注册失败", "注册请求被服务器拒绝。");
        }
        return;
    }

    if (pendingAction == PendingAction::Logout) {
        pendingAction = PendingAction::None;
        if (type == sjtu::ResultType::Success) {
            resetAuthState();
            if (!isShuttingDown) {
                QMessageBox::information(this, "已退出", "当前用户已退出登录。");
            }
        } else {
            if (!isShuttingDown) {
                QMessageBox::warning(this, "退出失败", "退出登录失败，请稍后重试。");
            }
        }
        return;
    }

    if (pendingAction == PendingAction::QueryProfile) {
        pendingAction = PendingAction::None;
        if (type != sjtu::ResultType::Profile) {
            showProfileDialogOnQuery = false;
            QMessageBox::warning(this, "查询失败", "无法获取个人信息。");
            return;
        }
        const auto *profile = dynamic_cast<const sjtu::ProfileResult *>(&result);
        if (profile == nullptr) {
            showProfileDialogOnQuery = false;
            QMessageBox::warning(this, "查询失败", "个人信息解析失败。");
            return;
        }

        currentUsername = QString::fromStdString(profile->username());
        currentName = QString::fromStdString(profile->name());
        currentEmail = QString::fromStdString(profile->email());
        currentPrivilege = profile->privilege();
        isLoggedIn = true;
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
        if (type != sjtu::ResultType::Ticket) {
            if (ticketPageWidget != nullptr) {
                ticketPageWidget->clearTickets();
            }
            QMessageBox::warning(this, "查询结果", "未查询到可用车次。");
            return;
        }

        const auto *ticketResult = dynamic_cast<const sjtu::TicketResult *>(&result);
        if (ticketResult == nullptr) {
            if (ticketPageWidget != nullptr) {
                ticketPageWidget->clearTickets();
            }
            QMessageBox::warning(this, "查询失败", "车票结果解析失败。");
            return;
        }

        QVector<TicketListWidget::TicketListItem> displayTickets;
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
            item.price = ticket.price_;
            item.remain = ticket.seat_;
            displayTickets.push_back(item);
        }

        if (ticketPageWidget != nullptr) {
            ticketPageWidget->setTickets(displayTickets);
            stackedPanel->setCurrentWidget(ticketPageWidget);
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

QString MainWindow::escapeArg(const QString &arg) {
    QString escaped = arg;
    escaped.replace(' ', "_");
    return escaped;
}

void MainWindow::handleAuthChanged(const QString &msg) {
    // setStatusMessage(msg);
}

void MainWindow::initializeComponents() {
    homePageWidget = new HomePageWidget(stackedPanel);
    ticketPageWidget = new TicketListWidget(stackedPanel);
    orderPageWidget = new PlaceholderPageWidget("订单", stackedPanel);
    managePageWidget = new PlaceholderPageWidget("管理", stackedPanel);

    stackedPanel->addWidget(homePageWidget);
    stackedPanel->addWidget(ticketPageWidget);
    stackedPanel->addWidget(orderPageWidget);
    stackedPanel->addWidget(managePageWidget);
    stackedPanel->setCurrentWidget(homePageWidget);

    connect(homePageWidget, &HomePageWidget::queryTicketRequested,
            this, &MainWindow::onQueryTicketRequested);

    connect(ticketPageWidget, &TicketListWidget::trainNameClicked, this, [](const QString &trainName) {
        qDebug() << "Train name clicked:" << trainName;
    });

    connect(ticketPageWidget, &TicketListWidget::purchaseRequested, this, [](const QString &trainName) {
        qDebug() << "Purchase requested for train:" << trainName;
    });

    connect(topBar, &TopBar::mainButtonClicked, this, [&]() {
        stackedPanel->setCurrentWidget(homePageWidget);
    });
    connect(topBar, &TopBar::ticketButtonClicked, this, [&]() {
        stackedPanel->setCurrentWidget(ticketPageWidget);
    });
    connect(topBar, &TopBar::orderButtonClicked, this, [&]() {
        stackedPanel->setCurrentWidget(orderPageWidget);
    });
    connect(topBar, &TopBar::manageButtonClicked, this, [&]() {
        stackedPanel->setCurrentWidget(managePageWidget);
    });

    initialized = true;
}

} // namespace sjtu::client