#ifndef PROFILE_DIALOG_HPP
#define PROFILE_DIALOG_HPP

#include <QDialog>

class QLabel;

namespace sjtu {
namespace client {

class ProfileDialog : public QDialog {
    Q_OBJECT

public:
    explicit ProfileDialog(QWidget *parent = nullptr);

    void setProfile(const QString &username, const QString &name, const QString &email, int privilege);

private:
    QLabel *usernameValue;
    QLabel *nameValue;
    QLabel *emailValue;
    QLabel *privilegeValue;
};

} // namespace client
} // namespace sjtu

#endif // PROFILE_DIALOG_HPP
