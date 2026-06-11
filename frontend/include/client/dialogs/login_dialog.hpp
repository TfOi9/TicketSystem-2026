#ifndef LOGIN_DIALOG_HPP
#define LOGIN_DIALOG_HPP

#include <QDialog>

class QLineEdit;
class QPushButton;

namespace sjtu {
namespace client {

class LoginDialog : public QDialog {
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);

    QString username() const;
    QString password() const;

    static bool validateUsername(const QString &username, QString &errorMsg);
    static bool validatePassword(const QString &password, QString &errorMsg);

private slots:
    void togglePasswordVisibility();

private:
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QPushButton *togglePwdBtn;
};

} // namespace client
} // namespace sjtu

#endif // LOGIN_DIALOG_HPP
