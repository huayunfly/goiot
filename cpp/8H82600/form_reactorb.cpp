#include "form_reactorb.h"
#include "ui_form_reactorb.h"

FormReactorB::FormReactorB(QWidget *parent) :
    FormCommon(parent),
    ui(new Ui::FormReactorB)
{
    ui->setupUi(this);
    this->setObjectName("reactorb");
    InitUiState();
}

FormReactorB::~FormReactorB()
{
    delete ui;
}

bool FormReactorB::event(QEvent *event)
{
    if (event == nullptr)
    {
        return false;
    }

    return FormCommon::event(event);
}

void FormReactorB::InitUiState()
{

}
