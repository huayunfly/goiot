#ifndef FORM_MOTORCONTROL_H
#define FORM_MOTORCONTROL_H

#include <QWidget>

namespace Ui {
class FormMotorControl;
}

class FormMotorControl : public QWidget
{
    Q_OBJECT

public:
    explicit FormMotorControl(QWidget *parent = nullptr);
    ~FormMotorControl();

private:
    void OnBtnClicked(void);

private:
    Ui::FormMotorControl *ui;
};

#endif // FORM_MOTORCONTROL_H
