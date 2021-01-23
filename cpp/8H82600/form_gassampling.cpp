#include "form_gassampling.h"
#include "ui_form_gassampling.h"

FormGasSampling::FormGasSampling(QWidget *parent) :
    FormCommon(parent, "gassampling", QString::fromUtf8("采气/尾气")),
    ui(new Ui::FormGasSampling)
{
    ui->setupUi(this);
    InitUiState();
}

FormGasSampling::~FormGasSampling()
{
    delete ui;
}

bool FormGasSampling::event(QEvent *event)
{
    if (event == nullptr)
    {
        return false;
    }

    return FormCommon::event(event);
}

void FormGasSampling::InitUiState()
{
     ui->label_HC4401->installEventFilter(this); // PFC downstream /VAC valves
     ui->label_HC4402->installEventFilter(this);
     ui->label_HC4403->installEventFilter(this);

     ui->label_HC5101->installEventFilter(this);
     ui->label_HC5102->installEventFilter(this);
     ui->label_HC5103->installEventFilter(this);
     ui->label_HC5104->installEventFilter(this);
     ui->label_HC5201->installEventFilter(this);
     ui->label_HC5202->installEventFilter(this);
     ui->label_HC5203->installEventFilter(this);
     ui->label_HC5204->installEventFilter(this);

     ui->label_TICA5101->installEventFilter(this);
     ui->label_TICA5201->installEventFilter(this);
     ui->label_TICA5105->installEventFilter(this);
     ui->label_TICA5205->installEventFilter(this);
     ui->label_TICA5501->installEventFilter(this);
     ui->label_TICA5502->installEventFilter(this);
}
