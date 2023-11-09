#include "form_expinfo.h"
#include "ui_form_expinfo.h"
#include "QFileDialog"
#include <thread>

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
    //ui->textEdit_recordpath->clear();
    //ui->textEdit_recordpath->setText(folderPath);
    QString button_id = "button_filefolder_selection";
    bool ok = write_data_func_(this->objectName(), button_id, folderPath);
    assert(ok);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}
