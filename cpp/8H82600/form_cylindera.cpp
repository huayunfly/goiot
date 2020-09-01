#include "form_cylindera.h"
#include "ui_form_cylindera.h"

FormCylinderA::FormCylinderA(QWidget *parent) :
    FormCommon(parent),
    ui(new Ui::FormCylinderA)
{
    ui->setupUi(this);
    this->setObjectName("cylindera");
    InitUiState();
}

FormCylinderA::~FormCylinderA()
{
    delete ui;
}

bool FormCylinderA::event(QEvent *event)
{
    if (event == nullptr)
    {
        return false;
    }

    return FormCommon::event(event);
}

void FormCylinderA::InitUiState()
{

}
