#include "client/dialogs/login_dialog.hpp"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace sjtu::client {

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("登录");
    setModal(true);
    setMinimumWidth(360);

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(16, 16, 16, 16);
    root->setSpacing(12);

    QLabel *title = new QLabel("登录 TicketSystem", this);
    title->setObjectName("DialogTitle");
    root->addWidget(title);

    QFormLayout *form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight);
    form->setHorizontalSpacing(10);
    form->setVerticalSpacing(10);

    usernameEdit = new QLineEdit(this);
    usernameEdit->setPlaceholderText("用户名");
    passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText("密码");
    passwordEdit->setEchoMode(QLineEdit::Password);

    form->addRow("用户名", usernameEdit);
    form->addRow("密码", passwordEdit);
    root->addLayout(form);

    QHBoxLayout *buttonRow = new QHBoxLayout();
    buttonRow->addStretch(1);
    QPushButton *cancel = new QPushButton("取消", this);
    QPushButton *confirm = new QPushButton("登录", this);
    confirm->setDefault(true);
    buttonRow->addWidget(cancel);
    buttonRow->addWidget(confirm);
    root->addLayout(buttonRow);

    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(confirm, &QPushButton::clicked, this, &QDialog::accept);

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

QString LoginDialog::username() const {
    return usernameEdit->text().trimmed();
}

QString LoginDialog::password() const {
    return passwordEdit->text();
}

} // namespace sjtu::client
