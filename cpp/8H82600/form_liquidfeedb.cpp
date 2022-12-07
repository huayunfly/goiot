#include "form_liquidfeedb.h"
#include "ui_form_liquidfeedb.h"

FormLiquidFeedB::FormLiquidFeedB(QWidget *parent, bool admin) :
    FormCommon(parent, "liquidfeedb", QString::fromUtf8("进液9-16"), admin),
    ui(new Ui::FormLiquidFeedB)
{
    ui->setupUi(this);
    InitUiState();
}

FormLiquidFeedB::~FormLiquidFeedB()
{
    delete ui;
}

bool FormLiquidFeedB::event(QEvent *event)
{
    if (event == nullptr)
    {
        return false;
    }

    return FormCommon::event(event);
}

void FormLiquidFeedB::InitUiState()
{
    if (admin_privilege_)
    {
        ui->label_HC2407->installEventFilter(this);
        ui->label_HC2408->installEventFilter(this);
        ui->label_HC2411->installEventFilter(this);
        ui->label_HC2412->installEventFilter(this);
        ui->label_HC2413->installEventFilter(this);
        ui->label_HC2414->installEventFilter(this);
        ui->label_HC2415->installEventFilter(this);
        ui->label_HC2416->installEventFilter(this);
        ui->label_HC2417->installEventFilter(this);
        ui->label_HC2418->installEventFilter(this);
        ui->label_HC2419->installEventFilter(this);
        ui->label_HC2420->installEventFilter(this);
        ui->label_HC2421->installEventFilter(this);
        ui->label_HC2422->installEventFilter(this);
        ui->label_HC2423->installEventFilter(this);
        ui->label_HC2424->installEventFilter(this);
        ui->label_HC2425->installEventFilter(this);
        ui->label_TICA2415->installEventFilter(this);
        ui->label_TICA3503->installEventFilter(this);
        ui->label_TICA3504->installEventFilter(this);
    }
}
