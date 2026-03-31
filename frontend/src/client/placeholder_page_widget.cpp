#include "../../include/client/placeholder_page_widget.hpp"

#include <QLabel>
#include <QVBoxLayout>

namespace sjtu::client {

PlaceholderPageWidget::PlaceholderPageWidget(const QString &title, QWidget *parent) : QWidget(parent) {
    setObjectName("PlaceholderPage");
    setAttribute(Qt::WA_StyledBackground);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);

    QLabel *titleLabel = new QLabel(title, this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setObjectName("PlaceholderTitle");

    QLabel *hint = new QLabel("此页面正在建设中", this);
    hint->setAlignment(Qt::AlignCenter);
    hint->setObjectName("PlaceholderHint");

    layout->addStretch(1);
    layout->addWidget(titleLabel);
    layout->addWidget(hint);
    layout->addStretch(1);

    setStyleSheet(R"(
        #PlaceholderPage {
            background-color: #eef3fb;
        }

        #PlaceholderTitle {
            color: #1e3a5f;
            font-size: 28px;
            font-weight: 700;
        }

        #PlaceholderHint {
            color: #64748b;
            font-size: 14px;
        }
    )");
}

} // namespace sjtu::client
