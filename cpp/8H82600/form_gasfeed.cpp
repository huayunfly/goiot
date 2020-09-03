#include <QLabel>
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
//    ui_state_map_.emplace("svlabel", UiStateDef(RES_ON, RES_OFF, RES_OFF, 1, 0, VDeviceType::ONOFF, MeasurementUnit::DEGREE, 0));
//    ui_state_map_.emplace("svlabel_2", UiStateDef(RES_PRO_SV1, QString(), QString(), 8, 1, VDeviceType::MULTI_STATE, MeasurementUnit::NONE, 9));
//    ui_state_map_.emplace("svlabel_3", UiStateDef(RES_ON, RES_OFF, RES_OFF, 200, 0, VDeviceType::PROCESS_FLOAT, MeasurementUnit::NONE, 0));
    ui->svlabel->installEventFilter(this);
    ui->svlabel_2->installEventFilter(this);
    ui->svlabel_3->installEventFilter(this);
}
