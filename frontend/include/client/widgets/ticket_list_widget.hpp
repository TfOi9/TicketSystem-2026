#ifndef TICKET_LIST_WIDGET_HPP
#define TICKET_LIST_WIDGET_HPP

#include <QWidget>
#include <QVector>

class QLabel;
class QTableWidget;

namespace sjtu {
namespace client {

class TicketListWidget : public QWidget {
    Q_OBJECT

public:
    struct TicketListItem {
        QString trainName;
        QString startStation;
        QString endStation;
        QString departureTime;
        QString arrivalTime;
        int durationMinutes;
        qint64 departureSortKey;
        qint64 arrivalSortKey;
        int price;
        int remain;
    };

    explicit TicketListWidget(QWidget *parent = nullptr);

public slots:
    void clearTickets();
    void setTickets(const QVector<TicketListItem> &tickets);

signals:
    void trainNameClicked(const QString &trainName);
    void purchaseRequested(const QString &trainName);

private slots:
    void onCellClicked(int row, int column);
    void onPurchaseButtonClicked(const QString &trainName);

private:
    QLabel *titleLabel;
    QTableWidget *tableWidget;
};

} // namespace client
} // namespace sjtu

#endif // TICKET_LIST_WIDGET_HPP
