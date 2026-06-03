#include "client/dialogs/buy_ticket_dialog.hpp"

#include <QCheckBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace sjtu::client {

BuyTicketDialog::BuyTicketDialog(const TicketInfo &info, QWidget *parent)
    : QDialog(parent), countSpinBox(nullptr), queueCheckBox(nullptr) {
    setWindowTitle(QString::fromUtf8("确认购票"));
    setModal(true);
    setMinimumWidth(400);

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(16, 16, 16, 16);
    root->setSpacing(12);

    QLabel *title = new QLabel(QString::fromUtf8("确认购买车票"), this);
    title->setObjectName("DialogTitle");
    root->addWidget(title);

    QFormLayout *form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight);
    form->setHorizontalSpacing(10);
    form->setVerticalSpacing(8);

    form->addRow(QString::fromUtf8("车次"), new QLabel(info.trainName, this));
    form->addRow(QString::fromUtf8("出发站"), new QLabel(info.fromStation, this));
    form->addRow(QString::fromUtf8("到达站"), new QLabel(info.toStation, this));
    form->addRow(QString::fromUtf8("出发时间"), new QLabel(info.departureTime, this));
    form->addRow(QString::fromUtf8("到达时间"), new QLabel(info.arrivalTime, this));
    form->addRow(QString::fromUtf8("单价"), new QLabel(QString::fromUtf8("¥") + QString::number(info.price), this));

    QLabel *remainLabel = new QLabel(QString::number(info.remain), this);
    if (info.remain > 0) {
        remainLabel->setStyleSheet("color: #16a34a; font-weight: 600;");
    } else {
        remainLabel->setStyleSheet("color: #dc2626; font-weight: 600;");
    }
    form->addRow(QString::fromUtf8("余票"), remainLabel);

    root->addLayout(form);

    QHBoxLayout *countRow = new QHBoxLayout();
    QLabel *countLabel = new QLabel(QString::fromUtf8("购买张数"), this);
    countSpinBox = new QSpinBox(this);
    countSpinBox->setRange(1, 5);
    countSpinBox->setValue(1);
    countRow->addWidget(countLabel);
    countRow->addWidget(countSpinBox, 1);
    root->addLayout(countRow);

    queueCheckBox = new QCheckBox(QString::fromUtf8("无票时加入候补队列"), this);
    root->addWidget(queueCheckBox);

    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch(1);
    QPushButton *cancel = new QPushButton(QString::fromUtf8("取消"), this);
    QPushButton *confirm = new QPushButton(QString::fromUtf8("确认购买"), this);
    confirm->setDefault(true);
    confirm->setStyleSheet("background-color: #3d80de; color: #ffffff; border: none;");
    btnRow->addWidget(cancel);
    btnRow->addWidget(confirm);
    root->addLayout(btnRow);

    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(confirm, &QPushButton::clicked, this, &QDialog::accept);

    setStyleSheet(R"(
        QDialog { background-color: #ffffff; }
        QLabel#DialogTitle { font-size: 16px; font-weight: 600; color: #1f2d3d; }
        QLabel { color: #334155; font-size: 13px; }
        QSpinBox { border: 1px solid #cbd5e1; border-radius: 6px; padding: 6px 8px; min-height: 18px; }
        QCheckBox { color: #334155; font-size: 13px; }
        QPushButton { padding: 6px 14px; border-radius: 6px; border: 1px solid #cbd5e1; background-color: #f8fafc; }
    )");
}

int BuyTicketDialog::ticketCount() const {
    return countSpinBox->value();
}

bool BuyTicketDialog::useQueue() const {
    return queueCheckBox->isChecked();
}

} // namespace sjtu::client
