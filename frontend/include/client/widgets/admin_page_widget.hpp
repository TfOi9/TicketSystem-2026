#ifndef ADMIN_PAGE_WIDGET_HPP
#define ADMIN_PAGE_WIDGET_HPP

#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;
class QTextEdit;
class QDateEdit;
class QSpinBox;
class QComboBox;

namespace sjtu {
namespace client {

class AdminPageWidget : public QWidget {
    Q_OBJECT

public:
    explicit AdminPageWidget(QWidget *parent = nullptr);

    void showTrainResult(const QString &info);
    void showUserResult(const QString &info);
    void clearResults();

signals:
    void queryTrainRequested(const QString &trainId, const QString &date);
    void releaseTrainRequested(const QString &trainId);
    void deleteTrainRequested(const QString &trainId);
    void addTrainRequested(const QString &command);
    void importTrainsRequested(const QString &filePath);
    void batchReleaseRequested();
    void queryProfileRequested(const QString &username);
    void addUserRequested(const QString &username, const QString &password,
                          const QString &name, const QString &email, int privilege);

private slots:
    void onQueryTrainClicked();
    void onReleaseTrainClicked();
    void onDeleteTrainClicked();
    void onAddTrainClicked();
    void onImportTrainsClicked();
    void onBatchReleaseClicked();
    void onQueryProfileClicked();
    void onAddUserClicked();

private:
    QLineEdit *trainIdEdit;
    QDateEdit *trainDateEdit;
    QPushButton *queryTrainButton;
    QPushButton *releaseTrainButton;
    QPushButton *deleteTrainButton;
    QPushButton *addTrainButton;
    QPushButton *importTrainButton;
    QPushButton *batchReleaseButton;

    QLineEdit *usernameEdit;
    QPushButton *queryUserButton;
    QPushButton *addUserButton;

    QLabel *trainResultLabel;
    QLabel *userResultLabel;
};

} // namespace client
} // namespace sjtu

#endif // ADMIN_PAGE_WIDGET_HPP
