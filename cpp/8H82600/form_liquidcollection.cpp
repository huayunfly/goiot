#include "form_liquidcollection.h"
#include "ui_form_liquidcollection.h"

FormLiquidCollection::FormLiquidCollection(QWidget *parent, bool admin) :
    FormCommon(parent, "liquidcollection", QString::fromUtf8("液体收集"), admin),
    ui(new Ui::FormLiquidCollection)
{
    ui->setupUi(this);
    InitUiState();
}

FormLiquidCollection::~FormLiquidCollection()
{
    delete ui;
}

bool FormLiquidCollection::event(QEvent *event)
{
    if (event == nullptr)
    {
        return false;
    }

    return FormCommon::event(event);
}

void FormLiquidCollection::InitUiState()
{
    if (admin_privilege_)
    {
        ui->label_HC6101->installEventFilter(this);
        ui->label_HC6102->installEventFilter(this);
        ui->label_HC6103->installEventFilter(this);
        ui->label_HC6104->installEventFilter(this);
        ui->label_HC6105->installEventFilter(this);
        ui->label_HC6201->installEventFilter(this);
        ui->label_HC6202->installEventFilter(this);
        ui->label_HC6203->installEventFilter(this);
        ui->label_HC6204->installEventFilter(this);
        ui->label_HC6205->installEventFilter(this);

        ui->label_TICA6501->installEventFilter(this);
        ui->label_TICA6502->installEventFilter(this);
    }
}
