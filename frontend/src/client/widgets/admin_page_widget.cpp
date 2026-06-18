#include "client/widgets/admin_page_widget.hpp"
#include "client/dialogs/add_train_dialog.hpp"

#include <QDate>
#include <QDateEdit>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QComboBox>
#include <QVBoxLayout>

namespace sjtu::client {

AdminPageWidget::AdminPageWidget(QWidget *parent) : QWidget(parent) {
    setObjectName("AdminPage");
    setAttribute(Qt::WA_StyledBackground);

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setObjectName("AdminScrollArea");

    QWidget *inner = new QWidget(scrollArea);
    QVBoxLayout *layout = new QVBoxLayout(inner);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(20);

    QGroupBox *trainGroup = new QGroupBox(QString::fromUtf8("列车管理"), inner);
    trainGroup->setObjectName("AdminGroup");
    QVBoxLayout *trainLayout = new QVBoxLayout(trainGroup);
    trainLayout->setContentsMargins(16, 16, 16, 16);
    trainLayout->setSpacing(12);

    QHBoxLayout *trainIdRow = new QHBoxLayout();
    QLabel *trainIdLabel = new QLabel(QString::fromUtf8("列车编号"), trainGroup);
    trainIdLabel->setObjectName("AdminLabel");
    trainIdEdit = new QLineEdit(trainGroup);
    trainIdEdit->setPlaceholderText(QString::fromUtf8("例如：G101"));
    trainIdEdit->setMinimumWidth(100);

    QLabel *trainDateLabel = new QLabel(QString::fromUtf8("日期"), trainGroup);
    trainDateLabel->setObjectName("AdminLabel");
    trainDateEdit = new QDateEdit(trainGroup);
    trainDateEdit->setDisplayFormat("MM-dd");
    trainDateEdit->setDate(QDate(2026, 6, 1));
    trainDateEdit->setMinimumDate(QDate(2026, 6, 1));
    trainDateEdit->setMaximumDate(QDate(2026, 8, 31));
    trainDateEdit->setCalendarPopup(true);
    trainDateEdit->setFixedWidth(100);

    queryTrainButton = new QPushButton(QString::fromUtf8("查询列车"), trainGroup);
    queryTrainButton->setObjectName("AdminBtnPrimary");
    releaseTrainButton = new QPushButton(QString::fromUtf8("发布列车"), trainGroup);
    releaseTrainButton->setObjectName("AdminBtnSuccess");
    deleteTrainButton = new QPushButton(QString::fromUtf8("删除列车"), trainGroup);
    deleteTrainButton->setObjectName("AdminBtnDanger");
    addTrainButton = new QPushButton(QString::fromUtf8("添加列车"), trainGroup);
    addTrainButton->setObjectName("AdminBtnPrimary");
    importTrainButton = new QPushButton(QString::fromUtf8("导入火车"), trainGroup);
    importTrainButton->setObjectName("AdminBtnPrimary");
    batchReleaseButton = new QPushButton(QString::fromUtf8("批量发布"), trainGroup);
    batchReleaseButton->setObjectName("AdminBtnSuccess");

    trainIdRow->addWidget(trainIdLabel);
    trainIdRow->addWidget(trainIdEdit);
    trainIdRow->addWidget(trainDateLabel);
    trainIdRow->addWidget(trainDateEdit);
    trainIdRow->addWidget(queryTrainButton);
    trainIdRow->addWidget(releaseTrainButton);
    trainIdRow->addWidget(deleteTrainButton);
    trainIdRow->addWidget(addTrainButton);
    trainIdRow->addWidget(importTrainButton);
    trainIdRow->addWidget(batchReleaseButton);
    trainIdRow->addStretch(1);
    trainLayout->addLayout(trainIdRow);

    trainResultLabel = new QLabel(trainGroup);
    trainResultLabel->setObjectName("AdminResultLabel");
    trainResultLabel->setWordWrap(true);
    trainResultLabel->setTextFormat(Qt::RichText);
    trainResultLabel->setMinimumHeight(60);
    trainLayout->addWidget(trainResultLabel);
    layout->addWidget(trainGroup);

    QGroupBox *userGroup = new QGroupBox(QString::fromUtf8("用户管理"), inner);
    userGroup->setObjectName("AdminGroup");
    QVBoxLayout *userLayout = new QVBoxLayout(userGroup);
    userLayout->setContentsMargins(16, 16, 16, 16);
    userLayout->setSpacing(12);

    QHBoxLayout *userRow = new QHBoxLayout();
    QLabel *userLabel = new QLabel(QString::fromUtf8("用户名"), userGroup);
    userLabel->setObjectName("AdminLabel");
    usernameEdit = new QLineEdit(userGroup);
    usernameEdit->setPlaceholderText(QString::fromUtf8("查询或创建用户"));
    usernameEdit->setMinimumWidth(160);

    queryUserButton = new QPushButton(QString::fromUtf8("查询用户"), userGroup);
    queryUserButton->setObjectName("AdminBtnPrimary");
    addUserButton = new QPushButton(QString::fromUtf8("添加用户"), userGroup);
    addUserButton->setObjectName("AdminBtnPrimary");

    userRow->addWidget(userLabel);
    userRow->addWidget(usernameEdit);
    userRow->addWidget(queryUserButton);
    userRow->addWidget(addUserButton);
    userRow->addStretch(1);
    userLayout->addLayout(userRow);

    userResultLabel = new QLabel(userGroup);
    userResultLabel->setObjectName("AdminResultLabel");
    userResultLabel->setWordWrap(true);
    userResultLabel->setTextFormat(Qt::RichText);
    userResultLabel->setMinimumHeight(60);
    userLayout->addWidget(userResultLabel);
    layout->addWidget(userGroup);

    layout->addStretch(1);

    scrollArea->setWidget(inner);

    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->addWidget(scrollArea);

    connect(queryTrainButton, &QPushButton::clicked, this, &AdminPageWidget::onQueryTrainClicked);
    connect(releaseTrainButton, &QPushButton::clicked, this, &AdminPageWidget::onReleaseTrainClicked);
    connect(deleteTrainButton, &QPushButton::clicked, this, &AdminPageWidget::onDeleteTrainClicked);
    connect(addTrainButton, &QPushButton::clicked, this, &AdminPageWidget::onAddTrainClicked);
    connect(importTrainButton, &QPushButton::clicked, this, &AdminPageWidget::onImportTrainsClicked);
    connect(batchReleaseButton, &QPushButton::clicked, this, &AdminPageWidget::onBatchReleaseClicked);
    connect(queryUserButton, &QPushButton::clicked, this, &AdminPageWidget::onQueryProfileClicked);
    connect(addUserButton, &QPushButton::clicked, this, &AdminPageWidget::onAddUserClicked);

    setStyleSheet(R"(
        #AdminPage {
            background-color: #eef3fb;
        }

        #AdminScrollArea {
            background-color: transparent;
        }

        QGroupBox#AdminGroup {
            border: 1px solid #d9e3f0;
            border-radius: 12px;
            background-color: #ffffff;
            margin-top: 12px;
            padding-top: 16px;
            font-size: 15px;
            font-weight: 700;
            color: #1e3a5f;
        }

        QGroupBox#AdminGroup::title {
            subcontrol-origin: margin;
            left: 16px;
            padding: 0 8px;
        }

        QLabel#AdminLabel {
            color: #334155;
            font-size: 13px;
            font-weight: 600;
        }

        QLabel#AdminResultLabel {
            background-color: #f8fafc;
            border: 1px solid #e2e8f0;
            border-radius: 8px;
            padding: 10px 14px;
            color: #1e3a5f;
            font-size: 13px;
            min-height: 30px;
        }

        #AdminPage QLineEdit {
            border: 1px solid #cbd5e1;
            border-radius: 6px;
            padding: 6px 8px;
            min-height: 20px;
            background-color: #ffffff;
        }

        #AdminPage QLineEdit:focus {
            border-color: #3d80de;
        }

        #AdminPage QDateEdit {
            border: 1px solid #cbd5e1;
            border-radius: 6px;
            padding: 4px 6px;
            min-height: 20px;
            background-color: #ffffff;
        }

        QPushButton#AdminBtnPrimary {
            border: none;
            border-radius: 6px;
            padding: 6px 14px;
            color: #ffffff;
            background-color: #3d80de;
        }

        QPushButton#AdminBtnPrimary:hover {
            background-color: #2f6fc6;
        }

        QPushButton#AdminBtnSuccess {
            border: none;
            border-radius: 6px;
            padding: 6px 14px;
            color: #ffffff;
            background-color: #16a34a;
        }

        QPushButton#AdminBtnSuccess:hover {
            background-color: #15803d;
        }

        QPushButton#AdminBtnDanger {
            border: none;
            border-radius: 6px;
            padding: 6px 14px;
            color: #ffffff;
            background-color: #dc2626;
        }

        QPushButton#AdminBtnDanger:hover {
            background-color: #b91c1c;
        }
    )");
}

void AdminPageWidget::showTrainResult(const QString &info) {
    trainResultLabel->setText(info);
}

void AdminPageWidget::showUserResult(const QString &info) {
    userResultLabel->setText(info);
}

void AdminPageWidget::clearResults() {
    trainResultLabel->clear();
    userResultLabel->clear();
}

void AdminPageWidget::onQueryTrainClicked() {
    const QString trainId = trainIdEdit->text().trimmed();
    if (trainId.isEmpty()) {
        showTrainResult(QString::fromUtf8("<span style='color:#dc2626;'>请输入列车编号。</span>"));
        return;
    }
    const QString date = trainDateEdit->date().toString("MM-dd");
    emit queryTrainRequested(trainId, date);
}

void AdminPageWidget::onReleaseTrainClicked() {
    const QString trainId = trainIdEdit->text().trimmed();
    if (trainId.isEmpty()) {
        showTrainResult(QString::fromUtf8("<span style='color:#dc2626;'>请输入列车编号。</span>"));
        return;
    }
    emit releaseTrainRequested(trainId);
}

void AdminPageWidget::onDeleteTrainClicked() {
    const QString trainId = trainIdEdit->text().trimmed();
    if (trainId.isEmpty()) {
        showTrainResult(QString::fromUtf8("<span style='color:#dc2626;'>请输入列车编号。</span>"));
        return;
    }
    emit deleteTrainRequested(trainId);
}

void AdminPageWidget::onAddTrainClicked() {
    AddTrainDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        const QString cmd = dialog.commandLine();
        if (cmd.isEmpty()) {
            showTrainResult(QString::fromUtf8("<span style='color:#dc2626;'>表单不完整，请填写所有字段。</span>"));
            return;
        }
        emit addTrainRequested(cmd);
    }
}

void AdminPageWidget::onImportTrainsClicked() {
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        QString::fromUtf8("选择火车信息文件"),
        QString(),
        QString::fromUtf8("文本文件 (*.txt);;所有文件 (*)")
    );
    if (filePath.isEmpty()) {
        return;
    }
    emit importTrainsRequested(filePath);
}

void AdminPageWidget::onBatchReleaseClicked() {
    emit batchReleaseRequested();
}

void AdminPageWidget::onQueryProfileClicked() {
    const QString username = usernameEdit->text().trimmed();
    if (username.isEmpty()) {
        showUserResult(QString::fromUtf8("<span style='color:#dc2626;'>请输入用户名。</span>"));
        return;
    }
    emit queryProfileRequested(username);
}

void AdminPageWidget::onAddUserClicked() {
    const QString username = usernameEdit->text().trimmed();
    if (username.isEmpty()) {
        showUserResult(QString::fromUtf8("<span style='color:#dc2626;'>请输入用户名。</span>"));
        return;
    }

    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle(QString::fromUtf8("添加用户"));
    dialog->setModal(true);
    dialog->setMinimumWidth(380);

    QVBoxLayout *root = new QVBoxLayout(dialog);
    root->setContentsMargins(16, 16, 16, 16);
    root->setSpacing(12);

    QLabel *title = new QLabel(QString::fromUtf8("创建新用户"), dialog);
    title->setObjectName("DialogTitle");

    QFormLayout *form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight);
    form->setHorizontalSpacing(10);
    form->setVerticalSpacing(10);

    QLineEdit *pwEdit = new QLineEdit(dialog);
    pwEdit->setPlaceholderText(QString::fromUtf8("密码"));
    pwEdit->setEchoMode(QLineEdit::Password);

    QLineEdit *nmEdit = new QLineEdit(dialog);
    nmEdit->setPlaceholderText(QString::fromUtf8("姓名"));

    QLineEdit *emEdit = new QLineEdit(dialog);
    emEdit->setPlaceholderText(QString::fromUtf8("邮箱"));

    QComboBox *privCombo = new QComboBox(dialog);
    privCombo->addItem("0", 0);
    privCombo->addItem("1", 1);
    privCombo->addItem("2", 2);
    privCombo->addItem("10", 10);
    privCombo->setCurrentIndex(1);

    form->addRow(QString::fromUtf8("用户名"), new QLabel(username, dialog));
    form->addRow(QString::fromUtf8("密码"), pwEdit);
    form->addRow(QString::fromUtf8("姓名"), nmEdit);
    form->addRow(QString::fromUtf8("邮箱"), emEdit);
    form->addRow(QString::fromUtf8("权限"), privCombo);

    root->addWidget(title);
    root->addLayout(form);

    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch(1);
    QPushButton *cancel = new QPushButton(QString::fromUtf8("取消"), dialog);
    QPushButton *confirm = new QPushButton(QString::fromUtf8("创建"), dialog);
    confirm->setDefault(true);
    btnRow->addWidget(cancel);
    btnRow->addWidget(confirm);
    root->addLayout(btnRow);

    connect(cancel, &QPushButton::clicked, dialog, &QDialog::reject);
    connect(confirm, &QPushButton::clicked, dialog, [&]() {
        dialog->accept();
    });

    dialog->setStyleSheet(R"(
        QDialog { background-color: #ffffff; }
        QLabel#DialogTitle { font-size: 16px; font-weight: 600; color: #1f2d3d; }
        QLabel { color: #334155; font-size: 13px; }
        QLineEdit, QComboBox { border: 1px solid #cbd5e1; border-radius: 6px; padding: 6px 8px; min-height: 18px; }
        QLineEdit:focus, QComboBox:focus { border-color: #3d80de; }
        QPushButton { padding: 6px 14px; border-radius: 6px; border: 1px solid #cbd5e1; background-color: #f8fafc; }
    )");

    if (dialog->exec() == QDialog::Accepted) {
        const QString password = pwEdit->text();
        const QString name = nmEdit->text().trimmed();
        const QString email = emEdit->text().trimmed();
        int privilege = privCombo->currentData().toInt();

        if (password.isEmpty() || name.isEmpty() || email.isEmpty()) {
            showUserResult(QString::fromUtf8("<span style='color:#dc2626;'>请填写所有字段。</span>"));
            dialog->deleteLater();
            return;
        }
        emit addUserRequested(username, password, name, email, privilege);
    }
    dialog->deleteLater();
}

} // namespace sjtu::client
