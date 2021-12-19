#ifndef DIALOG_SETVALUE_H
#define DIALOG_SETVALUE_H

#include <QDialog>
#include "data_model.h"

namespace Ui {
class DialogSetValue;
}

class DialogSetValue : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSetValue(QWidget *parent = nullptr, const QString& current = QString(),
                            MeasurementUnit unit = MeasurementUnit::NONE, int high_limit = 0, int low_limit = 0);
    ~DialogSetValue();

    void keyPressEvent(QKeyEvent* e) override;

    float NewValue()
    {
        return new_value_;
    }

private slots:
    void on_lineEditSV_textChanged(const QString &arg1);

private:
    Ui::DialogSetValue *ui;
    float new_value_;
    int high_limit_;
    int low_limit_;
};

#endif // DIALOG_SETVALUE_H
