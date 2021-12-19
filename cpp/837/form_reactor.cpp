#include "form_reactor.h"
#include "ui_form_reactor.h"

FormReactor::FormReactor(QWidget *parent) :
    FormCommon(parent, "reactor", QString::fromUtf8("反应器")),
    ui(new Ui::FormReactor)
{
    ui->setupUi(this);
    InitUiState();
}

FormReactor::~FormReactor()
{
    delete ui;
}

bool FormReactor::event(QEvent *event)
{
    if (event == nullptr)
    {
        return false;
    }

    return FormCommon::event(event);
}

void FormReactor::InitUiState()
{
    ui->label_TICA2102->installEventFilter(this);
    ui->label_TICA2103->installEventFilter(this);
    ui->label_TICA2104->installEventFilter(this);
    ui->label_TICA2105->installEventFilter(this);
    ui->label_TICA2106->installEventFilter(this);
    ui->label_TICA2201->installEventFilter(this);
    ui->label_TICA2202->installEventFilter(this);
    ui->label_TICA2203->installEventFilter(this);

    //ui->label_TICA2501->installEventFilter(this); // 釜管路预热
    //ui->label_TICA2502->installEventFilter(this); // 釜加热
    ui->label_TICA2503->installEventFilter(this);
    ui->label_TICA2504->installEventFilter(this);
    ui->label_TICA2601->installEventFilter(this);
    ui->label_TICA2602->installEventFilter(this);
    ui->label_TICA2603->installEventFilter(this);
    ui->label_TICA2604->installEventFilter(this);

    //ui->label_TICA2101->installEventFilter(this); // 固定床入口管预热
    ui->label_TICA2401->installEventFilter(this);
    ui->label_TICA2801->installEventFilter(this);
    ui->label_TICA2404->installEventFilter(this);
    ui->label_TICA2804->installEventFilter(this);

    ui->label_HC2404->installEventFilter(this);
    ui->label_HC2804->installEventFilter(this);
}
