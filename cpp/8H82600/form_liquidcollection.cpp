#include "form_liquidcollection.h"
#include "ui_form_liquidcollection.h"

FormLiquidCollection::FormLiquidCollection(QWidget *parent) :
    FormCommon(parent),
    ui(new Ui::FormLiquidCollection)
{
    ui->setupUi(this);
    this->setObjectName("liquidcollection");
    InitUiState();
}

FormLiquidCollection::~FormLiquidCollection()
{
    delete ui;
}

bool FormLiquidCollection::event(QEvent *event)
{
    if (event == nullptr)
    {
        return false;
    }

    return FormCommon::event(event);
}

void FormLiquidCollection::InitUiState()
{

}
