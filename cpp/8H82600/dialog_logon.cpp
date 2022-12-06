#include "dialog_logon.h"
#include "ui_dialog_logon.h"

DialogLogon::DialogLogon(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogLogon), admin_logon_(false)
{
    ui->setupUi(this);
}

DialogLogon::~DialogLogon()
{
    delete ui;
}

void DialogLogon::on_pushButton_logon_clicked()
{
    if (ui->lineEdit_user->text().compare("jiahua", Qt::CaseInsensitive) == 0 &&
            ui->lineEdit_password->text().compare("jiahua123") == 0)
    {
        this->accept();
    }
    else if (ui->lineEdit_user->text().compare("admin", Qt::CaseInsensitive) == 0 &&
             ui->lineEdit_password->text().compare("hello@123") == 0)
    {
        admin_logon_ = true;
        this->accept();
    }
    else
    {
        this->ui->label_tip->setText("账号或密码错误");
    }
}

void DialogLogon::on_lineEdit_user_textChanged(const QString &arg1)
{
    this->ui->label_tip->clear();
}

void DialogLogon::on_lineEdit_password_textChanged(const QString &arg1)
{
    this->ui->label_tip->clear();
}
