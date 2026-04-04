#ifndef TICKET_QUERY_WIDGET_HPP
#define TICKET_QUERY_WIDGET_HPP

#include <QWidget>

class QLineEdit;
class QDateEdit;
class QPushButton;

namespace sjtu {
namespace client {

class TicketQueryWidget : public QWidget {
    Q_OBJECT

public:
    explicit TicketQueryWidget(QWidget *parent = nullptr);

signals:
    void queryTicketRequested(const QString &fromStation, const QString &toStation, const QString &date);

private slots:
    void onSwapClicked();
    void onQueryClicked();

private:
    QLineEdit *fromStationEdit;
    QLineEdit *toStationEdit;
    QDateEdit *dateEdit;
    QPushButton *swapButton;
    QPushButton *queryButton;
};

} // namespace client
} // namespace sjtu

#endif // TICKET_QUERY_WIDGET_HPP
