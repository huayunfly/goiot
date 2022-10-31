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
    ui->label_HC4209->installEventFilter(this);
    ui->label_HC4210->installEventFilter(this);
    ui->label_HC4211->installEventFilter(this);
    ui->label_HC4212->installEventFilter(this);
    ui->label_HC4213->installEventFilter(this);
    ui->label_HC4214->installEventFilter(this);
    ui->label_HC4215->installEventFilter(this);
    ui->label_HC4216->installEventFilter(this);

    ui->label_HC4609->installEventFilter(this);
    ui->label_HC4610->installEventFilter(this);
    ui->label_HC4611->installEventFilter(this);
    ui->label_HC4612->installEventFilter(this);
    ui->label_HC4613->installEventFilter(this);
    ui->label_HC4614->installEventFilter(this);
    ui->label_HC4615->installEventFilter(this);
    ui->label_HC4616->installEventFilter(this);
    ui->label_PICA4309->installEventFilter(this);
    ui->label_PICA4310->installEventFilter(this);
    ui->label_PICA4311->installEventFilter(this);
    ui->label_PICA4312->installEventFilter(this);
    ui->label_PICA4313->installEventFilter(this);
    ui->label_PICA4314->installEventFilter(this);
    ui->label_PICA4315->installEventFilter(this);
    ui->label_PICA4316->installEventFilter(this);
    ui->label_TICA4109->installEventFilter(this);
    ui->label_TICA4110->installEventFilter(this);
    ui->label_TICA4111->installEventFilter(this);
    ui->label_TICA4112->installEventFilter(this);
    ui->label_TICA4113->installEventFilter(this);
    ui->label_TICA4114->installEventFilter(this);
    ui->label_TICA4115->installEventFilter(this);
    ui->label_TICA4116->installEventFilter(this);
    ui->label_TICA4129->installEventFilter(this);
    ui->label_TICA4130->installEventFilter(this);
    ui->label_TICA4131->installEventFilter(this);
    ui->label_TICA4132->installEventFilter(this);
    ui->label_TICA4133->installEventFilter(this);
    ui->label_TICA4134->installEventFilter(this);
    ui->label_TICA4135->installEventFilter(this);
    ui->label_TICA4136->installEventFilter(this);
    ui->label_TICA4609->installEventFilter(this);
    ui->label_TICA4610->installEventFilter(this);
    ui->label_TICA4611->installEventFilter(this);
    ui->label_TICA4612->installEventFilter(this);
    ui->label_TICA4613->installEventFilter(this);
    ui->label_TICA4614->installEventFilter(this);
    ui->label_TICA4615->installEventFilter(this);
    ui->label_TICA4616->installEventFilter(this);
    ui->label_TICA4709->installEventFilter(this);
    ui->label_TICA4710->installEventFilter(this);
    ui->label_TICA4711->installEventFilter(this);
    ui->label_TICA4712->installEventFilter(this);
    ui->label_TICA4713->installEventFilter(this);
    ui->label_TICA4714->installEventFilter(this);
    ui->label_TICA4715->installEventFilter(this);
    ui->label_TICA4716->installEventFilter(this);
}
