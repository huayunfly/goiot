#include "form_liquidfeeda.h"
#include "ui_form_liquidfeeda.h"

FormLiquidFeedA::FormLiquidFeedA(QWidget *parent, bool admin) :
    FormCommon(parent, "liquidfeeda", QString::fromUtf8("进液1-8"), admin),
    ui(new Ui::FormLiquidFeedA)
{
    ui->setupUi(this);
    InitUiState();
}

FormLiquidFeedA::~FormLiquidFeedA()
{
    delete ui;
}

bool FormLiquidFeedA::event(QEvent *event)
{
    if (event == nullptr)
    {
        return false;
    }

    return FormCommon::event(event);
}

void FormLiquidFeedA::InitUiState()
{
    if (admin_privilege_)
    {
        ui->label_HC2306->installEventFilter(this);
        ui->label_HC2307->installEventFilter(this);
        ui->label_HC2308->installEventFilter(this);
        ui->label_HC2311->installEventFilter(this);
        ui->label_HC2312->installEventFilter(this);
        ui->label_HC2313->installEventFilter(this);
        ui->label_HC2314->installEventFilter(this);
        ui->label_HC2315->installEventFilter(this);
        ui->label_HC2316->installEventFilter(this);
        ui->label_HC2317->installEventFilter(this);
        ui->label_HC2318->installEventFilter(this);
        ui->label_HC2319->installEventFilter(this);
        ui->label_HC2320->installEventFilter(this);
        ui->label_HC2321->installEventFilter(this);
        ui->label_HC2322->installEventFilter(this);
        ui->label_HC2323->installEventFilter(this);
        ui->label_HC2324->installEventFilter(this);
        ui->label_HC2325->installEventFilter(this);
        ui->label_TICA2315->installEventFilter(this);
        ui->label_TICA3501->installEventFilter(this);
        ui->label_TICA3502->installEventFilter(this);
    }
}
