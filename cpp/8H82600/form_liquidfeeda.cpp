#include "form_liquidfeeda.h"
#include "ui_form_liquidfeeda.h"

FormLiquidFeedA::FormLiquidFeedA(QWidget *parent) :
    FormCommon(parent),
    ui(new Ui::FormLiquidFeedA)
{
    ui->setupUi(this);
    this->setObjectName("liquidfeeda");
    InitUiState();
}

FormLiquidFeedA::~FormLiquidFeedA()
{
    delete ui;
}

bool FormLiquidFeedA::event(QEvent *event)
{
    if (event == nullptr)
    {
        return false;
    }

    return FormCommon::event(event);
}

void FormLiquidFeedA::InitUiState()
{

}
