#ifndef TOP_BAR_HPP
#define TOP_BAR_HPP

#include <QWidget>
#include <QLabel>
#include <QPushButton>

#include <functional>

namespace sjtu {
namespace client {

class TopBar : public QWidget {
    Q_OBJECT

public:
    TopBar(QWidget *parent = nullptr);

    void refreshBar();
    void setAuthState(bool loggedIn, const QString &username, bool isAdmin);
    
    void setAuthChangedCallBack(const std::function<void(const QString &)> &cb) {
        authChangedCallback = cb;
    }

signals:
    void mainButtonClicked();
    void ticketButtonClicked();
    void orderButtonClicked();
    void manageButtonClicked();
    void loginRequested();
    void registerRequested();
    void logoutRequested();
    void profileRequested();
    void authChanged(const QString &msg);

private slots:
    void onLoginButtonClicked();
    void onRegisterButtonClicked();
    void onLogoutButtonClicked();
    void onProfileButtonClicked();
    void onMainButtonClicked();
    void onTicketButtonClicked();
    void onOrderButtonClicked();
    void onManageButtonClicked();

private:
    QLabel *titleLabel;
    QPushButton *loginButton;
    QPushButton *registerButton;
    QPushButton *mainButton;
    QPushButton *ticketButton;
    QPushButton *orderButton;
    QPushButton *manageButton;
    QPushButton *userButton;

    bool loggedIn;
    bool isAdmin;
    QString currentUsername;

    std::function<void(const QString &)> authChangedCallback;

    void applyStyle();

};

} // namespace client
} // namespace sjtu

#endif // TOP_BAR_HPP