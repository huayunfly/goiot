#include <QLabel>
#include "form_gasfeed.h"
#include "ui_form_gasfeed.h"
#include "events.h"
#include "resourcedef.h"

FormGasFeed::FormGasFeed(QWidget *parent) :
    FormCommon(parent, "gasfeed", QString::fromUtf8("进气")),
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
    ui->label_HC1020->installEventFilter(this);
    ui->label_HC1021->installEventFilter(this);
    ui->label_HC1022->installEventFilter(this);
    ui->label_HC1110->installEventFilter(this);
    ui->label_HC1120->installEventFilter(this);
    ui->label_HC1130->installEventFilter(this);
    ui->label_HC1140->installEventFilter(this);
    ui->label_HC1201->installEventFilter(this);
    ui->label_HC1202->installEventFilter(this);
    ui->label_HC1203->installEventFilter(this);
    ui->label_HC1204->installEventFilter(this);
    ui->label_HC1205->installEventFilter(this);
    ui->label_HC1206->installEventFilter(this);
    ui->label_HC1207->installEventFilter(this);
    ui->label_HC1208->installEventFilter(this);
    ui->label_HC3110->installEventFilter(this);
    ui->label_HC3120->installEventFilter(this);
    ui->label_HC1400->installEventFilter(this);
    ui->label_HC1402->installEventFilter(this);
    //
    ui->label_FICA1110->installEventFilter(this);
    ui->label_FICA1120->installEventFilter(this);
    ui->label_FICA1130->installEventFilter(this);
    ui->label_FICA1140->installEventFilter(this);
}
