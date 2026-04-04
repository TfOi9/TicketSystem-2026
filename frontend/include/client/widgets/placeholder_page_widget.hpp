#ifndef PLACEHOLDER_PAGE_WIDGET_HPP
#define PLACEHOLDER_PAGE_WIDGET_HPP

#include <QWidget>

namespace sjtu {
namespace client {

class PlaceholderPageWidget : public QWidget {
    Q_OBJECT

public:
    explicit PlaceholderPageWidget(const QString &title, QWidget *parent = nullptr);
};

} // namespace client
} // namespace sjtu

#endif // PLACEHOLDER_PAGE_WIDGET_HPP
