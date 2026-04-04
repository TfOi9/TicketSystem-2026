#include "client/widgets/ticket_list_widget.hpp"

#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace sjtu::client {

TicketListWidget::TicketListWidget(QWidget *parent)
    : QWidget(parent),
      titleLabel(nullptr),
      tableWidget(nullptr) {
    setObjectName("TicketListPage");
    setAttribute(Qt::WA_StyledBackground);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 18, 24, 18);
    layout->setSpacing(12);

    titleLabel = new QLabel("待选车次", this);
    titleLabel->setObjectName("TicketListTitle");
    layout->addWidget(titleLabel);

    tableWidget = new QTableWidget(this);
    tableWidget->setObjectName("TicketListTable");
    tableWidget->setColumnCount(8);
    tableWidget->setHorizontalHeaderLabels(
        {"车次名", "始发站", "终到站", "出发时间", "到达时间", "价格", "余票", "操作"}
    );
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    tableWidget->verticalHeader()->setVisible(false);
    tableWidget->setAlternatingRowColors(true);
    tableWidget->horizontalHeader()->setStretchLastSection(false);
    tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    tableWidget->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    tableWidget->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    tableWidget->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
    tableWidget->horizontalHeader()->setSectionResizeMode(7, QHeaderView::Stretch);

    connect(tableWidget, &QTableWidget::cellClicked,
            this, &TicketListWidget::onCellClicked);

    layout->addWidget(tableWidget, 1);

    setStyleSheet(R"(
        #TicketListPage {
            background-color: #eef3fb;
        }

        QLabel#TicketListTitle {
            color: #1e3a5f;
            font-size: 22px;
            font-weight: 700;
        }

        QTableWidget#TicketListTable {
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

        QTableWidget#TicketListTable QPushButton {
            min-width: 72px;
            border: none;
            border-radius: 6px;
            padding: 5px 10px;
            color: #ffffff;
            background-color: #3d80de;
        }

        QTableWidget#TicketListTable QPushButton:hover {
            background-color: #2f6fc6;
        }
    )");
}

void TicketListWidget::clearTickets() {
    tableWidget->setRowCount(0);
}

void TicketListWidget::setTickets(const QVector<TicketListItem> &tickets) {
    clearTickets();
    tableWidget->setRowCount(tickets.size());

    for (int row = 0; row < tickets.size(); ++row) {
        const TicketListItem &ticket = tickets[row];

        QTableWidgetItem *trainNameItem = new QTableWidgetItem(ticket.trainName);
        QFont trainFont = trainNameItem->font();
        trainFont.setUnderline(true);
        trainNameItem->setFont(trainFont);
        trainNameItem->setForeground(QColor("#1d4ed8"));

        tableWidget->setItem(row, 0, trainNameItem);
        tableWidget->setItem(row, 1, new QTableWidgetItem(ticket.startStation));
        tableWidget->setItem(row, 2, new QTableWidgetItem(ticket.endStation));
        tableWidget->setItem(row, 3, new QTableWidgetItem(ticket.departureTime));
        tableWidget->setItem(row, 4, new QTableWidgetItem(ticket.arrivalTime));
        tableWidget->setItem(row, 5, new QTableWidgetItem(QString::number(ticket.price)));
        tableWidget->setItem(row, 6, new QTableWidgetItem(QString::number(ticket.remain)));

        QPushButton *buyButton = new QPushButton("购票", tableWidget);
        connect(buyButton, &QPushButton::clicked, this, [this, trainName = ticket.trainName]() {
            onPurchaseButtonClicked(trainName);
        });
        tableWidget->setCellWidget(row, 7, buyButton);
    }
}

void TicketListWidget::onCellClicked(int row, int column) {
    if (column != 0) {
        return;
    }

    QTableWidgetItem *item = tableWidget->item(row, column);
    if (item == nullptr) {
        return;
    }

    emit trainNameClicked(item->text());
}

void TicketListWidget::onPurchaseButtonClicked(const QString &trainName) {
    emit purchaseRequested(trainName);
}

} // namespace sjtu::client
