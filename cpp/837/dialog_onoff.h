#ifndef DIALOG_ONOFF_H
#define DIALOG_ONOFF_H

#include <QDialog>

namespace Ui {
class DialogOnOff;
}

class DialogOnOff : public QDialog
{
    Q_OBJECT

public:
    static const int ON = 1;
    static const int OFF = 0;

    explicit DialogOnOff(QWidget *parent = nullptr, int on = 0);
    ~DialogOnOff();

    int NewValue()
    {
        return new_value_;
    }

private slots:
    void on_toolButton_clicked();

private:
    Ui::DialogOnOff *ui;
    int new_value_;
};

#endif // DIALOG_ONOFF_H
