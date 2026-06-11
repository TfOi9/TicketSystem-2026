#ifndef ADD_TRAIN_DIALOG_HPP
#define ADD_TRAIN_DIALOG_HPP

#include <QDialog>

class QLineEdit;
class QDateEdit;
class QSpinBox;
class QComboBox;

namespace sjtu {
namespace client {

class AddTrainDialog : public QDialog {
    Q_OBJECT

public:
    explicit AddTrainDialog(QWidget *parent = nullptr);

    QString commandLine() const;

private:
    QLineEdit *trainIdEdit;
    QSpinBox *stationCountSpin;
    QSpinBox *seatNumSpin;
    QLineEdit *stationsEdit;
    QLineEdit *pricesEdit;
    QLineEdit *startTimeEdit;
    QLineEdit *travelTimesEdit;
    QLineEdit *stopoverTimesEdit;
    QDateEdit *saleDateBeginEdit;
    QDateEdit *saleDateEndEdit;
    QComboBox *typeCombo;
};

} // namespace client
} // namespace sjtu

#endif // ADD_TRAIN_DIALOG_HPP
