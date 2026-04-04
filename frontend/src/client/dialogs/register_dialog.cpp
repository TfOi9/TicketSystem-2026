#include "client/dialogs/register_dialog.hpp"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace sjtu::client {

RegisterDialog::RegisterDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("注册用户");
    setModal(true);
    setMinimumWidth(380);

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(16, 16, 16, 16);
    root->setSpacing(12);

    QLabel *title = new QLabel("创建新用户", this);
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
    nameEdit = new QLineEdit(this);
    nameEdit->setPlaceholderText("姓名");
    emailEdit = new QLineEdit(this);
    emailEdit->setPlaceholderText("邮箱");

    form->addRow("用户名", usernameEdit);
    form->addRow("密码", passwordEdit);
    form->addRow("姓名", nameEdit);
    form->addRow("邮箱", emailEdit);
    root->addLayout(form);

    QHBoxLayout *buttonRow = new QHBoxLayout();
    buttonRow->addStretch(1);
    QPushButton *cancel = new QPushButton("取消", this);
    QPushButton *confirm = new QPushButton("注册", this);
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

QString RegisterDialog::username() const {
    return usernameEdit->text().trimmed();
}

QString RegisterDialog::password() const {
    return passwordEdit->text();
}

QString RegisterDialog::displayName() const {
    return nameEdit->text().trimmed();
}

QString RegisterDialog::email() const {
    return emailEdit->text().trimmed();
}

} // namespace sjtu::client
