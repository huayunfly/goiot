#include <qevent.h>
#include "dialog_inputinfo.h"
#include "ui_dialog_inputinfo.h"


DialogInputInfo::DialogInputInfo(QWidget *parent, const QString& headline) :
    QDialog(parent),
    ui(new Ui::DialogInputInfo),
    _headline(headline),
    _ok(false)
{
    ui->setupUi(this);
    this->setWindowTitle("设置输入");
}

DialogInputInfo::~DialogInputInfo()
{
    delete ui;
}

void DialogInputInfo::keyPressEvent(QKeyEvent* e)
{
    assert(e != nullptr);
    if( e->key() == Qt::Key_Return)
    {
        this->accept();
    }
    else if (e->key() == Qt::Key_Escape)
    {
        this->reject();
    }
    else
    {
        QDialog::keyPressEvent(e);
    }
}

void DialogInputInfo::on_lineEdit_info_textChanged(const QString &arg1)
{
    if (!arg1.isEmpty() && arg1.length() < 25)
    {
        _new_input = arg1;
    }
}

void DialogInputInfo::on_button_ok_clicked()
{
    _ok = true;
    this->close();
}

void DialogInputInfo::on_button_cancel_clicked()
{
    _ok = false;
    this->close();
}
