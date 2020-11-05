#include "form_liquidsamplingb.h"
#include "ui_form_liquidsamplingb.h"

FormLiquidSamplingB::FormLiquidSamplingB(QWidget *parent) :
    FormCommon(parent),
    ui(new Ui::FormLiquidSamplingB)
{
    ui->setupUi(this);
    this->setObjectName("liquidsamplingb");
    InitUiState();
}

FormLiquidSamplingB::~FormLiquidSamplingB()
{
    delete ui;
}

bool FormLiquidSamplingB::event(QEvent *event)
{
    if (event == nullptr)
    {
        return false;
    }

    return FormCommon::event(event);
}

void FormLiquidSamplingB::InitUiState()
{
    ui->label_HC7301->installEventFilter(this);
    ui->label_HC7302->installEventFilter(this);
    ui->label_HC7401->installEventFilter(this);
    ui->label_HC7402->installEventFilter(this);
    ui->label_HC7403->installEventFilter(this);
    ui->label_HC7404->installEventFilter(this);
    ui->label_HC7405->installEventFilter(this);
    ui->label_HC7406->installEventFilter(this);
    ui->label_HC7407->installEventFilter(this);
    ui->label_HC7408->installEventFilter(this);
    ui->label_HC7409->installEventFilter(this);
    ui->label_HC7410->installEventFilter(this);
    ui->label_HC7411->installEventFilter(this);
    ui->label_HC7412->installEventFilter(this);
    ui->label_HC7413->installEventFilter(this);
    ui->label_HC7414->installEventFilter(this);
    ui->label_HC7415->installEventFilter(this);
    ui->label_HC7416->installEventFilter(this);
    ui->label_HC7417->installEventFilter(this);
}
