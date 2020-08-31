#include "form_liquidsamplinga.h"
#include "ui_form_liquidsamplinga.h"

FormLiquidSamplingA::FormLiquidSamplingA(QWidget *parent) :
    FormCommon(parent),
    ui(new Ui::FormLiquidSamplingA)
{
    ui->setupUi(this);
    this->setObjectName("liquidsamplinga");
    InitUiState();
}

FormLiquidSamplingA::~FormLiquidSamplingA()
{
    delete ui;
}

bool FormLiquidSamplingA::event(QEvent *event)
{
    if (event == nullptr)
    {
        return false;
    }

    return FormCommon::event(event);
}

void FormLiquidSamplingA::InitUiState()
{

}
