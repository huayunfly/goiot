#include "form_liquidsamplinga.h"
#include "ui_form_liquidsamplinga.h"

FormLiquidSamplingA::FormLiquidSamplingA(QWidget *parent) :
    FormCommon(parent),
    ui(new Ui::FormLiquidSamplingA)
{
    ui->setupUi(this);
    this->setObjectName("liquidsamplinga");
    InitUiState();
}

FormLiquidSamplingA::~FormLiquidSamplingA()
{
    delete ui;
}

bool FormLiquidSamplingA::event(QEvent *event)
{
    if (event == nullptr)
    {
        return false;
    }

    return FormCommon::event(event);
}

void FormLiquidSamplingA::InitUiState()
{
    ui->label_HC7101->installEventFilter(this);
    ui->label_HC7102->installEventFilter(this);
    ui->label_HC7201->installEventFilter(this);
    ui->label_HC7202->installEventFilter(this);
    ui->label_HC7203->installEventFilter(this);
    ui->label_HC7204->installEventFilter(this);
    ui->label_HC7205->installEventFilter(this);
    ui->label_HC7206->installEventFilter(this);
    ui->label_HC7207->installEventFilter(this);
    ui->label_HC7208->installEventFilter(this);
    ui->label_HC7209->installEventFilter(this);
    ui->label_HC7210->installEventFilter(this);
    ui->label_HC7211->installEventFilter(this);
    ui->label_HC7212->installEventFilter(this);
    ui->label_HC7213->installEventFilter(this);
    ui->label_HC7214->installEventFilter(this);
    ui->label_HC7215->installEventFilter(this);
    ui->label_HC7216->installEventFilter(this);
    ui->label_HC7217->installEventFilter(this);

    ui->label_TICA7501->installEventFilter(this);
    ui->label_TICA7502->installEventFilter(this);
}
