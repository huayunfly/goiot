#ifndef FORM_MOTORCONTROL_H
#define FORM_MOTORCONTROL_H

#include <QWidget>
#include "events.h"
#include "form_common.h"

namespace Ui {
class FormMotorControl;
}

enum class MotorGroup
{
    CYLINDER16 = 0,
    CYLINDER32 = 1,
    REACTOR = 2
};

class FormMotorControl : public FormCommon
{
    Q_OBJECT

public:
    explicit FormMotorControl(QWidget *parent = nullptr,
                              const QString& object_name = QString(),
                              const QString& display_name = QString(),
                              MotorGroup group = MotorGroup::CYLINDER16);
    ~FormMotorControl();

protected:
    /// <summary>
    /// Event handlers.
    /// </summary>
    bool event(QEvent* event) override;

private:
    void on_buttonClicked(void);
    void on_cellChanged(int row, int column);

private:
    Ui::FormMotorControl *ui;
    MotorGroup group_;
    // Class specified const
    const int ROW_COUNT = 16;
    const int COL_COUNT = 12;
    const int COL_NAME = 0;
    const int COL_STATUS = 1;
    const int COL_ENABLE = 2;
    const int COL_DISABLE = 3;
    const int COL_ALARM = 4;
    const int COL_CLEAR_ALARM = 5;
    const int COL_RUN = 6;
    const int COL_PV = 7;
    const int COL_SV = 8;
    const int COL_SPEED = 9;
    const int COL_START = 10;
    const int COL_STOP = 11;
    // Cylinder motor specified parameters
    const float MAX_REACTOR_SPEED = 600.0f; /*rpm*/
    const int REACTOR_REDUCTION_RATIO = 5; /*反应器伺服减速比*/
    const float MAX_CYLINDER_FLOWRATE = 40.0f; /*mL/min*/
    const float MAX_TARGET_IN_ML = 100.0f; /*mL*/
    const int BLOCK_NUM_MOVE = 0; // 缸活塞绝对量移动至指定位置Block number
    const float CYLINDER_VOLUME = 100; /*缸有效容积ml*/
    const float PITCH = 10.0f; /*缸螺距mm*/
    const int CYLINDER_REDUCTION_RATIO = 700; /*减速比*/
    const float TOTAL_TRAVEL = 98.244f; /*对应100mL总行程mm*/
    const int MOTOR_UNIT_PER_MILLIMETER = 1000000; /* 每毫米电机指令单位 */
};

#endif // FORM_MOTORCONTROL_H
