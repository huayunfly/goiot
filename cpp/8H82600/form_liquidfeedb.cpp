#include "form_liquidfeedb.h"
#include "ui_form_liquidfeedb.h"

FormLiquidFeedB::FormLiquidFeedB(QWidget *parent) :
    FormCommon(parent),
    ui(new Ui::FormLiquidFeedB)
{
    ui->setupUi(this);
    this->setObjectName("liquidfeedb");
    InitUiState();
}

FormLiquidFeedB::~FormLiquidFeedB()
{
    delete ui;
}

bool FormLiquidFeedB::event(QEvent *event)
{
    if (event == nullptr)
    {
        return false;
    }

    return FormCommon::event(event);
}

void FormLiquidFeedB::InitUiState()
{

}
