#include "client/widgets/orders_page_widget.hpp"

#include <QHeaderView>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace {

QString statusText(int status) {
    switch (status) {
    case 1: return QString::fromUtf8("已购");
    case 2: return QString::fromUtf8("待候");
    case 3: return QString::fromUtf8("已退");
    default: return QString::fromUtf8("无效");
    }
}

QColor statusColor(int status) {
    switch (status) {
    case 1: return QColor("#16a34a");
    case 2: return QColor("#d97706");
    case 3: return QColor("#dc2626");
    default: return QColor("#6b7280");
    }
}

QString formatDuration(int minutes) {
    if (minutes <= 0) return "0m";
    const int hours = minutes / 60;
    const int mins = minutes % 60;
    if (hours == 0) return QString::number(mins) + "m";
    if (mins == 0) return QString::number(hours) + "h";
    return QString::number(hours) + "h " + QString::number(mins) + "m";
}

void centerItem(QTableWidgetItem *item) {
    if (item != nullptr) item->setTextAlignment(Qt::AlignCenter);
}

} // namespace

namespace sjtu::client {

OrdersPageWidget::OrdersPageWidget(QWidget *parent)
    : QWidget(parent), titleLabel(nullptr), refreshButton(nullptr), tableWidget(nullptr) {
    setObjectName("OrdersPage");
    setAttribute(Qt::WA_StyledBackground);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 18, 24, 18);
    layout->setSpacing(12);

    QHBoxLayout *headerRow = new QHBoxLayout();
    titleLabel = new QLabel(QString::fromUtf8("我的订单"), this);
    titleLabel->setObjectName("OrdersTitle");
    headerRow->addWidget(titleLabel);
    headerRow->addStretch(1);

    refreshButton = new QPushButton(QString::fromUtf8("刷新"), this);
    refreshButton->setObjectName("OrdersRefresh");
    headerRow->addWidget(refreshButton);
    layout->addLayout(headerRow);

    tableWidget = new QTableWidget(this);
    tableWidget->setObjectName("OrdersTable");
    tableWidget->setColumnCount(10);
    tableWidget->setHorizontalHeaderLabels(
        {QString::fromUtf8("序号"), QString::fromUtf8("车次"), QString::fromUtf8("出发站"),
         QString::fromUtf8("到达站"), QString::fromUtf8("出发时间"), QString::fromUtf8("到达时间"),
         QString::fromUtf8("价格"), QString::fromUtf8("张数"), QString::fromUtf8("状态"), QString::fromUtf8("操作")}
    );
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    tableWidget->verticalHeader()->setVisible(false);
    tableWidget->setAlternatingRowColors(true);
    tableWidget->setSortingEnabled(true);

    QHeaderView *header = tableWidget->horizontalHeader();
    header->setSectionsClickable(true);
    header->setSortIndicatorShown(true);
    header->setStretchLastSection(false);
    header->setSectionResizeMode(QHeaderView::Fixed);

    tableWidget->setColumnWidth(0, 50);
    tableWidget->setColumnWidth(1, 110);
    tableWidget->setColumnWidth(2, 110);
    tableWidget->setColumnWidth(3, 110);
    tableWidget->setColumnWidth(4, 150);
    tableWidget->setColumnWidth(5, 150);
    tableWidget->setColumnWidth(6, 80);
    tableWidget->setColumnWidth(7, 60);
    tableWidget->setColumnWidth(8, 60);
    tableWidget->setColumnWidth(9, 80);

    layout->addWidget(tableWidget, 1);

    connect(refreshButton, &QPushButton::clicked, this, &OrdersPageWidget::refreshRequested);

    setStyleSheet(R"(
        #OrdersPage {
            background-color: #eef3fb;
        }

        QLabel#OrdersTitle {
            color: #1e3a5f;
            font-size: 22px;
            font-weight: 700;
        }

        QPushButton#OrdersRefresh {
            border: none;
            border-radius: 6px;
            padding: 6px 14px;
            color: #ffffff;
            background-color: #3d80de;
        }

        QPushButton#OrdersRefresh:hover {
            background-color: #2f6fc6;
        }

        QTableWidget#OrdersTable {
            border: 1px solid #d9e3f0;
            border-radius: 10px;
            background-color: #ffffff;
            gridline-color: #edf2f7;
            alternate-background-color: #f8fbff;
        }

        QHeaderView::section {
            background-color: #f0f6ff;
            border: none;
            border-bottom: 1px solid #d9e3f0;
            color: #355070;
            font-weight: 600;
            padding: 8px;
        }

        QTableWidget#OrdersTable QPushButton {
            min-width: 60px;
            border: none;
            border-radius: 6px;
            padding: 5px 8px;
            color: #ffffff;
            background-color: #dc2626;
        }

        QTableWidget#OrdersTable QPushButton:hover {
            background-color: #b91c1c;
        }
    )");
}

void OrdersPageWidget::clearOrders() {
    tableWidget->setRowCount(0);
}

void OrdersPageWidget::setOrders(const QVector<OrderItem> &orders) {
    const bool sortingEnabled = tableWidget->isSortingEnabled();
    tableWidget->setSortingEnabled(false);
    clearOrders();
    tableWidget->setRowCount(orders.size());

    for (int row = 0; row < orders.size(); ++row) {
        const OrderItem &order = orders[row];

        auto *indexItem = new QTableWidgetItem(QString::number(order.index));
        centerItem(indexItem);
        tableWidget->setItem(row, 0, indexItem);

        QTableWidgetItem *trainItem = new QTableWidgetItem(order.trainName);
        QFont trainFont = trainItem->font();
        trainFont.setUnderline(true);
        trainItem->setFont(trainFont);
        trainItem->setForeground(QColor("#1d4ed8"));
        centerItem(trainItem);
        tableWidget->setItem(row, 1, trainItem);

        auto *startItem = new QTableWidgetItem(order.startStation);
        auto *endItem = new QTableWidgetItem(order.endStation);
        auto *depItem = new QTableWidgetItem(order.departureTime);
        auto *arrItem = new QTableWidgetItem(order.arrivalTime);
        auto *priceItem = new QTableWidgetItem(QString::number(order.price));
        auto *countItem = new QTableWidgetItem(QString::number(order.count));
        centerItem(startItem);
        centerItem(endItem);
        centerItem(depItem);
        centerItem(arrItem);
        centerItem(priceItem);
        centerItem(countItem);

        tableWidget->setItem(row, 2, startItem);
        tableWidget->setItem(row, 3, endItem);
        tableWidget->setItem(row, 4, depItem);
        tableWidget->setItem(row, 5, arrItem);
        tableWidget->setItem(row, 6, priceItem);
        tableWidget->setItem(row, 7, countItem);

        QTableWidgetItem *statusItem = new QTableWidgetItem(statusText(order.status));
        statusItem->setForeground(statusColor(order.status));
        QFont statusFont = statusItem->font();
        statusFont.setBold(true);
        statusItem->setFont(statusFont);
        centerItem(statusItem);
        tableWidget->setItem(row, 8, statusItem);

        QWidget *buttonHost = new QWidget(tableWidget);
        QHBoxLayout *buttonLayout = new QHBoxLayout(buttonHost);
        buttonLayout->setContentsMargins(4, 2, 4, 2);

        if (order.status == 1 || order.status == 2) {
            QPushButton *refundButton = new QPushButton(QString::fromUtf8("退票"), this);
            int orderIdx = order.index;
            connect(refundButton, &QPushButton::clicked, this, [this, orderIdx]() {
                onRefundClicked(orderIdx);
            });
            buttonLayout->addWidget(refundButton, 0, Qt::AlignCenter);
        }
        tableWidget->setCellWidget(row, 9, buttonHost);
    }

    tableWidget->setSortingEnabled(sortingEnabled);
}

void OrdersPageWidget::onRefundClicked(int orderIndex) {
    emit refundRequested(orderIndex);
}

} // namespace sjtu::client
