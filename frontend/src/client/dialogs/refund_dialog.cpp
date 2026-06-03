#include "client/dialogs/refund_dialog.hpp"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace sjtu::client {

RefundDialog::RefundDialog(const RefundInfo &info, QWidget *parent)
    : QDialog(parent), orderIndex_(info.orderIndex) {
    setWindowTitle(QString::fromUtf8("确认退票"));
    setModal(true);
    setMinimumWidth(380);

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(16, 16, 16, 16);
    root->setSpacing(12);

    QLabel *title = new QLabel(QString::fromUtf8("确认退票操作"), this);
    title->setObjectName("DialogTitle");
    root->addWidget(title);

    QFormLayout *form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight);
    form->setHorizontalSpacing(10);
    form->setVerticalSpacing(8);

    form->addRow(QString::fromUtf8("订单序号"), new QLabel(QString::number(info.orderIndex), this));
    form->addRow(QString::fromUtf8("车次"), new QLabel(info.trainName, this));
    form->addRow(QString::fromUtf8("出发站"), new QLabel(info.fromStation, this));
    form->addRow(QString::fromUtf8("到达站"), new QLabel(info.toStation, this));
    form->addRow(QString::fromUtf8("出发时间"), new QLabel(info.departureTime, this));
    form->addRow(QString::fromUtf8("单价"), new QLabel(QString::fromUtf8("¥") + QString::number(info.price), this));
    form->addRow(QString::fromUtf8("张数"), new QLabel(QString::number(info.count), this));

    QLabel *statusLabel = new QLabel(info.statusText, this);
    statusLabel->setStyleSheet("font-weight: 600; color: #dc2626;");
    form->addRow(QString::fromUtf8("状态"), statusLabel);

    root->addLayout(form);

    QLabel *warning = new QLabel(QString::fromUtf8("确定要退掉这张车票吗？此操作不可撤销。"), this);
    warning->setWordWrap(true);
    warning->setStyleSheet("color: #dc2626; font-size: 12px;");
    root->addWidget(warning);

    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch(1);
    QPushButton *cancel = new QPushButton(QString::fromUtf8("取消"), this);
    QPushButton *confirm = new QPushButton(QString::fromUtf8("确认退票"), this);
    confirm->setDefault(true);
    confirm->setStyleSheet("background-color: #dc2626; color: #ffffff; border: none;");
    btnRow->addWidget(cancel);
    btnRow->addWidget(confirm);
    root->addLayout(btnRow);

    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(confirm, &QPushButton::clicked, this, &QDialog::accept);

    setStyleSheet(R"(
        QDialog { background-color: #ffffff; }
        QLabel#DialogTitle { font-size: 16px; font-weight: 600; color: #1f2d3d; }
        QLabel { color: #334155; font-size: 13px; }
        QPushButton { padding: 6px 14px; border-radius: 6px; border: 1px solid #cbd5e1; background-color: #f8fafc; }
    )");
}

int RefundDialog::orderIndex() const {
    return orderIndex_;
}

} // namespace sjtu::client
