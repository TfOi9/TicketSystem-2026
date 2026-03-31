#include "../../include/client/status_bar.hpp"

#include <QLabel>
#include <QHBoxLayout>

namespace sjtu::client {

StatusBar::StatusBar(QWidget *parent) : QWidget(parent) {
    setObjectName("StatusBar");
    setAttribute(Qt::WA_StyledBackground);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 6, 12, 6);
    layout->setSpacing(0);

    connectionStatusLabel = new QLabel("服务器状态: 未连接", this);
    layout->addWidget(connectionStatusLabel, 0, Qt::AlignLeft);
    layout->addStretch(1);

    setMinimumHeight(32);

    setStyleSheet(R"(
        #StatusBar {
            background-color: #f5f7fa;
            border-top: 1px solid #d7dde5;
        }

        #StatusBar QLabel {
            color: #2f3b4a;
            font-size: 13px;
        }
    )");
}

void StatusBar::setConnectionStatus(const QString &status) {
    connectionStatusLabel->setText("服务器状态: " + status);
}

} // namespace sjtu::client
