#ifndef DIALOG_SETTIMERANGE_H
#define DIALOG_SETTIMERANGE_H

#include <QDialog>

namespace Ui {
class DialogSetTimeRange;
}

class DialogSetTimeRange : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSetTimeRange(QWidget *parent = nullptr);
    ~DialogSetTimeRange();

    double Min();

    double Max();

private slots:
    void on_dateTimeEdit_min_dateTimeChanged(const QDateTime &dateTime);

    void on_dateTimeEdit_max_dateTimeChanged(const QDateTime &dateTime);

    void on_comboBox_range_currentIndexChanged(int index);

private:
    Ui::DialogSetTimeRange *ui;
    double min_time_;
    double max_time_;
};

#endif // DIALOG_SETTIMERANGE_H
