#ifndef HOME_PAGE_WIDGET_HPP
#define HOME_PAGE_WIDGET_HPP

#include <QWidget>

namespace sjtu {
namespace client {

class TicketQueryWidget;

class HomePageWidget : public QWidget {
    Q_OBJECT

public:
    explicit HomePageWidget(QWidget *parent = nullptr);

signals:
    void queryTicketRequested(const QString &fromStation, const QString &toStation, const QString &date);

private:
    TicketQueryWidget *queryWidget;
};

} // namespace client
} // namespace sjtu

#endif // HOME_PAGE_WIDGET_HPP
