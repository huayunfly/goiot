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
}
