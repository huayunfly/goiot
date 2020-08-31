#include "form_liquidswitch.h"
#include "ui_form_liquidswitch.h"

FormLiquidSwitch::FormLiquidSwitch(QWidget *parent) :
    FormCommon(parent),
    ui(new Ui::FormLiquidSwitch)
{
    ui->setupUi(this);
    this->setObjectName("liquidswtich");
    InitUiState();
}

FormLiquidSwitch::~FormLiquidSwitch()
{
    delete ui;
}

bool FormLiquidSwitch::event(QEvent *event)
{
    if (event == nullptr)
    {
        return false;
    }

    return FormCommon::event(event);
}

void FormLiquidSwitch::InitUiState()
{

}
