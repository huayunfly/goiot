#include "form_liquidsamplingb.h"
#include "ui_form_liquidsamplingb.h"

FormLiquidSamplingB::FormLiquidSamplingB(QWidget *parent) :
    FormCommon(parent),
    ui(new Ui::FormLiquidSamplingB)
{
    ui->setupUi(this);
    this->setObjectName("liquidsamplingb");
    InitUiState();
}

FormLiquidSamplingB::~FormLiquidSamplingB()
{
    delete ui;
}

bool FormLiquidSamplingB::event(QEvent *event)
{
    if (event == nullptr)
    {
        return false;
    }

    return FormCommon::event(event);
}

void FormLiquidSamplingB::InitUiState()
{

}
