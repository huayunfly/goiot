#include "dialog_liquidsamplingcheck.h"
#include "ui_dialog_liquidsamplingcheck.h"
#include "QPushButton"

DialogLiquidSamplingCheck::DialogLiquidSamplingCheck(QWidget *parent, TaskCategory category) :
    QDialog(parent),
    ui(new Ui::DialogLiquidSamplingCheck), category_(category)
{
    ui->setupUi(this);
    ui->label_tip->setVisible(false);
    ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(false);
    if (category_ == TaskCategory::SAMPLING)
    {
        ui->checkBox_exp_stop->setEnabled(false);
        this->setWindowTitle(QString("采样任务启动确认"));
    }
    else
    {
        ui->checkBox_light->setEnabled(false);
        this->setWindowTitle(QString("收集任务启动确认"));
    }
}

DialogLiquidSamplingCheck::~DialogLiquidSamplingCheck()
{
    delete ui;
}

void DialogLiquidSamplingCheck::on_checkBox_power_stateChanged(int arg1)
{
    Q_UNUSED(arg1);
    CheckStatus();
}

void DialogLiquidSamplingCheck::on_checkBox_purge_stateChanged(int arg1)
{
    Q_UNUSED(arg1);
    CheckStatus();
}

void DialogLiquidSamplingCheck::on_checkBox_sink_stateChanged(int arg1)
{
    Q_UNUSED(arg1);
    CheckStatus();
}

void DialogLiquidSamplingCheck::on_checkBox_obstacle_stateChanged(int arg1)
{
    Q_UNUSED(arg1);
    CheckStatus();
}

void DialogLiquidSamplingCheck::on_checkBox_light_stateChanged(int arg1)
{
    Q_UNUSED(arg1);
    CheckStatus();
}

void DialogLiquidSamplingCheck::on_checkBox_exp_stop_stateChanged(int arg1)
{
    Q_UNUSED(arg1);
    CheckStatus();
}

void DialogLiquidSamplingCheck::CheckStatus()
{
    if (category_ == TaskCategory::SAMPLING &&
            ui->checkBox_power->checkState() == Qt::CheckState::Checked &&
            ui->checkBox_light->checkState() == Qt::CheckState::Checked &&
            ui->checkBox_obstacle->checkState() == Qt::CheckState::Checked &&
            ui->checkBox_sink->checkState() == Qt::CheckState::Checked &&
            ui->checkBox_purge->checkState() == Qt::CheckState::Checked)
    {
        ui->label_tip->setVisible(true);
        ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(true);
    }
    else if (category_ == TaskCategory::COLLECTION &&
            ui->checkBox_power->checkState() == Qt::CheckState::Checked &&
            ui->checkBox_obstacle->checkState() == Qt::CheckState::Checked &&
            ui->checkBox_sink->checkState() == Qt::CheckState::Checked &&
            ui->checkBox_purge->checkState() == Qt::CheckState::Checked &&
            ui->checkBox_exp_stop ->checkState() == Qt::CheckState::Checked)
    {
        ui->label_tip->setVisible(true);
        ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(true);
    }
    else
    {
        ui->label_tip->setVisible(false);
        ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(false);
    }
}
