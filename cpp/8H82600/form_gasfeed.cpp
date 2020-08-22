#include <qwindowdefs_win.h>
#include "form_gasfeed.h"
#include "ui_form_gasfeed.h"
#include "events.h"

FormGasFeed::FormGasFeed(QWidget *parent) :
    FormCommon(parent),
    ui(new Ui::FormGasFeed)
{
    ui->setupUi(this);
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
