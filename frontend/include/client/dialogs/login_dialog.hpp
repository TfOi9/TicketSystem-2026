#ifndef LOGIN_DIALOG_HPP
#define LOGIN_DIALOG_HPP

#include <QDialog>

class QLineEdit;

namespace sjtu {
namespace client {

class LoginDialog : public QDialog {
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);

    QString username() const;
    QString password() const;

private:
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
};

} // namespace client
} // namespace sjtu

#endif // LOGIN_DIALOG_HPP
