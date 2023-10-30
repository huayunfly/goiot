#include "form_gassampling.h"
#include "ui_form_gassampling.h"
#include <thread>

FormGasSampling::FormGasSampling(QWidget *parent, bool admin) :
    FormCommon(parent, "gassampling", QString::fromUtf8("采气/尾气"), admin),
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
    if (admin_privilege_)
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

        ui->label_TICA5105->installEventFilter(this);
        ui->label_TICA5205->installEventFilter(this);
        ui->label_TICA5501->installEventFilter(this);
        ui->label_TICA5502->installEventFilter(this);
    }
}

void FormGasSampling::SendGCStartPulseSignal(const QString& button_id)
{
    // Send a GC1 start pulse signal.
    bool ok = write_data_func_(this->objectName(), button_id, QString::number(1));
    assert(ok);
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    ok = write_data_func_(this->objectName(), button_id, QString::number(0));
    assert(ok);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

void FormGasSampling::on_pushButton_GC1_start_clicked()
{
    SendGCStartPulseSignal("button_gc1_start");
}

void FormGasSampling::on_pushButton_GC2_start_clicked()
{
    SendGCStartPulseSignal("button_gc2_start");
}
