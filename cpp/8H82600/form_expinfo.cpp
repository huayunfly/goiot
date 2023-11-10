#include <thread>
#include <chrono>
#include "form_expinfo.h"
#include "ui_form_expinfo.h"
#include "QFileDialog"
#include "dialog_inputinfo.h"


FormExpInfo::FormExpInfo(QWidget *parent, bool admin) :
    FormCommon(parent, "expinfo", QString::fromUtf8("实验设置"), admin),
    ui(new Ui::FormExpInfo)
{
    ui->setupUi(this);
}

FormExpInfo::~FormExpInfo()
{
    delete ui;
}

bool FormExpInfo::event(QEvent *event)
{
    if (event == nullptr)
    {
        return false;
    }

    return FormCommon::event(event);
}

void FormExpInfo::InitUiState()
{

}

void FormExpInfo::on_button_filefolder_selection_clicked()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, "选择记录文件夹");
    if (folderPath.isEmpty())
    {
        return;
    }
    QString button_id = "button_filefolder_selection";
    bool ok = write_data_func_(this->objectName(), button_id, folderPath);
    assert(ok);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

void FormExpInfo::on_button_expname_clicked()
{
    DialogInputInfo dlg_input(this, "实验名称");
    dlg_input.exec();

    if(dlg_input.IsOK())
    {
        QString button_id = "button_expname";
        QString expname = dlg_input.NewInput();
        if (expname.isEmpty())
        {
            return;
        }
        bool ok = write_data_func_(this->objectName(), button_id, expname);
        assert(ok);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void FormExpInfo::on_button_exp_run_clicked()
{
    QString tos = QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0,
                                  'g', 16);
    bool ok = write_data_func_(this->objectName(), "button_exp_run", QString::number(1));
    assert(ok);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    // Set Experiment TOS (time of start), using virtual button_id
    ok = write_data_func_(this->objectName(), "button_exp_tos", tos);
    assert(ok);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    if (ok)
    {
        ui->button_exp_run->setEnabled(false);
        ui->button_exp_stop->setEnabled(true);
    }
}

void FormExpInfo::on_button_exp_stop_clicked()
{
    bool ok = write_data_func_(this->objectName(), "button_exp_run", QString::number(0));
    assert(ok);
    // Set Experiment TOS (time of start), using virtual button_id
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    if (ok)
    {
        ui->button_exp_run->setEnabled(true);
        ui->button_exp_stop->setEnabled(false);
    }
}
