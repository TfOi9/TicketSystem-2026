#ifndef STATUS_BAR_HPP
#define STATUS_BAR_HPP

#include <QWidget>

class QLabel;

namespace sjtu {
namespace client {

class StatusBar : public QWidget {
    Q_OBJECT

public:
    explicit StatusBar(QWidget *parent = nullptr);

public slots:
    void setConnectionStatus(const QString &status);

private:
    QLabel *connectionStatusLabel;
};

} // namespace client
} // namespace sjtu

#endif // STATUS_BAR_HPP
