#ifndef REFUND_DIALOG_HPP
#define REFUND_DIALOG_HPP

#include <QDialog>

class QLabel;

namespace sjtu {
namespace client {

class RefundDialog : public QDialog {
    Q_OBJECT

public:
    struct RefundInfo {
        QString trainName;
        QString fromStation;
        QString toStation;
        QString departureTime;
        int price;
        int count;
        QString statusText;
        int orderIndex;
    };

    explicit RefundDialog(const RefundInfo &info, QWidget *parent = nullptr);

    int orderIndex() const;

private:
    int orderIndex_;
};

} // namespace client
} // namespace sjtu

#endif // REFUND_DIALOG_HPP
