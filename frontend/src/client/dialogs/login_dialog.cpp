#include "client/dialogs/login_dialog.hpp"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QVBoxLayout>

namespace sjtu::client {

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle(QString::fromUtf8("登录"));
    setModal(true);
    setMinimumWidth(380);

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(16, 16, 16, 16);
    root->setSpacing(12);

    QLabel *title = new QLabel(QString::fromUtf8("登录 TicketSystem"), this);
    title->setObjectName("DialogTitle");
    root->addWidget(title);

    QFormLayout *form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight);
    form->setHorizontalSpacing(10);
    form->setVerticalSpacing(10);

    usernameEdit = new QLineEdit(this);
    usernameEdit->setPlaceholderText(QString::fromUtf8("用户名"));
    usernameEdit->setMaxLength(20);

    passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText(QString::fromUtf8("密码"));
    passwordEdit->setEchoMode(QLineEdit::Password);

    QHBoxLayout *pwdRow = new QHBoxLayout();
    pwdRow->setSpacing(0);
    pwdRow->addWidget(passwordEdit);

    togglePwdBtn = new QPushButton(QString::fromUtf8("👁"), this);
    togglePwdBtn->setFixedSize(28, 28);
    togglePwdBtn->setCheckable(true);
    togglePwdBtn->setFlat(true);
    togglePwdBtn->setCursor(Qt::PointingHandCursor);
    togglePwdBtn->setToolTip(QString::fromUtf8("显示/隐藏密码"));
    pwdRow->addWidget(togglePwdBtn);

    form->addRow(QString::fromUtf8("用户名"), usernameEdit);
    form->addRow(QString::fromUtf8("密码"), pwdRow);
    root->addLayout(form);

    QHBoxLayout *buttonRow = new QHBoxLayout();
    buttonRow->addStretch(1);
    QPushButton *cancel = new QPushButton(QString::fromUtf8("取消"), this);
    QPushButton *confirm = new QPushButton(QString::fromUtf8("登录"), this);
    confirm->setDefault(true);
    buttonRow->addWidget(cancel);
    buttonRow->addWidget(confirm);
    root->addLayout(buttonRow);

    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(confirm, &QPushButton::clicked, this, [this]() {
        const QString uname = usernameEdit->text().trimmed();
        const QString pwd = passwordEdit->text();
        QString errorMsg;
        if (!validateUsername(uname, errorMsg)) {
            QMessageBox::warning(this, QString::fromUtf8("输入错误"), errorMsg);
            return;
        }
        if (!validatePassword(pwd, errorMsg)) {
            QMessageBox::warning(this, QString::fromUtf8("输入错误"), errorMsg);
            return;
        }
        accept();
    });
    connect(togglePwdBtn, &QPushButton::toggled, this, &LoginDialog::togglePasswordVisibility);

    setStyleSheet(R"(
        QDialog {
            background-color: #ffffff;
        }
        QLabel#DialogTitle {
            font-size: 16px;
            font-weight: 600;
            color: #1f2d3d;
        }
        QLabel {
            color: #334155;
            font-size: 13px;
        }
        QLineEdit {
            border: 1px solid #cbd5e1;
            border-radius: 6px;
            padding: 6px 8px;
            min-height: 18px;
        }
        QLineEdit:focus {
            border-color: #3d80de;
        }
        QPushButton {
            padding: 6px 14px;
            border-radius: 6px;
            border: 1px solid #cbd5e1;
            background-color: #f8fafc;
        }
    )");
}

void LoginDialog::togglePasswordVisibility() {
    if (passwordEdit->echoMode() == QLineEdit::Password) {
        passwordEdit->setEchoMode(QLineEdit::Normal);
    } else {
        passwordEdit->setEchoMode(QLineEdit::Password);
    }
}

QString LoginDialog::username() const {
    return usernameEdit->text().trimmed();
}

QString LoginDialog::password() const {
    return passwordEdit->text();
}

bool LoginDialog::validateUsername(const QString &username, QString &errorMsg) {
    if (username.isEmpty()) {
        errorMsg = QString::fromUtf8("用户名不能为空。");
        return false;
    }
    if (username.length() > 20) {
        errorMsg = QString::fromUtf8("用户名长度不能超过 20 个字符。");
        return false;
    }
    QRegularExpression re("^[a-zA-Z][a-zA-Z0-9_]*$");
    if (!re.match(username).hasMatch()) {
        errorMsg = QString::fromUtf8("用户名必须以字母开头，且仅包含字母、数字和下划线。");
        return false;
    }
    return true;
}

bool LoginDialog::validatePassword(const QString &password, QString &errorMsg) {
    if (password.isEmpty()) {
        errorMsg = QString::fromUtf8("密码不能为空。");
        return false;
    }
    if (password.length() < 1 || password.length() > 30) {
        errorMsg = QString::fromUtf8("密码长度必须在 1 到 30 个字符之间。");
        return false;
    }
    return true;
}

} // namespace sjtu::client
