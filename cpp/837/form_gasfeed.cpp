#include <QLabel>
#include "form_gasfeed.h"
#include "ui_form_gasfeed.h"
#include "events.h"
#include "resourcedef.h"

FormGasFeed::FormGasFeed(QWidget *parent) :
    FormCommon(parent, "gasfeed", QString::fromUtf8("进气单元")),
    ui(new Ui::FormGasFeed)
{
    ui->setupUi(this);
    InitUiState();
}

FormGasFeed::~FormGasFeed()
{
    delete ui;
}

bool FormGasFeed::event(QEvent *event)
{
    if (event == nullptr)
    {
        return false;
    }

    return FormCommon::event(event);
}

void FormGasFeed::InitUiState()
{
    ui->label_FICA1111->installEventFilter(this);
    ui->label_FICA1121->installEventFilter(this);
    ui->label_FICA1131->installEventFilter(this);
    ui->label_FICA1141->installEventFilter(this);
    ui->label_FICA1151->installEventFilter(this);
    ui->label_FICA1511->installEventFilter(this);
    ui->label_FICA1521->installEventFilter(this);
    ui->label_FICA1531->installEventFilter(this);
    ui->label_FICA1541->installEventFilter(this);
    ui->label_FICA1551->installEventFilter(this);
}
