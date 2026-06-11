#ifndef REGISTER_DIALOG_HPP
#define REGISTER_DIALOG_HPP

#include <QDialog>

class QLineEdit;
class QPushButton;

namespace sjtu {
namespace client {

class RegisterDialog : public QDialog {
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);

    QString username() const;
    QString password() const;
    QString displayName() const;
    QString email() const;

    static bool validateUsername(const QString &username, QString &errorMsg);
    static bool validatePassword(const QString &password, QString &errorMsg);
    static bool validateName(const QString &name, QString &errorMsg);
    static bool validateEmail(const QString &email, QString &errorMsg);

private slots:
    void togglePasswordVisibility();

private:
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QLineEdit *nameEdit;
    QLineEdit *emailEdit;
    QPushButton *togglePwdBtn;
};

} // namespace client
} // namespace sjtu

#endif // REGISTER_DIALOG_HPP
