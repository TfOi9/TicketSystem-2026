#include "client/widgets/home_page_widget.hpp"
#include "client/widgets/ticket_query_widget.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace sjtu::client {

HomePageWidget::HomePageWidget(QWidget *parent) : QWidget(parent), queryWidget(nullptr) {
    setObjectName("HomePage");
    setAttribute(Qt::WA_StyledBackground);

    QVBoxLayout *pageLayout = new QVBoxLayout(this);
    pageLayout->setContentsMargins(26, 20, 26, 20);
    pageLayout->setSpacing(0);

    QWidget *banner = new QWidget(this);
    banner->setObjectName("HomeBanner");
    QHBoxLayout *bannerLayout = new QHBoxLayout(banner);
    bannerLayout->setContentsMargins(26, 22, 26, 22);
    bannerLayout->setSpacing(22);

    queryWidget = new TicketQueryWidget(banner);
    queryWidget->setMinimumWidth(420);
    bannerLayout->addWidget(queryWidget, 0, Qt::AlignVCenter);

    QWidget *rightBlock = new QWidget(banner);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightBlock);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->addStretch(1);

    QLabel *title = new QLabel("畅行 2025 夏季班次", rightBlock);
    title->setObjectName("HomeBannerTitle");
    QLabel *subtitle = new QLabel("在左侧输入出发站、目的地和日期，快速发起车票查询", rightBlock);
    subtitle->setObjectName("HomeBannerSubtitle");
    subtitle->setWordWrap(true);

    rightLayout->addWidget(title);
    rightLayout->addWidget(subtitle);
    rightLayout->addStretch(2);

    bannerLayout->addWidget(rightBlock, 1);
    pageLayout->addWidget(banner, 1);

    connect(queryWidget, &TicketQueryWidget::queryTicketRequested,
            this, &HomePageWidget::queryTicketRequested);

    setStyleSheet(R"(
        #HomePage {
            background-color: #eef3fb;
        }

        #HomeBanner {
            border-radius: 16px;
            background-color: #3d80de;
        }

        QLabel#HomeBannerTitle {
            font-size: 34px;
            font-weight: 700;
            color: #ffffff;
        }

        QLabel#HomeBannerSubtitle {
            font-size: 16px;
            color: #dbeafe;
            line-height: 1.4;
        }
    )");
}

} // namespace sjtu::client
