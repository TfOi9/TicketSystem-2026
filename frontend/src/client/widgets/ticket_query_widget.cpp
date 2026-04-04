#include "client/widgets/ticket_query_widget.hpp"

#include <QDate>
#include <QDateEdit>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace sjtu::client {

TicketQueryWidget::TicketQueryWidget(QWidget *parent) : QWidget(parent) {
    setObjectName("TicketQueryWidget");
    setAttribute(Qt::WA_StyledBackground);

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(18, 18, 18, 18);
    root->setSpacing(14);

    QFormLayout *form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight);
    form->setHorizontalSpacing(10);
    form->setVerticalSpacing(10);

    fromStationEdit = new QLineEdit(this);
    fromStationEdit->setPlaceholderText("例如：上海虹桥");
    toStationEdit = new QLineEdit(this);
    toStationEdit->setPlaceholderText("例如：北京南");

    dateEdit = new QDateEdit(this);
    dateEdit->setDisplayFormat("MM-dd");
    dateEdit->setDate(QDate(2025, 6, 1));
    dateEdit->setMinimumDate(QDate(2025, 6, 1));
    dateEdit->setMaximumDate(QDate(2025, 8, 31));
    dateEdit->setCalendarPopup(true);

    form->addRow("出发站", fromStationEdit);
    form->addRow("到达站", toStationEdit);
    form->addRow("出发日", dateEdit);
    root->addLayout(form);

    QHBoxLayout *actions = new QHBoxLayout();
    actions->setSpacing(10);
    swapButton = new QPushButton("换向", this);
    queryButton = new QPushButton("查询车票", this);
    queryButton->setDefault(true);
    actions->addWidget(swapButton);
    actions->addWidget(queryButton);
    root->addLayout(actions);

    connect(swapButton, &QPushButton::clicked, this, &TicketQueryWidget::onSwapClicked);
    connect(queryButton, &QPushButton::clicked, this, &TicketQueryWidget::onQueryClicked);

    setStyleSheet(R"(
        #TicketQueryWidget {
            background-color: rgba(255, 255, 255, 0.95);
            border-radius: 12px;
        }

        #TicketQueryWidget QLineEdit,
        #TicketQueryWidget QDateEdit {
            border: 1px solid #cbd5e1;
            border-radius: 6px;
            padding: 6px 8px;
            min-height: 20px;
            background-color: #ffffff;
        }

        #TicketQueryWidget QLineEdit:focus,
        #TicketQueryWidget QDateEdit:focus {
            border-color: #3d80de;
        }

        #TicketQueryWidget QPushButton {
            border: none;
            border-radius: 6px;
            padding: 8px 14px;
            color: #ffffff;
            background-color: #3d80de;
        }

        #TicketQueryWidget QPushButton:hover {
            background-color: #2f6fc6;
        }
    )");
}

void TicketQueryWidget::onSwapClicked() {
    const QString from = fromStationEdit->text();
    fromStationEdit->setText(toStationEdit->text());
    toStationEdit->setText(from);
}

void TicketQueryWidget::onQueryClicked() {
    const QString from = fromStationEdit->text().trimmed();
    const QString to = toStationEdit->text().trimmed();
    const QString date = dateEdit->date().toString("MM-dd");
    emit queryTicketRequested(from, to, date);
}

} // namespace sjtu::client
