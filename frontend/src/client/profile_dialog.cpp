#include "../../include/client/profile_dialog.hpp"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace sjtu::client {

ProfileDialog::ProfileDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("个人信息");
    setModal(true);
    setMinimumWidth(360);

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(16, 16, 16, 16);
    root->setSpacing(12);

    QLabel *title = new QLabel("当前用户信息", this);
    title->setObjectName("DialogTitle");
    root->addWidget(title);

    QFormLayout *form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight);
    form->setHorizontalSpacing(10);
    form->setVerticalSpacing(10);

    usernameValue = new QLabel("-", this);
    nameValue = new QLabel("-", this);
    emailValue = new QLabel("-", this);
    privilegeValue = new QLabel("-", this);

    form->addRow("用户名", usernameValue);
    form->addRow("姓名", nameValue);
    form->addRow("邮箱", emailValue);
    form->addRow("权限", privilegeValue);
    root->addLayout(form);

    QHBoxLayout *buttonRow = new QHBoxLayout();
    buttonRow->addStretch(1);
    QPushButton *ok = new QPushButton("关闭", this);
    buttonRow->addWidget(ok);
    root->addLayout(buttonRow);

    connect(ok, &QPushButton::clicked, this, &QDialog::accept);

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
        QPushButton {
            padding: 6px 14px;
            border-radius: 6px;
            border: 1px solid #cbd5e1;
            background-color: #f8fafc;
        }
    )");
}

void ProfileDialog::setProfile(const QString &username, const QString &name, const QString &email, int privilege) {
    usernameValue->setText(username);
    nameValue->setText(name);
    emailValue->setText(email);
    privilegeValue->setText(QString::number(privilege));
}

} // namespace sjtu::client
