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
    ui->label_HC3101->installEventFilter(this);
    ui->label_HC3201->installEventFilter(this);
    ui->label_HC3301->installEventFilter(this);
    ui->label_HC3401->installEventFilter(this);
    ui->label_HC3501->installEventFilter(this);
    ui->label_HC3601->installEventFilter(this);
    ui->label_HC3701->installEventFilter(this);
    ui->label_HC3801->installEventFilter(this);
    //
    ui->label_HC3102->installEventFilter(this);
    ui->label_HC3202->installEventFilter(this);
    ui->label_HC3302->installEventFilter(this);
    ui->label_HC3402->installEventFilter(this);
    ui->label_HC3502->installEventFilter(this);
    ui->label_HC3602->installEventFilter(this);
    ui->label_HC3702->installEventFilter(this);
    ui->label_HC3802->installEventFilter(this);
    //
    ui->label_HC3103->installEventFilter(this);
    ui->label_HC3203->installEventFilter(this);
    ui->label_HC3303->installEventFilter(this);
    ui->label_HC3403->installEventFilter(this);
    ui->label_HC3503->installEventFilter(this);
    ui->label_HC3603->installEventFilter(this);
    ui->label_HC3703->installEventFilter(this);
    ui->label_HC3803->installEventFilter(this);
    //
    ui->label_HC3104->installEventFilter(this);
    ui->label_HC3204->installEventFilter(this);
    ui->label_HC3304->installEventFilter(this);
    ui->label_HC3404->installEventFilter(this);
    ui->label_HC3504->installEventFilter(this);
    ui->label_HC3604->installEventFilter(this);
    ui->label_HC3704->installEventFilter(this);
    ui->label_HC3804->installEventFilter(this);
    //
    ui->label_HC3105->installEventFilter(this);
    ui->label_HC3205->installEventFilter(this);
    ui->label_HC3305->installEventFilter(this);
    ui->label_HC3405->installEventFilter(this);
    ui->label_HC3505->installEventFilter(this);
    ui->label_HC3605->installEventFilter(this);
    ui->label_HC3705->installEventFilter(this);
    ui->label_HC3805->installEventFilter(this);
    //
    ui->label_HC3106->installEventFilter(this);
    ui->label_HC3206->installEventFilter(this);
    ui->label_HC3306->installEventFilter(this);
    ui->label_HC3406->installEventFilter(this);
    ui->label_HC3506->installEventFilter(this);
    ui->label_HC3606->installEventFilter(this);
    ui->label_HC3706->installEventFilter(this);
    ui->label_HC3806->installEventFilter(this);
    //
    ui->label_HC3107->installEventFilter(this);
    ui->label_HC3207->installEventFilter(this);
    ui->label_HC3307->installEventFilter(this);
    ui->label_HC3407->installEventFilter(this);
    ui->label_HC3507->installEventFilter(this);
    ui->label_HC3607->installEventFilter(this);
    ui->label_HC3707->installEventFilter(this);
    ui->label_HC3807->installEventFilter(this);
    //
    ui->label_HC3108->installEventFilter(this);
    ui->label_HC3208->installEventFilter(this);
    ui->label_HC3308->installEventFilter(this);
    ui->label_HC3408->installEventFilter(this);
    ui->label_HC3508->installEventFilter(this);
    ui->label_HC3608->installEventFilter(this);
    ui->label_HC3708->installEventFilter(this);
    ui->label_HC3808->installEventFilter(this);
    //
    ui->label_FICA3101->installEventFilter(this);
    ui->label_FICA3102->installEventFilter(this);
    ui->label_FICA3103->installEventFilter(this);
    ui->label_FICA3104->installEventFilter(this);
    ui->label_FICA3105->installEventFilter(this);
    ui->label_FICA3106->installEventFilter(this);
    ui->label_FICA3107->installEventFilter(this);
    ui->label_FICA3108->installEventFilter(this);
    //
    ui->label_FICA3501->installEventFilter(this);
    ui->label_FICA3502->installEventFilter(this);
    ui->label_FICA3503->installEventFilter(this);
    ui->label_FICA3504->installEventFilter(this);
    ui->label_FICA3505->installEventFilter(this);
    ui->label_FICA3506->installEventFilter(this);
    ui->label_FICA3507->installEventFilter(this);
    ui->label_FICA3508->installEventFilter(this);
}
