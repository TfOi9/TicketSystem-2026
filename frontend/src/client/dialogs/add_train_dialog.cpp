#include "client/dialogs/add_train_dialog.hpp"

#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QSpinBox>
#include <QVBoxLayout>

namespace sjtu::client {

AddTrainDialog::AddTrainDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle(QString::fromUtf8("添加列车"));
    setModal(true);
    setMinimumWidth(520);
    setMinimumHeight(600);
    resize(540, 640);

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget *inner = new QWidget(scrollArea);
    QVBoxLayout *innerLayout = new QVBoxLayout(inner);
    innerLayout->setContentsMargins(16, 16, 16, 16);
    innerLayout->setSpacing(12);

    QLabel *title = new QLabel(QString::fromUtf8("添加新列车"), inner);
    title->setObjectName("DialogTitle");
    innerLayout->addWidget(title);

    QFormLayout *form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight);
    form->setHorizontalSpacing(10);
    form->setVerticalSpacing(10);

    trainIdEdit = new QLineEdit(inner);
    trainIdEdit->setPlaceholderText(QString::fromUtf8("例如：G101"));
    form->addRow(QString::fromUtf8("列车编号 (-i)"), trainIdEdit);

    stationCountSpin = new QSpinBox(inner);
    stationCountSpin->setRange(2, 100);
    stationCountSpin->setValue(3);
    stationCountSpin->setSuffix(QString::fromUtf8(" 站"));
    stationCountSpin->setToolTip(QString::fromUtf8("2-100 站"));
    form->addRow(QString::fromUtf8("站点数量 (-n)"), stationCountSpin);

    seatNumSpin = new QSpinBox(inner);
    seatNumSpin->setRange(1, 100000);
    seatNumSpin->setValue(1000);
    seatNumSpin->setSuffix(QString::fromUtf8(" 座"));
    form->addRow(QString::fromUtf8("座位总数 (-m)"), seatNumSpin);

    stationsEdit = new QLineEdit(inner);
    stationsEdit->setPlaceholderText(QString::fromUtf8("上海虹桥|南京南|北京南"));
    stationsEdit->setToolTip(QString::fromUtf8("用 | 分隔，数量与站点数量一致"));
    form->addRow(QString::fromUtf8("站点列表 (-s)"), stationsEdit);

    pricesEdit = new QLineEdit(inner);
    pricesEdit->setPlaceholderText(QString::fromUtf8("114|514 (共 n-1 段)"));
    pricesEdit->setToolTip(QString::fromUtf8("用 | 分隔，数量 = 站点数-1"));
    form->addRow(QString::fromUtf8("各段票价 (-p)"), pricesEdit);

    startTimeEdit = new QLineEdit(inner);
    startTimeEdit->setPlaceholderText("08:00");
    startTimeEdit->setToolTip(QString::fromUtf8("格式 hh:mm"));
    form->addRow(QString::fromUtf8("始发时间 (-x)"), startTimeEdit);

    travelTimesEdit = new QLineEdit(inner);
    travelTimesEdit->setPlaceholderText(QString::fromUtf8("600|600 (共 n-1 段)"));
    travelTimesEdit->setToolTip(QString::fromUtf8("用 | 分隔，数量 = 站点数-1"));
    form->addRow(QString::fromUtf8("行驶时间分 (-t)"), travelTimesEdit);

    stopoverTimesEdit = new QLineEdit(inner);
    stopoverTimesEdit->setPlaceholderText(QString::fromUtf8("5 (共 n-2 段，2站时填 _)"));
    stopoverTimesEdit->setToolTip(QString::fromUtf8("用 | 分隔，数量 = 站点数-2，仅2站时填 _"));
    form->addRow(QString::fromUtf8("停靠时间分 (-o)"), stopoverTimesEdit);

    saleDateBeginEdit = new QDateEdit(inner);
    saleDateBeginEdit->setDisplayFormat("MM-dd");
    saleDateBeginEdit->setDate(QDate(2026, 6, 1));
    saleDateBeginEdit->setMinimumDate(QDate(2026, 6, 1));
    saleDateBeginEdit->setMaximumDate(QDate(2026, 8, 31));
    saleDateBeginEdit->setCalendarPopup(true);
    form->addRow(QString::fromUtf8("发售起始 (-d begin)"), saleDateBeginEdit);

    saleDateEndEdit = new QDateEdit(inner);
    saleDateEndEdit->setDisplayFormat("MM-dd");
    saleDateEndEdit->setDate(QDate(2026, 8, 31));
    saleDateEndEdit->setMinimumDate(QDate(2026, 6, 1));
    saleDateEndEdit->setMaximumDate(QDate(2026, 8, 31));
    saleDateEndEdit->setCalendarPopup(true);
    form->addRow(QString::fromUtf8("发售截止 (-d end)"), saleDateEndEdit);

    typeCombo = new QComboBox(inner);
    typeCombo->addItem(QString::fromUtf8("G (高铁)"), QString("G"));
    typeCombo->addItem(QString::fromUtf8("D (动车)"), QString("D"));
    typeCombo->addItem(QString::fromUtf8("T (特快)"), QString("T"));
    typeCombo->addItem(QString::fromUtf8("K (快速)"), QString("K"));
    form->addRow(QString::fromUtf8("列车类型 (-y)"), typeCombo);

    innerLayout->addLayout(form);

    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch(1);
    QPushButton *cancel = new QPushButton(QString::fromUtf8("取消"), inner);
    QPushButton *confirm = new QPushButton(QString::fromUtf8("添加"), inner);
    confirm->setDefault(true);
    btnRow->addWidget(cancel);
    btnRow->addWidget(confirm);
    innerLayout->addLayout(btnRow);

    scrollArea->setWidget(inner);
    root->addWidget(scrollArea);

    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(confirm, &QPushButton::clicked, this, [this]() {
        const QString trainId = trainIdEdit->text().trimmed();
        const int stationCount = stationCountSpin->value();
        const QString stations = stationsEdit->text().trimmed();
        const QString prices = pricesEdit->text().trimmed();
        const QString startTime = startTimeEdit->text().trimmed();
        const QString travelTimes = travelTimesEdit->text().trimmed();
        const QString stopoverTimes = stopoverTimesEdit->text().trimmed();
        const QString type = typeCombo->currentData().toString();

        if (trainId.isEmpty() || stations.isEmpty() || prices.isEmpty()
            || startTime.isEmpty() || travelTimes.isEmpty()
            || stopoverTimes.isEmpty() || type.isEmpty()) {
            QMessageBox::warning(this, QString::fromUtf8("表单不完整"),
                QString::fromUtf8("请填写所有必填字段。"));
            return;
        }

        const QStringList stationList = stations.split('|', Qt::SkipEmptyParts);
        if (stationList.size() != stationCount) {
            QMessageBox::warning(this, QString::fromUtf8("站点数不匹配"),
                QString::fromUtf8("站点列表中的站点数 (%1) 与站点数量 (%2) 不一致。")
                    .arg(stationList.size()).arg(stationCount));
            return;
        }

        const QStringList priceList = prices.split('|', Qt::SkipEmptyParts);
        if (priceList.size() != stationCount - 1) {
            QMessageBox::warning(this, QString::fromUtf8("票价段数不匹配"),
                QString::fromUtf8("票价应有 %1 段 (当前 %2 段)。")
                    .arg(stationCount - 1).arg(priceList.size()));
            return;
        }

        const QStringList travelList = travelTimes.split('|', Qt::SkipEmptyParts);
        if (travelList.size() != stationCount - 1) {
            QMessageBox::warning(this, QString::fromUtf8("行驶时间不匹配"),
                QString::fromUtf8("行驶时间应有 %1 段 (当前 %2 段)。")
                    .arg(stationCount - 1).arg(travelList.size()));
            return;
        }

        if (stationCount == 2) {
            if (stopoverTimes != "_") {
                QMessageBox::warning(this, QString::fromUtf8("停靠时间错误"),
                    QString::fromUtf8("只有 2 站时，停靠时间应填入 _ 。"));
                return;
            }
        } else {
            const QStringList stopList = stopoverTimes.split('|', Qt::SkipEmptyParts);
            if (stopList.size() != stationCount - 2) {
                QMessageBox::warning(this, QString::fromUtf8("停靠时间不匹配"),
                    QString::fromUtf8("停靠时间应有 %1 段 (当前 %2 段)。")
                        .arg(stationCount - 2).arg(stopList.size()));
                return;
            }
        }

        QRegularExpression timeRe("^\\d{1,2}:\\d{2}$");
        if (!timeRe.match(startTime).hasMatch()) {
            QMessageBox::warning(this, QString::fromUtf8("时间格式错误"),
                QString::fromUtf8("始发时间格式应为 hh:mm。"));
            return;
        }

        if (saleDateBeginEdit->date() > saleDateEndEdit->date()) {
            QMessageBox::warning(this, QString::fromUtf8("日期错误"),
                QString::fromUtf8("发售起始日期不能晚于截止日期。"));
            return;
        }

        accept();
    });

    setStyleSheet(R"(
        QDialog { background-color: #ffffff; }
        QLabel#DialogTitle { font-size: 16px; font-weight: 600; color: #1f2d3d; }
        QLabel { color: #334155; font-size: 13px; }
        QLineEdit, QSpinBox, QComboBox, QDateEdit {
            border: 1px solid #cbd5e1; border-radius: 6px; padding: 6px 8px; min-height: 20px;
        }
        QLineEdit:focus, QSpinBox:focus, QComboBox:focus, QDateEdit:focus { border-color: #3d80de; }
        QPushButton { padding: 6px 14px; border-radius: 6px; border: 1px solid #cbd5e1; background-color: #f8fafc; }
    )");
}

QString AddTrainDialog::commandLine() const {
    const QString trainId = trainIdEdit->text().trimmed();
    const int stationCount = stationCountSpin->value();
    const int seatNum = seatNumSpin->value();
    QString stations = stationsEdit->text().trimmed();
    QString prices = pricesEdit->text().trimmed();
    const QString startTime = startTimeEdit->text().trimmed();
    QString travelTimes = travelTimesEdit->text().trimmed();
    QString stopoverTimes = stopoverTimesEdit->text().trimmed();
    const QString saleBegin = saleDateBeginEdit->date().toString("MM-dd");
    const QString saleEnd = saleDateEndEdit->date().toString("MM-dd");
    const QString type = typeCombo->currentData().toString();

    if (trainId.isEmpty() || stations.isEmpty() || prices.isEmpty()
        || startTime.isEmpty() || travelTimes.isEmpty()
        || stopoverTimes.isEmpty() || type.isEmpty()) {
        return {};
    }

    stations.replace(" ", "_");
    prices.replace(" ", "_");
    travelTimes.replace(" ", "_");
    stopoverTimes.replace(" ", "_");

    const QString saleDate = saleBegin + "|" + saleEnd;

    QString cmd = "add_train -i " + trainId
                + " -n " + QString::number(stationCount)
                + " -m " + QString::number(seatNum)
                + " -s " + stations
                + " -p " + prices
                + " -x " + startTime
                + " -t " + travelTimes
                + " -o " + stopoverTimes
                + " -d " + saleDate
                + " -y " + type;
    return cmd;
}

} // namespace sjtu::client
