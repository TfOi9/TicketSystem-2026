#include "client/widgets/ticket_list_widget.hpp"

#include <QHeaderView>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace {

class SortableTableItem : public QTableWidgetItem {
public:
    SortableTableItem(const QString &text, qint64 sortKey)
        : QTableWidgetItem(text), sortKey_(sortKey) {}

    bool operator<(const QTableWidgetItem &other) const override {
        const auto *otherItem = dynamic_cast<const SortableTableItem *>(&other);
        if (otherItem != nullptr) {
            return sortKey_ < otherItem->sortKey_;
        }
        return QTableWidgetItem::operator<(other);
    }

private:
    qint64 sortKey_;
};

QString formatDuration(int minutes) {
    if (minutes <= 0) {
        return "0m";
    }
    const int hours = minutes / 60;
    const int mins = minutes % 60;
    if (hours == 0) {
        return QString::number(mins) + "m";
    }
    if (mins == 0) {
        return QString::number(hours) + "h";
    }
    return QString::number(hours) + "h " + QString::number(mins) + "m";
}

void centerItem(QTableWidgetItem *item) {
    if (item != nullptr) {
        item->setTextAlignment(Qt::AlignCenter);
    }
}

} // namespace

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
    tableWidget->setColumnCount(9);
    tableWidget->setHorizontalHeaderLabels(
        {"车次名", "出发站", "到达站", "出发时间", "到达时间", "历时", "价格", "余票", "操作"}
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

    tableWidget->setColumnWidth(0, 120);
    tableWidget->setColumnWidth(1, 115);
    tableWidget->setColumnWidth(2, 115);
    tableWidget->setColumnWidth(3, 155);
    tableWidget->setColumnWidth(4, 155);
    tableWidget->setColumnWidth(5, 90);
    tableWidget->setColumnWidth(6, 80);
    tableWidget->setColumnWidth(7, 80);
    tableWidget->setColumnWidth(8, 92);

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
    const bool sortingEnabled = tableWidget->isSortingEnabled();
    tableWidget->setSortingEnabled(false);

    clearTickets();
    tableWidget->setRowCount(tickets.size());

    for (int row = 0; row < tickets.size(); ++row) {
        const TicketListItem &ticket = tickets[row];

        QTableWidgetItem *trainNameItem = new QTableWidgetItem(ticket.trainName);
        QFont trainFont = trainNameItem->font();
        trainFont.setUnderline(true);
        trainNameItem->setFont(trainFont);
        trainNameItem->setForeground(QColor("#1d4ed8"));
        centerItem(trainNameItem);

        auto *depItem = new SortableTableItem(ticket.departureTime, ticket.departureSortKey);
        auto *arrItem = new SortableTableItem(ticket.arrivalTime, ticket.arrivalSortKey);
        auto *durationItem = new SortableTableItem(
            formatDuration(ticket.durationMinutes),
            ticket.durationMinutes
        );
        auto *priceItem = new SortableTableItem(QString::number(ticket.price), ticket.price);

        centerItem(depItem);
        centerItem(arrItem);
        centerItem(durationItem);
        centerItem(priceItem);

        tableWidget->setItem(row, 0, trainNameItem);
        auto *startItem = new QTableWidgetItem(ticket.startStation);
        auto *endItem = new QTableWidgetItem(ticket.endStation);
        auto *remainItem = new QTableWidgetItem(QString::number(ticket.remain));
        centerItem(startItem);
        centerItem(endItem);
        centerItem(remainItem);

        tableWidget->setItem(row, 1, startItem);
        tableWidget->setItem(row, 2, endItem);
        tableWidget->setItem(row, 3, depItem);
        tableWidget->setItem(row, 4, arrItem);
        tableWidget->setItem(row, 5, durationItem);
        tableWidget->setItem(row, 6, priceItem);
        tableWidget->setItem(row, 7, remainItem);

        QPushButton *buyButton = new QPushButton("购票", this);
        connect(buyButton, &QPushButton::clicked, this, [this, trainName = ticket.trainName]() {
            onPurchaseButtonClicked(trainName);
        });

        QWidget *buttonHost = new QWidget(tableWidget);
        QHBoxLayout *buttonLayout = new QHBoxLayout(buttonHost);
        buttonLayout->setContentsMargins(0, 0, 0, 0);
        buttonLayout->addWidget(buyButton, 0, Qt::AlignCenter);
        tableWidget->setCellWidget(row, 8, buttonHost);
    }

    tableWidget->setSortingEnabled(sortingEnabled);
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
