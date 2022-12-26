#ifndef DIALOG_LIQUIDSAMPLINGCHECK_H
#define DIALOG_LIQUIDSAMPLINGCHECK_H

#include <QDialog>

namespace Ui {
class DialogLiquidSamplingCheck;
}

class DialogLiquidSamplingCheck : public QDialog
{
    Q_OBJECT

public:
    enum class TaskCategory
    {
        SAMPLING = 0,
        COLLECTION = 1
    };

public:
    explicit DialogLiquidSamplingCheck(QWidget *parent = nullptr,
                                       TaskCategory category = TaskCategory::SAMPLING);
    ~DialogLiquidSamplingCheck();

private slots:
    void on_checkBox_power_stateChanged(int arg1);

    void on_checkBox_purge_stateChanged(int arg1);

    void on_checkBox_sink_stateChanged(int arg1);

    void on_checkBox_obstacle_stateChanged(int arg1);

    void on_checkBox_light_stateChanged(int arg1);

    void on_checkBox_exp_stop_stateChanged(int arg1);

private:
    void CheckStatus();

private:
    Ui::DialogLiquidSamplingCheck *ui;
    TaskCategory category_;
};

#endif // DIALOG_LIQUIDSAMPLINGCHECK_H
