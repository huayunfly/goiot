#ifndef FORM_MOTORCONTROL_H
#define FORM_MOTORCONTROL_H

#include <QWidget>
#include "events.h"

namespace Ui {
class FormMotorControl;
}

class FormMotorControl : public QWidget
{
    Q_OBJECT

public:
    explicit FormMotorControl(QWidget *parent = nullptr);
    ~FormMotorControl();

protected:
    /// <summary>
    /// Event handlers.
    /// </summary>
    bool event(QEvent* event) override;

private:
    void OnBtnClicked(void);

private:
    Ui::FormMotorControl *ui;
};

#endif // FORM_MOTORCONTROL_H
