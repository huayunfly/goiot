#include "form_trend.h"
#include "ui_form_trend.h"

FormTrend::FormTrend(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormTrend)
{
    ui->setupUi(this);
}

FormTrend::~FormTrend()
{
    delete ui;
}
