#include "form_reactora.h"
#include "ui_form_reactora.h"

FormReactorA::FormReactorA(QWidget *parent) :
    FormCommon(parent),
    ui(new Ui::FormReactorA)
{
    ui->setupUi(this);
    this->setObjectName("reactora");
    InitUiState();
}

FormReactorA::~FormReactorA()
{
    delete ui;
}

bool FormReactorA::event(QEvent *event)
{
    if (event == nullptr)
    {
        return false;
    }

    return FormCommon::event(event);
}

void FormReactorA::InitUiState()
{

}
