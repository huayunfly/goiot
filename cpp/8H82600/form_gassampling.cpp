#include "form_gassampling.h"
#include "ui_form_gassampling.h"

FormGasSampling::FormGasSampling(QWidget *parent) :
    FormCommon(parent),
    ui(new Ui::FormGasSampling)
{
    ui->setupUi(this);
}

FormGasSampling::~FormGasSampling()
{
    delete ui;
}

bool FormGasSampling::event(QEvent *event)
{
    if (event == nullptr)
    {
        return false;
    }

    return FormCommon::event(event);
}

void FormGasSampling::InitUiState()
{

}
