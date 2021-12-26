#include "form_history.h"
#include "ui_form_history.h"

FormHistory::FormHistory(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormHistory)
{
    ui->setupUi(this);
}

FormHistory::~FormHistory()
{
    delete ui;
}
