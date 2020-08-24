#include <qwindowdefs_win.h>
#include "form_gasfeed.h"
#include "ui_form_gasfeed.h"
#include "events.h"
#include "resourcedef.h"

FormGasFeed::FormGasFeed(QWidget *parent) :
    FormCommon(parent),
    ui(new Ui::FormGasFeed)
{
    ui->setupUi(this);
    this->setObjectName("gasfeed");
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
    ui_state_map_.emplace("label", UiStateDef(RES_ON, RES_OFF, RES_OFF, 1, 0, VDeviceType::ONOFF));
}
