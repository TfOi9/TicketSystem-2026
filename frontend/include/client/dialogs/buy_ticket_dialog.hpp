#ifndef BUY_TICKET_DIALOG_HPP
#define BUY_TICKET_DIALOG_HPP

#include <QDialog>

class QLabel;
class QSpinBox;
class QCheckBox;

namespace sjtu {
namespace client {

class BuyTicketDialog : public QDialog {
    Q_OBJECT

public:
    struct TicketInfo {
        QString trainName;
        QString fromStation;
        QString toStation;
        QString departureTime;
        QString arrivalTime;
        int price;
        int remain;
        QString date;
    };

    explicit BuyTicketDialog(const TicketInfo &info, QWidget *parent = nullptr);

    int ticketCount() const;
    bool useQueue() const;

private:
    QSpinBox *countSpinBox;
    QCheckBox *queueCheckBox;
};

} // namespace client
} // namespace sjtu

#endif // BUY_TICKET_DIALOG_HPP
