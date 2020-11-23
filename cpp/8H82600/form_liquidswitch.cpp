#include "form_liquidswitch.h"
#include "ui_form_liquidswitch.h"

FormLiquidSwitch::FormLiquidSwitch(QWidget *parent) :
    FormCommon(parent),
    ui(new Ui::FormLiquidSwitch)
{
    ui->setupUi(this);
    this->setObjectName("liquidswitch");
    InitUiState();
}

FormLiquidSwitch::~FormLiquidSwitch()
{
    delete ui;
}

bool FormLiquidSwitch::event(QEvent *event)
{
    if (event == nullptr)
    {
        return false;
    }

    return FormCommon::event(event);
}

void FormLiquidSwitch::InitUiState()
{
    ui->label_HC2301->installEventFilter(this);
    ui->label_HC2302->installEventFilter(this);
    ui->label_HC2303->installEventFilter(this);
    ui->label_HC2304->installEventFilter(this);
    ui->label_HC2401->installEventFilter(this);
    ui->label_HC2403->installEventFilter(this);
    ui->label_HC2404->installEventFilter(this);   
    ui->label_FICA2305->installEventFilter(this);
    ui->label_FICA2405->installEventFilter(this);
}
