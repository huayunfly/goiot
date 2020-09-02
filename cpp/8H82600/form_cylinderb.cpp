#include "form_cylinderb.h"
#include "ui_form_cylinderb.h"

FormCylinderB::FormCylinderB(QWidget *parent) :
    FormCommon(parent),
    ui(new Ui::FormCylinderB)
{
    ui->setupUi(this);
    this->setObjectName("cylinderb");
    InitUiState();
}

FormCylinderB::~FormCylinderB()
{
    delete ui;
}

bool FormCylinderB::event(QEvent *event)
{
    if (event == nullptr)
    {
        return false;
    }

    return FormCommon::event(event);
}

void FormCylinderB::InitUiState()
{

}
