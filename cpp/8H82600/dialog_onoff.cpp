#include "dialog_onoff.h"
#include "ui_dialog_onoff.h"

DialogOnOff::DialogOnOff(QWidget *parent, int on) :
    QDialog(parent),
    ui(new Ui::DialogOnOff), new_value_(on)
{
    ui->setupUi(this);
    this->setFixedSize(QSize(48, 56));
    this->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    QIcon button_icon;
    if (new_value_ == OFF)
    {
        button_icon.addFile(":/ui_ctrls/UICtrls/Res/poweron.bmp");
    }
    else
    {
        button_icon.addFile(":/ui_ctrls/UICtrls/Res/poweroff.bmp");
    }
    ui->toolButton->setIcon(button_icon); // pushButton->setIcon() 图像错位
    ui->toolButton->setIconSize(QSize(48, 56));
}

DialogOnOff::~DialogOnOff()
{
    delete ui;
}

void DialogOnOff::on_toolButton_clicked()
{
    if (new_value_ == OFF)
    {
        new_value_ = ON;
    }
    else
    {
        new_value_ = OFF;
    }
    this->accept();
}
