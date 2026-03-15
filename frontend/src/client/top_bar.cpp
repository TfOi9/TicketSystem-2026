#include "../../include/client/top_bar.hpp"

#include <QLayout>
#include <QMenu>
#include <QAction>

namespace sjtu::client {

TopBar::TopBar(QWidget *parent) : QWidget(parent) {
    setObjectName("TopBar");
    setAttribute(Qt::WA_StyledBackground);
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(20, 10, 20, 10);
    layout->setSpacing(5);

    titleLabel = new QLabel("TicketSystem", this);
    loginButton = new QPushButton("登录", this);
    registerButton = new QPushButton("注册", this);
    mainButton = new QPushButton("首页", this);
    ticketButton = new QPushButton("购票", this);
    orderButton = new QPushButton("订单", this);
    manageButton = new QPushButton("管理", this);

    userButton = new QPushButton(this);
    userButton->setObjectName("userButton");
    userButton->setFlat(true);
    userButton->setCursor(Qt::PointingHandCursor);

    layout->addWidget(titleLabel, 0, Qt::AlignLeft);
    layout->addWidget(mainButton);
    layout->addWidget(ticketButton);
    layout->addWidget(orderButton);
    layout->addWidget(manageButton);
    layout->addStretch(1);
    layout->addWidget(loginButton);
    layout->addWidget(registerButton);
    layout->addWidget(userButton);

    setMinimumHeight(50);

    connect(loginButton, &QPushButton::clicked, this, &TopBar::onLoginButtonClicked);
    connect(registerButton, &QPushButton::clicked, this, &TopBar::onRegisterButtonClicked);
    connect(mainButton, &QPushButton::clicked, this, &TopBar::onMainButtonClicked);
    connect(ticketButton, &QPushButton::clicked, this, &TopBar::onTicketButtonClicked);
    connect(orderButton, &QPushButton::clicked, this, &TopBar::onOrderButtonClicked);
    connect(manageButton, &QPushButton::clicked, this, &TopBar::onManageButtonClicked);
    connect(userButton, &QPushButton::clicked, this, &TopBar::onProfileButtonClicked);

    QMenu *userMenu = new QMenu(userButton);
    QAction *profileAction = userMenu->addAction("个人信息");
    QAction *orderAction = userMenu->addAction("我的订单");
    QAction *logoutAction = userMenu->addAction("退出登录");
    
    connect(profileAction, &QAction::triggered, this, &TopBar::onProfileButtonClicked);
    connect(orderAction, &QAction::triggered, this, &TopBar::onOrderButtonClicked);
    connect(logoutAction, &QAction::triggered, this, &TopBar::onLogoutButtonClicked);

    userButton->setMenu(userMenu);

    applyStyle();

}

void TopBar::onLoginButtonClicked() {
    // todo
}

void TopBar::onRegisterButtonClicked() {
    // todo
}

void TopBar::onLogoutButtonClicked() {
    // todo
}

void TopBar::onProfileButtonClicked() {
    // todo
}

void TopBar::onMainButtonClicked() {
    emit mainButtonClicked();
}

void TopBar::onTicketButtonClicked() {
    emit ticketButtonClicked();
}

void TopBar::onOrderButtonClicked() {
    emit orderButtonClicked();
}

void TopBar::onManageButtonClicked() {
    emit manageButtonClicked();
}

void TopBar::refreshBar() {
    // todo
}

void TopBar::applyStyle() {
    setStyleSheet(R"(
        #TopBar {
            background-color: #3d80de;
            border-bottom: 1px solid #e0e0e0;
        }
        
        #TopBar QLabel {
            font-size: 20px;
            font-weight: bold;
            color: #ffffff;
        }
        
        #TopBar QPushButton {
            background-color: transparent;
            border: none;
            color: #ffffff;
            font-size: 14px;
            padding: 8px 16px;
            border-radius: 4px;
        }
        
        #TopBar QPushButton:hover {
            background-color: rgba(255,255,255,0.15);
            color: #ffffff;
        }

        QPushButton#userButton {
            background-color: transparent;
            border: none;
            color: #ffffff;
            font-size: 14px;
            padding: 8px 16px;
            border-radius: 4px;
        }

        QPushButton#userButton:hover {
            background-color: rgba(255,255,255,0.15);
            color: #ffffff;
        }

        QMenu {
            background-color: #ffffff;
            border: 1px solid #e0e0e0;
            padding: 4px 0;
        }

        QMenu::item {
            padding: 8px 16px;
            color: #333333;
        }

        QMenu::item:selected {
            background-color: #f5f5f5;
        }
    )");
}

} // namespace sjtu::client