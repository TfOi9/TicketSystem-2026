#ifndef ORDERS_PAGE_WIDGET_HPP
#define ORDERS_PAGE_WIDGET_HPP

#include <QWidget>
#include <QVector>

class QLabel;
class QTableWidget;
class QPushButton;

namespace sjtu {
namespace client {

class MainWindow;

class OrdersPageWidget : public QWidget {
    Q_OBJECT

    friend class MainWindow;

public:
    struct OrderItem {
        int index;
        QString trainName;
        QString startStation;
        QString endStation;
        QString departureTime;
        QString arrivalTime;
        int price;
        int count;
        int status; // 0=Invalid, 1=Purchased, 2=Pending, 3=Refunded
    };

    explicit OrdersPageWidget(QWidget *parent = nullptr);

    void setOrders(const QVector<OrderItem> &orders);
    void clearOrders();

signals:
    void refundRequested(int orderIndex);
    void refreshRequested();

private slots:
    void onRefundClicked(int orderIndex);

private:
    QLabel *titleLabel;
    QPushButton *refreshButton;
    QTableWidget *tableWidget;
};

} // namespace client
} // namespace sjtu

#endif // ORDERS_PAGE_WIDGET_HPP
