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
    ui->label_TICA101->installEventFilter(this);
    ui->label_TICA102->installEventFilter(this);
    ui->label_TICA103->installEventFilter(this);
    ui->label_TICA104->installEventFilter(this);
    ui->label_TICA105->installEventFilter(this);
    ui->label_TICA106->installEventFilter(this);
    ui->label_TICA107->installEventFilter(this);
    ui->label_TICA108->installEventFilter(this);

    ui->label_TICA201->installEventFilter(this);
    ui->label_TICA202->installEventFilter(this);
    ui->label_TICA203->installEventFilter(this);
    ui->label_TICA204->installEventFilter(this);
    ui->label_TICA205->installEventFilter(this);
    ui->label_TICA206->installEventFilter(this);
    ui->label_TICA207->installEventFilter(this);
    ui->label_TICA208->installEventFilter(this);

    ui->label_TICA301->installEventFilter(this);
    ui->label_TICA302->installEventFilter(this);
    ui->label_TICA303->installEventFilter(this);
    ui->label_TICA304->installEventFilter(this);
    ui->label_TICA305->installEventFilter(this);
    ui->label_TICA306->installEventFilter(this);
    ui->label_TICA307->installEventFilter(this);
    ui->label_TICA308->installEventFilter(this);
}
