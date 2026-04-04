#ifndef REGISTER_DIALOG_HPP
#define REGISTER_DIALOG_HPP

#include <QDialog>

class QLineEdit;

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

private:
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QLineEdit *nameEdit;
    QLineEdit *emailEdit;
};

} // namespace client
} // namespace sjtu

#endif // REGISTER_DIALOG_HPP
