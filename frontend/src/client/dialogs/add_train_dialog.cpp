#include "client/dialogs/add_train_dialog.hpp"

#include <QDate>
#include <QDateEdit>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QVBoxLayout>

namespace sjtu::client {

AddTrainDialog::AddTrainDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle(QString::fromUtf8("添加列车"));
    setModal(true);
    setMinimumWidth(480);

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(16, 16, 16, 16);
    root->setSpacing(12);

    QLabel *title = new QLabel(QString::fromUtf8("添加新列车"), this);
    title->setObjectName("DialogTitle");
    root->addWidget(title);

    QFormLayout *form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight);
    form->setHorizontalSpacing(10);
    form->setVerticalSpacing(8);

    trainIdEdit = new QLineEdit(this);
    trainIdEdit->setPlaceholderText(QString::fromUtf8("例如：G101"));
    form->addRow(QString::fromUtf8("列车编号"), trainIdEdit);

    stationCountSpin = new QSpinBox(this);
    stationCountSpin->setRange(2, 100);
    stationCountSpin->setValue(3);
    stationCountSpin->setSuffix(QString::fromUtf8(" 站"));
    form->addRow(QString::fromUtf8("站点数量"), stationCountSpin);

    stationsEdit = new QLineEdit(this);
    stationsEdit->setPlaceholderText(QString::fromUtf8("上海虹桥|南京南|北京南"));
    form->addRow(QString::fromUtf8("站点列表"), stationsEdit);

    seatsEdit = new QLineEdit(this);
    seatsEdit->setPlaceholderText(QString::fromUtf8("100|80"));
    form->addRow(QString::fromUtf8("各段座位数"), seatsEdit);

    pricesEdit = new QLineEdit(this);
    pricesEdit->setPlaceholderText(QString::fromUtf8("0|50|100"));
    form->addRow(QString::fromUtf8("各段票价"), pricesEdit);

    startTimeEdit = new QLineEdit(this);
    startTimeEdit->setPlaceholderText("08:00");
    form->addRow(QString::fromUtf8("始发时间"), startTimeEdit);

    travelTimesEdit = new QLineEdit(this);
    travelTimesEdit->setPlaceholderText(QString::fromUtf8("0|120|180"));
    form->addRow(QString::fromUtf8("行驶时间(分)"), travelTimesEdit);

    stopoverTimesEdit = new QLineEdit(this);
    stopoverTimesEdit->setPlaceholderText(QString::fromUtf8("10|5|0"));
    form->addRow(QString::fromUtf8("停靠时间(分)"), stopoverTimesEdit);

    saleDateBeginEdit = new QDateEdit(this);
    saleDateBeginEdit->setDisplayFormat("MM-dd");
    saleDateBeginEdit->setDate(QDate(2026, 6, 1));
    saleDateBeginEdit->setMinimumDate(QDate(2026, 6, 1));
    saleDateBeginEdit->setMaximumDate(QDate(2026, 9, 30));
    saleDateBeginEdit->setCalendarPopup(true);
    form->addRow(QString::fromUtf8("发售起始日"), saleDateBeginEdit);

    saleDateEndEdit = new QDateEdit(this);
    saleDateEndEdit->setDisplayFormat("MM-dd");
    saleDateEndEdit->setDate(QDate(2026, 9, 1));
    saleDateEndEdit->setMinimumDate(QDate(2026, 6, 1));
    saleDateEndEdit->setMaximumDate(QDate(2026, 9, 30));
    saleDateEndEdit->setCalendarPopup(true);
    form->addRow(QString::fromUtf8("发售截止日"), saleDateEndEdit);

    typeCombo = new QComboBox(this);
    typeCombo->addItem(QString::fromUtf8("G (高铁)"), QString("G"));
    typeCombo->addItem(QString::fromUtf8("D (动车)"), QString("D"));
    typeCombo->addItem(QString::fromUtf8("T (特快)"), QString("T"));
    typeCombo->addItem(QString::fromUtf8("K (快速)"), QString("K"));
    form->addRow(QString::fromUtf8("列车类型"), typeCombo);

    root->addLayout(form);

    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch(1);
    QPushButton *cancel = new QPushButton(QString::fromUtf8("取消"), this);
    QPushButton *confirm = new QPushButton(QString::fromUtf8("添加"), this);
    confirm->setDefault(true);
    btnRow->addWidget(cancel);
    btnRow->addWidget(confirm);
    root->addLayout(btnRow);

    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(confirm, &QPushButton::clicked, this, &QDialog::accept);

    setStyleSheet(R"(
        QDialog { background-color: #ffffff; }
        QLabel#DialogTitle { font-size: 16px; font-weight: 600; color: #1f2d3d; }
        QLabel { color: #334155; font-size: 13px; }
        QLineEdit, QSpinBox, QComboBox, QDateEdit {
            border: 1px solid #cbd5e1; border-radius: 6px; padding: 6px 8px; min-height: 18px;
        }
        QLineEdit:focus, QSpinBox:focus, QComboBox:focus, QDateEdit:focus { border-color: #3d80de; }
        QPushButton { padding: 6px 14px; border-radius: 6px; border: 1px solid #cbd5e1; background-color: #f8fafc; }
    )");
}

QString AddTrainDialog::commandLine() const {
    const QString trainId = trainIdEdit->text().trimmed();
    const int stationCount = stationCountSpin->value();
    QString stations = stationsEdit->text().trimmed();
    QString seats = seatsEdit->text().trimmed();
    QString prices = pricesEdit->text().trimmed();
    const QString startTime = startTimeEdit->text().trimmed();
    QString travelTimes = travelTimesEdit->text().trimmed();
    QString stopoverTimes = stopoverTimesEdit->text().trimmed();
    const QString saleBegin = saleDateBeginEdit->date().toString("MM-dd");
    const QString saleEnd = saleDateEndEdit->date().toString("MM-dd");
    const QString type = typeCombo->currentData().toString();

    if (trainId.isEmpty() || stations.isEmpty() || seats.isEmpty()
        || prices.isEmpty() || startTime.isEmpty() || travelTimes.isEmpty()
        || stopoverTimes.isEmpty() || type.isEmpty()) {
        return {};
    }

    stations.replace(" ", "_");
    seats.replace(" ", "_");
    prices.replace(" ", "_");
    travelTimes.replace(" ", "_");
    stopoverTimes.replace(" ", "_");

    QString cmd = "add_train -i " + trainId
                + " -n " + QString::number(stationCount)
                + " -m " + stations
                + " -s " + seats
                + " -p " + prices
                + " -x " + startTime
                + " -t " + travelTimes
                + " -o " + stopoverTimes
                + " -d " + saleBegin
                + " -y " + saleEnd
                + " -c " + type;
    return cmd;
}

} // namespace sjtu::client
