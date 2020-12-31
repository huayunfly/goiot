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
    ui->label_HC3109->installEventFilter(this);
    ui->label_HC3209->installEventFilter(this);
    ui->label_HC3309->installEventFilter(this);
    ui->label_HC3409->installEventFilter(this);
    ui->label_HC3509->installEventFilter(this);
    ui->label_HC3609->installEventFilter(this);
    ui->label_HC3709->installEventFilter(this);
    ui->label_HC3809->installEventFilter(this);
    //
    ui->label_HC3110->installEventFilter(this);
    ui->label_HC3210->installEventFilter(this);
    ui->label_HC3310->installEventFilter(this);
    ui->label_HC3410->installEventFilter(this);
    ui->label_HC3510->installEventFilter(this);
    ui->label_HC3610->installEventFilter(this);
    ui->label_HC3710->installEventFilter(this);
    ui->label_HC3810->installEventFilter(this);
    //
    ui->label_HC3111->installEventFilter(this);
    ui->label_HC3211->installEventFilter(this);
    ui->label_HC3311->installEventFilter(this);
    ui->label_HC3411->installEventFilter(this);
    ui->label_HC3511->installEventFilter(this);
    ui->label_HC3611->installEventFilter(this);
    ui->label_HC3711->installEventFilter(this);
    ui->label_HC3811->installEventFilter(this);
    //
    ui->label_HC3112->installEventFilter(this);
    ui->label_HC3212->installEventFilter(this);
    ui->label_HC3312->installEventFilter(this);
    ui->label_HC3412->installEventFilter(this);
    ui->label_HC3512->installEventFilter(this);
    ui->label_HC3612->installEventFilter(this);
    ui->label_HC3712->installEventFilter(this);
    ui->label_HC3812->installEventFilter(this);
    //
    ui->label_HC3113->installEventFilter(this);
    ui->label_HC3213->installEventFilter(this);
    ui->label_HC3313->installEventFilter(this);
    ui->label_HC3413->installEventFilter(this);
    ui->label_HC3513->installEventFilter(this);
    ui->label_HC3613->installEventFilter(this);
    ui->label_HC3713->installEventFilter(this);
    ui->label_HC3813->installEventFilter(this);
    //
    ui->label_HC3114->installEventFilter(this);
    ui->label_HC3214->installEventFilter(this);
    ui->label_HC3314->installEventFilter(this);
    ui->label_HC3414->installEventFilter(this);
    ui->label_HC3514->installEventFilter(this);
    ui->label_HC3614->installEventFilter(this);
    ui->label_HC3714->installEventFilter(this);
    ui->label_HC3814->installEventFilter(this);
    //
    ui->label_HC3115->installEventFilter(this);
    ui->label_HC3215->installEventFilter(this);
    ui->label_HC3315->installEventFilter(this);
    ui->label_HC3415->installEventFilter(this);
    ui->label_HC3515->installEventFilter(this);
    ui->label_HC3615->installEventFilter(this);
    ui->label_HC3715->installEventFilter(this);
    ui->label_HC3815->installEventFilter(this);
    //
    ui->label_HC3116->installEventFilter(this);
    ui->label_HC3216->installEventFilter(this);
    ui->label_HC3316->installEventFilter(this);
    ui->label_HC3416->installEventFilter(this);
    ui->label_HC3516->installEventFilter(this);
    ui->label_HC3616->installEventFilter(this);
    ui->label_HC3716->installEventFilter(this);
    ui->label_HC3816->installEventFilter(this);
    //
    ui->label_FICA3109->installEventFilter(this);
    ui->label_FICA3110->installEventFilter(this);
    ui->label_FICA3111->installEventFilter(this);
    ui->label_FICA3112->installEventFilter(this);
    ui->label_FICA3113->installEventFilter(this);
    ui->label_FICA3114->installEventFilter(this);
    ui->label_FICA3115->installEventFilter(this);
    ui->label_FICA3116->installEventFilter(this);
}
