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
    ui->label_HC4601->installEventFilter(this);
    ui->label_HC4602->installEventFilter(this);
    ui->label_HC4603->installEventFilter(this);
    ui->label_HC4604->installEventFilter(this);
    ui->label_HC4605->installEventFilter(this);
    ui->label_HC4606->installEventFilter(this);
    ui->label_HC4607->installEventFilter(this);
    ui->label_HC4608->installEventFilter(this);
    ui->label_PICA4301->installEventFilter(this);
    ui->label_PICA4302->installEventFilter(this);
    ui->label_PICA4303->installEventFilter(this);
    ui->label_PICA4304->installEventFilter(this);
    ui->label_PICA4305->installEventFilter(this);
    ui->label_PICA4306->installEventFilter(this);
    ui->label_PICA4307->installEventFilter(this);
    ui->label_PICA4308->installEventFilter(this);
    ui->label_TICA4101->installEventFilter(this);
    ui->label_TICA4102->installEventFilter(this);
    ui->label_TICA4103->installEventFilter(this);
    ui->label_TICA4104->installEventFilter(this);
    ui->label_TICA4105->installEventFilter(this);
    ui->label_TICA4106->installEventFilter(this);
    ui->label_TICA4107->installEventFilter(this);
    ui->label_TICA4108->installEventFilter(this);
    ui->label_TICA4121->installEventFilter(this);
    ui->label_TICA4122->installEventFilter(this);
    ui->label_TICA4123->installEventFilter(this);
    ui->label_TICA4124->installEventFilter(this);
    ui->label_TICA4125->installEventFilter(this);
    ui->label_TICA4126->installEventFilter(this);
    ui->label_TICA4127->installEventFilter(this);
    ui->label_TICA4128->installEventFilter(this);
    ui->label_TICA4601->installEventFilter(this);
    ui->label_TICA4602->installEventFilter(this);
    ui->label_TICA4603->installEventFilter(this);
    ui->label_TICA4604->installEventFilter(this);
    ui->label_TICA4605->installEventFilter(this);
    ui->label_TICA4606->installEventFilter(this);
    ui->label_TICA4607->installEventFilter(this);
    ui->label_TICA4608->installEventFilter(this);
}
