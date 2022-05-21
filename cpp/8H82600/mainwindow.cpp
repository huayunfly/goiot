#include <iostream>
#include <functional>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "form_gasfeed.h"
#include "form_liquidswitch.h"
#include "form_liquidfeeda.h"
#include "form_liquidfeedb.h"
#include "form_cylindera.h"
#include "form_cylinderb.h"
#include "form_reactora.h"
#include "form_reactorb.h"
#include "form_gassampling.h"
#include "form_liquidcollection.h"
#include "form_liquidsamplinga.h"
#include "form_liquidsamplingb.h"
#include "form_trend.h"
#include "events.h"
#include "dialog_setvalue.h"
#include "dialog_setposition.h"
#include "dialog_onoff.h"
#include "form_motorcontrol.h"
#include "form_liquiddistributor.h"
#include "resourcedef.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui_(new Ui::MainWindow), data_manager_(QApplication::applicationDirPath().toStdString() )
{
    ui_->setupUi(this);

    // Control tab pages
    std::vector<FormCommon*> form_vec;
    form_vec.push_back(new FormGasFeed);
    form_vec.push_back(new FormLiquidSwitch);
    form_vec.push_back(new FormLiquidFeedA);
    form_vec.push_back(new FormLiquidFeedB);
    form_vec.push_back(new FormCylinderA);
    form_vec.push_back(new FormCylinderB);
    form_vec.push_back(new FormReactorA);
    form_vec.push_back(new FormReactorB);
    form_vec.push_back(new FormGasSampling);
    form_vec.push_back(new FormLiquidCollection);
    form_vec.push_back(new FormLiquidSamplingA);
    form_vec.push_back(new FormLiquidSamplingB);

    for (auto& entry : form_vec)
    {
        entry->setBaseSize(QSize(1180, 900));
        entry->RegisterReadDataFunc(std::bind(&MainWindow::ReadData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
        entry->RegisterWriteDataFunc(std::bind(&MainWindow::WriteData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        ui_->tabWidget->addTab(entry, entry->GetDisplayName());
    }

    this->setWindowState(Qt::WindowMaximized);

    // Setup Ocx
    bool ok = ui_->widget_workflow->setControl("{B6F7A42C-8939-46F0-9BC4-518C1B3036D2}"); // WorkflowComponent.WorkflowComponentCtrl.1 "{B6F7A42C-8939-46F0-9BC4-518C1B3036D2}"
    if (ok)
    {
        ui_->widget_workflow->show();
    }

    // Setup listview pages
    QStandardItemModel* model = new QStandardItemModel(this);
    QList<QStandardItem*> list;
    QStandardItem* s1 = new QStandardItem(QIcon(ICON_CONTROL), QString("控制"));
    QStandardItem* s2 = new QStandardItem(QIcon(ICON_MOTOR), QString("伺服"));
    QStandardItem* s3 = new QStandardItem(QIcon(ICON_WORKFLOW), QString("流程"));
    QStandardItem* s4 = new QStandardItem(QIcon(ICON_TREND), QString("趋势"));
    QStandardItem* s5 = new QStandardItem(QIcon(ICON_HISTORY), QString("历史"));
    QStandardItem* s6 = new QStandardItem(QIcon(ICON_DISTRIBUTOR), QString("液体分配"));
    model->appendRow(s1);
    model->appendRow(s2);
    model->appendRow(s3);
    model->appendRow(s4);
    model->appendRow(s5);
    model->appendRow(s6);
    ui_->listView->setModel(model);
    QModelIndex index_want = model->index(0, 0);
    on_listView_clicked(index_want);
    //ui_->listView->setCurrentIndex(index_want);

    // Attach motor control widget
    std::vector<FormCommon*> motor_form_vec;
    motor_form_vec.push_back(new FormMotorControl(nullptr, "motorcontrol_cylinder16", "1#-8#注射缸", MotorGroup::CYLINDER16));
    motor_form_vec.push_back(new FormMotorControl(nullptr, "motorcontrol_cylinder32", "9#-16#注射缸", MotorGroup::CYLINDER32));
    motor_form_vec.push_back(new FormMotorControl(nullptr, "motorcontrol_reactor16", "1#-16#反应器", MotorGroup::REACTOR));
    for (auto& entry : motor_form_vec)
    {
        entry->RegisterWriteDataFunc(std::bind(&MainWindow::WriteData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        ui_->widget_motor->addTab(entry, entry->GetDisplayName());
    }

    // Attach liquid distributor widget
    std::vector<FormCommon*> liquid_distributor_form_vec;
    liquid_distributor_form_vec.push_back(new FormLiquidDistributor(nullptr, "distributor_sampling", "液体采样", "QPSQL:127.0.0.1:5432:8H82600:postgres:hello@123"));
    //liquid_distributor_form_vec.push_back(new FormLiquidDistributor(nullptr, "distributor_collection", "液体收集", "QPSQL:127.0.0.1:5432:837:postgres:hello@123"));
    for (auto& entry: liquid_distributor_form_vec)
    {
        entry->RegisterWriteDataFunc(std::bind(&MainWindow::WriteData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        ui_->widget_distributor->addTab(entry, entry->GetDisplayName());
    }

    // Setup data model
    InitDataModel();

    // Setup data manager
    data_manager_.LoadJsonConfig();
    data_manager_.RegisterRefreshFunc(std::bind(&MainWindow::RefreshUi, this, std::placeholders::_1)); // lambda [this](int i){classB::handle(i);}
    data_manager_.Start();
}

MainWindow::~MainWindow()
{
    data_manager_.Stop();
    delete ui_;
}

void MainWindow::InitDataModel()
{
    // data_to_ui
    //data_model_.SetDataToUiMap("mfcpfc.4.pv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::SCCM, 1, 100, 0));
    //data_model_.SetDataToUiMap("plc.1.writebyte_channel_0", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("svlabel"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    //data_model_.SetDataToUiMap("plc.1.out1", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("svlabel_2"), RES_PRO_SV1, WidgetType::STATE, MeasurementUnit::NONE, 0, 8, 1));

    //data_model_.SetDataToUiMap("mfcpfc.4.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("svlabel_3"), RES_EMPTY, WidgetType::PROCESS_VALUE, MeasurementUnit::SCCM, 1/*decimal*/, 100/*high*/, 0/*low*/));
    // gasfeed
    data_model_.SetDataToUiMap("plc.1.smc14_1", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_HC1110"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc14_2", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_HC1120"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc14_3", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_HC1130"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc14_4", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_HC1140"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc14_5", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_HC3110"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc14_6", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_HC3120"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc14_7", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_HC1020"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc14_8", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_HC1021"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc14_9", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_HC1022"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.mvalve7_sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_HC1201"), RES_SVALVE_1, WidgetType::NONE, MeasurementUnit::NONE, 0, 4, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve7_pv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_HC1201"), RES_SVALVE_1, WidgetType::STATE, MeasurementUnit::NONE, 0, 4, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_1", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_HC1202"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_2", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_HC1203"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_3", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_HC1204"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve8_sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_HC1205"), RES_SVALVE_7, WidgetType::NONE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve8_pv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_HC1205"), RES_SVALVE_7, WidgetType::STATE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve9_sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_HC1206"), RES_SVALVE_7, WidgetType::NONE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve9_pv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_HC1206"), RES_SVALVE_7, WidgetType::STATE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve10_sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_HC1207"), RES_SVALVE_7, WidgetType::NONE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve10_pv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_HC1207"), RES_SVALVE_7, WidgetType::STATE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve11_sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_HC1208"), RES_SVALVE_7, WidgetType::NONE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve11_pv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_HC1208"), RES_SVALVE_7, WidgetType::STATE, MeasurementUnit::NONE, 0, 8, 1));
    //
    data_model_.SetDataToUiMap("mfcpfc.1.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_FICA1110"), RES_MFC, WidgetType::PROCESS_VALUE, MeasurementUnit::SCCM, 0, 10000, 0));
    data_model_.SetDataToUiMap("mfcpfc.2.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_FICA1120"), RES_MFC, WidgetType::PROCESS_VALUE, MeasurementUnit::SCCM, 0, 1000, 0));
    data_model_.SetDataToUiMap("mfcpfc.3.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_FICA1130"), RES_MFC, WidgetType::PROCESS_VALUE, MeasurementUnit::SCCM, 0, 1000, 0));
    data_model_.SetDataToUiMap("mfcpfc.4.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_FICA1140"), RES_MFC, WidgetType::PROCESS_VALUE, MeasurementUnit::SCCM, 0, 10000, 0));
    data_model_.SetDataToUiMap("mfcpfc.1.pv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_FICA1110"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::SCCM, 1, 10000, 0));
    data_model_.SetDataToUiMap("mfcpfc.2.pv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_FICA1120"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::SCCM, 1, 1000, 0));
    data_model_.SetDataToUiMap("mfcpfc.3.pv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_FICA1130"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::SCCM, 1, 1000, 0));
    data_model_.SetDataToUiMap("mfcpfc.4.pv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_FICA1140"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::SCCM, 1, 10000, 0));

    data_model_.SetDataToUiMap("plc.1.pg_1", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_PIA1110"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::BARA, 1, 50, 0));
    data_model_.SetDataToUiMap("plc.1.pg_2", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_PIA1120"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::BARA, 1, 50, 0));
    data_model_.SetDataToUiMap("plc.1.pg_3", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_PIA1130"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::BARA, 1, 50, 0));
    data_model_.SetDataToUiMap("plc.1.pg_4", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_PIA1140"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::BARA, 1, 50, 0));
    data_model_.SetDataToUiMap("plc.1.pg_5", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_PIA1010"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::BARA, 1, 10, 0));
    data_model_.SetDataToUiMap("plc.1.pg_6", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_PIA1020"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::BARA, 1, 10, 0));
    data_model_.SetDataToUiMap("plc.1.pg_7", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_PIA3110"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::BARA, 1, 5, 0));
    data_model_.SetDataToUiMap("plc.1.pg_8", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_PIA3120"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::BARA, 1, 5, 0));
    // gasfeed - pump,electric valve
    data_model_.SetDataToUiMap("plc.1.dq3_2", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_HC1400"), RES_MECHANICAL_PUMP, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.dq3_7", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_HC1402"), RES_VALVE_ELECTRIC, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));

    // liquidswitch
    data_model_.SetDataToUiMap("plc.1.mvalve1_sv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_HC2301"), RES_SVALVE_1, WidgetType::NONE, MeasurementUnit::NONE, 0, 4, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve2_sv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_HC2303"), RES_SVALVE_2, WidgetType::NONE, MeasurementUnit::NONE, 0, 4, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve3_sv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_HC2304"), RES_SVALVE_8, WidgetType::NONE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve4_sv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_HC2401"), RES_SVALVE_1, WidgetType::NONE, MeasurementUnit::NONE, 0, 4, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve5_sv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_HC2403"), RES_SVALVE_2, WidgetType::NONE, MeasurementUnit::NONE, 0, 4, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve6_sv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_HC2404"), RES_SVALVE_8, WidgetType::NONE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve1_pv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_HC2301"), RES_SVALVE_1, WidgetType::STATE, MeasurementUnit::NONE, 0, 4, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_1", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_HC2302"), RES_SVALVE_5, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve2_pv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_HC2303"), RES_SVALVE_2, WidgetType::STATE, MeasurementUnit::NONE, 0, 4, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve3_pv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_HC2304"), RES_SVALVE_8, WidgetType::STATE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve4_pv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_HC2401"), RES_SVALVE_1, WidgetType::STATE, MeasurementUnit::NONE, 0, 4, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve5_pv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_HC2403"), RES_SVALVE_2, WidgetType::STATE, MeasurementUnit::NONE, 0, 4, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve6_pv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_HC2404"), RES_SVALVE_8, WidgetType::STATE, MeasurementUnit::NONE, 0, 8, 1));
    // liquidswitch - pump
    data_model_.SetDataToUiMap("plc.1.pump1_sv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_FICA2305"), RES_WATER_PUMP, WidgetType::PROCESS_VALUE, MeasurementUnit::MLM, 2, 100, 0));
    data_model_.SetDataToUiMap("plc.1.pump2_sv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_FICA2405"), RES_WATER_PUMP, WidgetType::PROCESS_VALUE, MeasurementUnit::MLM, 2, 100, 0));
    data_model_.SetDataToUiMap("plc.1.pump1_pv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("textEdit_FICA2305"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::MLM, 2, 100, 0));
    data_model_.SetDataToUiMap("plc.1.pump2_pv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("textEdit_FICA2405"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::MLM, 2, 100, 0));
    data_model_.SetDataToUiMap("plc.1.pump1_pressure", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("textEdit_PIA2305"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::MPA, 1, 10, 0));
    data_model_.SetDataToUiMap("plc.1.pump2_pressure", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("textEdit_PIA2405"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::MPA, 1, 10, 0));

    // liquidfeed
    data_model_.SetDataToUiMap("plc.1.smc9_2", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_HC2306"), RES_SVALVE_5, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_3", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_HC2307"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_4", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_HC2308"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_5", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_HC2311"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_6", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_HC2312"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_7", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_HC2313"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_8", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_HC2314"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_10", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_HC2315"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_11", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_HC2316"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_12", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_HC2317"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_12", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_HC2318"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_13", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_HC2319"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_14", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_HC2320"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_15", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_HC2321"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_17", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_HC2322"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_18", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_HC2323"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_19", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_HC2324"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_20", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_HC2325"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    //
    data_model_.SetDataToUiMap("plc.1.smc10_4", UiInfo(ui_->tabWidget->widget(3), QString::fromUtf8("label_HC2407"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_5", UiInfo(ui_->tabWidget->widget(3), QString::fromUtf8("label_HC2408"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_6", UiInfo(ui_->tabWidget->widget(3), QString::fromUtf8("label_HC2411"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_7", UiInfo(ui_->tabWidget->widget(3), QString::fromUtf8("label_HC2412"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_8", UiInfo(ui_->tabWidget->widget(3), QString::fromUtf8("label_HC2413"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_9", UiInfo(ui_->tabWidget->widget(3), QString::fromUtf8("label_HC2414"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_9", UiInfo(ui_->tabWidget->widget(3), QString::fromUtf8("label_HC2415"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_10", UiInfo(ui_->tabWidget->widget(3), QString::fromUtf8("label_HC2416"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_11", UiInfo(ui_->tabWidget->widget(3), QString::fromUtf8("label_HC2417"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_13", UiInfo(ui_->tabWidget->widget(3), QString::fromUtf8("label_HC2418"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_14", UiInfo(ui_->tabWidget->widget(3), QString::fromUtf8("label_HC2419"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_15", UiInfo(ui_->tabWidget->widget(3), QString::fromUtf8("label_HC2420"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_16", UiInfo(ui_->tabWidget->widget(3), QString::fromUtf8("label_HC2421"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_16", UiInfo(ui_->tabWidget->widget(3), QString::fromUtf8("label_HC2422"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_17", UiInfo(ui_->tabWidget->widget(3), QString::fromUtf8("label_HC2423"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_18", UiInfo(ui_->tabWidget->widget(3), QString::fromUtf8("label_HC2424"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_19", UiInfo(ui_->tabWidget->widget(3), QString::fromUtf8("label_HC2425"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    // EO/PO channel 1-8
    data_model_.SetDataToUiMap("plc.1.smc1_1", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3101"), RES_SVALVE_3, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc1_2", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3201"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc1_3", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3301"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc1_4", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3401"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc1_5", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3501"), RES_SVALVE_3, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc1_6", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3601"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc1_7", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3701"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc1_8", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3801"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc1_9", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3102"), RES_SVALVE_3, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc1_10", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3202"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc1_11", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3302"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc1_12", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3402"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc1_13", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3502"), RES_SVALVE_3, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc1_14", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3602"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc1_15", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3702"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc1_16", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3802"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    //
    data_model_.SetDataToUiMap("plc.1.smc2_1", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3103"), RES_SVALVE_3, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc2_2", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3203"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc2_3", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3303"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc2_4", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3403"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc2_5", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3503"), RES_SVALVE_3, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc2_6", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3603"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc2_7", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3703"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc2_8", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3803"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc2_9", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3104"), RES_SVALVE_3, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc2_10", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3204"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc2_11", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3304"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc2_12", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3404"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc2_13", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3504"), RES_SVALVE_3, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc2_14", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3604"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc2_15", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3704"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc2_16", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3804"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    //
    data_model_.SetDataToUiMap("plc.1.smc3_1", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3105"), RES_SVALVE_3, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc3_2", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3205"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc3_3", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3305"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc3_4", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3405"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc3_5", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3505"), RES_SVALVE_3, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc3_6", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3605"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc3_7", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3705"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc3_8", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3805"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc3_9", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3106"), RES_SVALVE_3, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc3_10", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3206"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc3_11", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3306"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc3_12", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3406"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc3_13", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3506"), RES_SVALVE_3, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc3_14", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3606"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc3_15", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3706"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc3_16", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3806"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    //
    data_model_.SetDataToUiMap("plc.1.smc4_1", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3107"), RES_SVALVE_3, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc4_2", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3207"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc4_3", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3307"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc4_4", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3407"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc4_5", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3507"), RES_SVALVE_3, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc4_6", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3607"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc4_7", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3707"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc4_8", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3807"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc4_9", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3108"), RES_SVALVE_3, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc4_10", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3208"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc4_11", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3308"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc4_12", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3408"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc4_13", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3508"), RES_SVALVE_3, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc4_14", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3608"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc4_15", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3708"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc4_16", UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_HC3808"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    // EO/PO cylinder
    // block busy PO
    data_model_.SetDataToUiMap("cylinder16.1.busy", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3101"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.3.busy", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3102"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.5.busy", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3103"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.7.busy", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3104"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.9.busy", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3105"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.11.busy", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3106"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                      UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.13.busy", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3107"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                      UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.15.busy", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3108"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                      UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    // break_off PO
    data_model_.SetDataToUiMap("cylinder16.1.brk_off", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3101_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                        UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.3.brk_off", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3102_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                        UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.5.brk_off", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3103_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                        UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.7.brk_off", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3104_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                        UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.9.brk_off", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3105_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                        UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.11.brk_off", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3106_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                         UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.13.brk_off", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3107_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                         UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.15.brk_off", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3108_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                         UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    // alarm PO
    data_model_.SetDataToUiMap("cylinder16.1.alm", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3101_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                    UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.3.alm", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3102_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                    UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.5.alm", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3103_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                    UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.7.alm", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3104_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                    UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.9.alm", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3105_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                    UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.11.alm", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3106_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.13.alm", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3107_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.15.alm", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3108_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    // block busy EO
    data_model_.SetDataToUiMap("cylinder16.2.busy", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3501"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.4.busy", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3502"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.6.busy", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3503"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.8.busy", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3504"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.10.busy", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3505"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                      UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.12.busy", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3506"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                      UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.14.busy", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3507"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                      UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.16.busy", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3508"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                      UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    // break_off EO
    data_model_.SetDataToUiMap("cylinder16.2.brk_off", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3501_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                        UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.4.brk_off", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3502_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                        UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.6.brk_off", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3503_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                        UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.8.brk_off", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3504_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                        UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.10.brk_off", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3505_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                         UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.12.brk_off", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3506_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                         UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.14.brk_off", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3507_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                         UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.16.brk_off", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3508_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                         UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    // alarm EO
    data_model_.SetDataToUiMap("cylinder16.2.alm", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3501_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                    UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.4.alm", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3502_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                    UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.6.alm", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3503_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                    UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.8.alm", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3504_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                    UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.10.alm", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3505_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.12.alm", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3506_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.14.alm", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3507_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder16.16.alm", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("label_FICA3508_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    // multi_turn PO
    data_model_.SetDataToUiMap("cylinder16.1.multi_turn", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("textEdit_FICA3101"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0),
                                                           UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0)});
    data_model_.SetDataToUiMap("cylinder16.3.multi_turn", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("textEdit_FICA3102"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0),
                                                           UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0)});
    data_model_.SetDataToUiMap("cylinder16.5.multi_turn", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("textEdit_FICA3103"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0),
                                                           UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0)});
    data_model_.SetDataToUiMap("cylinder16.7.multi_turn", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("textEdit_FICA3104"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0),
                                                           UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0)});
    data_model_.SetDataToUiMap("cylinder16.9.multi_turn", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("textEdit_FICA3105"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0),
                                                           UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0)});
    data_model_.SetDataToUiMap("cylinder16.11.multi_turn", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("textEdit_FICA3106"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0),
                                                            UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0)});
    data_model_.SetDataToUiMap("cylinder16.13.multi_turn", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("textEdit_FICA3107"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0),
                                                            UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0)});
    data_model_.SetDataToUiMap("cylinder16.15.multi_turn", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("textEdit_FICA3108"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0),
                                                            UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0)});
    // multi_turn EO
    data_model_.SetDataToUiMap("cylinder16.2.multi_turn", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("textEdit_FICA3501"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0),
                                                           UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0)});
    data_model_.SetDataToUiMap("cylinder16.4.multi_turn", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("textEdit_FICA3502"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0),
                                                           UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0)});
    data_model_.SetDataToUiMap("cylinder16.6.multi_turn", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("textEdit_FICA3503"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0),
                                                           UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0)});
    data_model_.SetDataToUiMap("cylinder16.8.multi_turn", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("textEdit_FICA3504"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0),
                                                           UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0)});
    data_model_.SetDataToUiMap("cylinder16.10.multi_turn", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("textEdit_FICA3505"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0),
                                                            UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0)});
    data_model_.SetDataToUiMap("cylinder16.12.multi_turn", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("textEdit_FICA3506"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0),
                                                            UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0)});
    data_model_.SetDataToUiMap("cylinder16.14.multi_turn", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("textEdit_FICA3507"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0),
                                                            UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0)});
    data_model_.SetDataToUiMap("cylinder16.16.multi_turn", {UiInfo(ui_->tabWidget->widget(4), QString::fromUtf8("textEdit_FICA3508"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0),
                                                            UiInfo(ui_->widget_motor->widget(0), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0)});
    // EO/PO channel 9-16
    data_model_.SetDataToUiMap("plc.1.smc5_1", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3109"), RES_SVALVE_3, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc5_2", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3209"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc5_3", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3309"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc5_4", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3409"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc5_5", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3509"), RES_SVALVE_3, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc5_6", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3609"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc5_7", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3709"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc5_8", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3809"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc5_9", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3110"), RES_SVALVE_3, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc5_10", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3210"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc5_11", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3310"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc5_12", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3410"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc5_13", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3510"), RES_SVALVE_3, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc5_14", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3610"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc5_15", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3710"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc5_16", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3810"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    //
    data_model_.SetDataToUiMap("plc.1.smc6_1", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3111"), RES_SVALVE_3, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc6_2", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3211"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc6_3", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3311"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc6_4", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3411"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc6_5", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3511"), RES_SVALVE_3, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc6_6", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3611"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc6_7", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3711"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc6_8", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3811"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc6_9", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3112"), RES_SVALVE_3, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc6_10", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3212"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc6_11", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3312"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc6_12", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3412"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc6_13", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3512"), RES_SVALVE_3, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc6_14", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3612"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc6_15", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3712"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc6_16", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3812"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    //
    data_model_.SetDataToUiMap("plc.1.smc7_1", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3113"), RES_SVALVE_3, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc7_2", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3213"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc7_3", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3313"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc7_4", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3413"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc7_5", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3513"), RES_SVALVE_3, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc7_6", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3613"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc7_7", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3713"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc7_8", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3813"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc7_9", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3114"), RES_SVALVE_3, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc7_10", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3214"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc7_11", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3314"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc7_12", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3414"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc7_13", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3514"), RES_SVALVE_3, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc7_14", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3614"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc7_15", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3714"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc7_16", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3814"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    //
    data_model_.SetDataToUiMap("plc.1.smc8_1", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3115"), RES_SVALVE_3, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc8_2", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3215"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc8_3", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3315"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc8_4", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3415"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc8_5", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3515"), RES_SVALVE_3, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc8_6", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3615"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc8_7", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3715"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc8_8", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3815"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc8_9", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3116"), RES_SVALVE_3, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc8_10", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3216"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc8_11", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3316"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc8_12", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3416"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc8_13", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3516"), RES_SVALVE_3, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc8_14", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3616"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc8_15", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3716"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc8_16", UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_HC3816"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    // EO/PO cylinder
    data_model_.SetDataToUiMap("cylinder32.1.busy", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3109"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.3.busy", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3110"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.5.busy", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3111"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.7.busy", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3112"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.9.busy", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3113"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.11.busy", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3114"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                      UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.13.busy", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3115"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                      UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.15.busy", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3116"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                      UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    // break_off PO
    data_model_.SetDataToUiMap("cylinder32.1.brk_off", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3109_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                        UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.3.brk_off", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3110_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                        UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.5.brk_off", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3111_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                        UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.7.brk_off", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3112_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                        UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.9.brk_off", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3113_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                        UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.11.brk_off", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3114_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                         UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.13.brk_off", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3115_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                         UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.15.brk_off", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3116_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                         UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    // alarm PO
    data_model_.SetDataToUiMap("cylinder32.1.alm", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3109_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                    UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.3.alm", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3110_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                    UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.5.alm", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3111_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                    UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.7.alm", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3112_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                    UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.9.alm", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3113_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                    UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.11.alm", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3114_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.13.alm", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3115_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.15.alm", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3116_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    // block busy PO
    data_model_.SetDataToUiMap("cylinder32.2.busy", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3509"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.4.busy", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3510"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.6.busy", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3511"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.8.busy", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3512"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.10.busy", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3513"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                      UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.12.busy", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3514"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                      UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.14.busy", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3515"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                      UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.16.busy", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3516"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                      UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    // break_off EO
    data_model_.SetDataToUiMap("cylinder32.2.brk_off", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3509_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                        UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.4.brk_off", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3510_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                        UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.6.brk_off", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3511_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                        UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.8.brk_off", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3512_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                        UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.10.brk_off", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3513_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                         UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.12.brk_off", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3514_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                         UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.14.brk_off", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3515_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                         UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.16.brk_off", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3516_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                         UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    // alarm EO
    data_model_.SetDataToUiMap("cylinder32.2.alm", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3509_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                                                      UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.4.alm", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3510_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                                                      UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.6.alm", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3511_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                                                      UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.8.alm", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3512_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                                                      UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.10.alm", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3513_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                                                       UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.12.alm", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3514_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                                                       UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.14.alm", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3515_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                                                       UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("cylinder32.16.alm", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("label_FICA3516_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    // multi_turn PO
    data_model_.SetDataToUiMap("cylinder32.1.multi_turn", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("textEdit_FICA3109"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0),
                                                           UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0)});
    data_model_.SetDataToUiMap("cylinder32.3.multi_turn", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("textEdit_FICA3110"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0),
                                                           UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0)});
    data_model_.SetDataToUiMap("cylinder32.5.multi_turn", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("textEdit_FICA3111"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0),
                                                           UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0)});
    data_model_.SetDataToUiMap("cylinder32.7.multi_turn", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("textEdit_FICA3112"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0),
                                                           UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0)});
    data_model_.SetDataToUiMap("cylinder32.9.multi_turn", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("textEdit_FICA3113"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0),
                                                           UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0)});
    data_model_.SetDataToUiMap("cylinder32.11.multi_turn", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("textEdit_FICA3114"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0),
                                                            UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0)});
    data_model_.SetDataToUiMap("cylinder32.13.multi_turn", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("textEdit_FICA3115"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0),
                                                            UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0)});
    data_model_.SetDataToUiMap("cylinder32.15.multi_turn", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("textEdit_FICA3116"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0),
                                                            UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0)});
    // multi_turn EO
    data_model_.SetDataToUiMap("cylinder32.2.multi_turn", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("textEdit_FICA3509"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0),
                                                           UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0)});
    data_model_.SetDataToUiMap("cylinder32.4.multi_turn", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("textEdit_FICA3510"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0),
                                                           UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0)});
    data_model_.SetDataToUiMap("cylinder32.6.multi_turn", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("textEdit_FICA3511"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0),
                                                           UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0)});
    data_model_.SetDataToUiMap("cylinder32.8.multi_turn", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("textEdit_FICA3512"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0),
                                                           UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0)});
    data_model_.SetDataToUiMap("cylinder32.10.multi_turn", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("textEdit_FICA3513"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0),
                                                            UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0)});
    data_model_.SetDataToUiMap("cylinder32.12.multi_turn", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("textEdit_FICA3514"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0),
                                                            UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0)});
    data_model_.SetDataToUiMap("cylinder32.14.multi_turn", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("textEdit_FICA3515"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0),
                                                            UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0)});
    data_model_.SetDataToUiMap("cylinder32.16.multi_turn", {UiInfo(ui_->tabWidget->widget(5), QString::fromUtf8("textEdit_FICA3516"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0),
                                                            UiInfo(ui_->widget_motor->widget(1), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::ML, 2, 100, 0)});
    // reactorA
    data_model_.SetDataToUiMap("plc.1.smc13_1", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4601"), RES_VALVE_GAS_LEFT, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc13_2", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4602"), RES_VALVE_GAS_LEFT, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc13_3", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4603"), RES_VALVE_GAS_LEFT, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc13_4", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4604"), RES_VALVE_GAS_LEFT, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc14_13", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4605"), RES_VALVE_GAS_LEFT, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc14_14", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4606"), RES_VALVE_GAS_LEFT, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc14_15", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4607"), RES_VALVE_GAS_LEFT, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc14_16", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4608"), RES_VALVE_GAS_LEFT, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    // reactor-liquid collction cylinder, using proximity switch signal 接近开关
    data_model_.SetDataToUiMap("plc.1.di3_1", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_ZI4621"), RES_CYLINDER_DOWN, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.di3_2", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_ZI4622"), RES_CYLINDER_DOWN, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.di3_3", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_ZI4623"), RES_CYLINDER_DOWN, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.di3_4", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_ZI4624"), RES_CYLINDER_DOWN, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.di3_5", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_ZI4625"), RES_CYLINDER_DOWN, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.di3_6", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_ZI4626"), RES_CYLINDER_DOWN, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.di3_7", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_ZI4627"), RES_CYLINDER_DOWN, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.di3_8", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_ZI4628"), RES_CYLINDER_DOWN, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    // reactorA-support cylinder
    data_model_.SetDataToUiMap("plc.1.smc1_19", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4201"), RES_CYLINDER_UP, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc1_20", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4202"), RES_CYLINDER_UP, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc2_19", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4203"), RES_CYLINDER_UP, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc2_20", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4204"), RES_CYLINDER_UP, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc3_19", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4205"), RES_CYLINDER_UP, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc3_20", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4206"), RES_CYLINDER_UP, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc4_19", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4207"), RES_CYLINDER_UP, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc4_20", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4208"), RES_CYLINDER_UP, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    // reactorA-PFC
    data_model_.SetDataToUiMap("mfcpfc.11.sv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_PICA4301"), RES_PFC, WidgetType::PROCESS_VALUE, MeasurementUnit::BARA, 1, 50, 0));
    data_model_.SetDataToUiMap("mfcpfc.12.sv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_PICA4302"), RES_PFC, WidgetType::PROCESS_VALUE, MeasurementUnit::BARA, 1, 50, 0));
    data_model_.SetDataToUiMap("mfcpfc.13.sv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_PICA4303"), RES_PFC, WidgetType::PROCESS_VALUE, MeasurementUnit::BARA, 1, 50, 0));
    data_model_.SetDataToUiMap("mfcpfc.14.sv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_PICA4304"), RES_PFC, WidgetType::PROCESS_VALUE, MeasurementUnit::BARA, 1, 50, 0));
    data_model_.SetDataToUiMap("mfcpfc.15.sv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_PICA4305"), RES_PFC, WidgetType::PROCESS_VALUE, MeasurementUnit::BARA, 1, 50, 0));
    data_model_.SetDataToUiMap("mfcpfc.16.sv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_PICA4306"), RES_PFC, WidgetType::PROCESS_VALUE, MeasurementUnit::BARA, 1, 50, 0));
    data_model_.SetDataToUiMap("mfcpfc.17.sv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_PICA4307"), RES_PFC, WidgetType::PROCESS_VALUE, MeasurementUnit::BARA, 1, 50, 0));
    data_model_.SetDataToUiMap("mfcpfc.18.sv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_PICA4308"), RES_PFC, WidgetType::PROCESS_VALUE, MeasurementUnit::BARA, 1, 50, 0));
    data_model_.SetDataToUiMap("mfcpfc.11.pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_PICA4301"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::BARA, 1, 50, 0));
    data_model_.SetDataToUiMap("mfcpfc.12.pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_PICA4302"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::BARA, 1, 50, 0));
    data_model_.SetDataToUiMap("mfcpfc.13.pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_PICA4303"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::BARA, 1, 50, 0));
    data_model_.SetDataToUiMap("mfcpfc.14.pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_PICA4304"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::BARA, 1, 50, 0));
    data_model_.SetDataToUiMap("mfcpfc.15.pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_PICA4305"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::BARA, 1, 50, 0));
    data_model_.SetDataToUiMap("mfcpfc.16.pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_PICA4306"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::BARA, 1, 50, 0));
    data_model_.SetDataToUiMap("mfcpfc.17.pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_PICA4307"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::BARA, 1, 50, 0));
    data_model_.SetDataToUiMap("mfcpfc.18.pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_PICA4308"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::BARA, 1, 50, 0));
    // reactorA-bottom TC
    data_model_.SetDataToUiMap("plc.1.temp1_sv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_TICA4101"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp2_sv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_TICA4102"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp3_sv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_TICA4103"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp4_sv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_TICA4104"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp5_sv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_TICA4105"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp6_sv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_TICA4106"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp7_sv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_TICA4107"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp8_sv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_TICA4108"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp1_pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_TICA4101"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp2_pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_TICA4102"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp3_pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_TICA4103"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp4_pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_TICA4104"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp5_pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_TICA4105"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp6_pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_TICA4106"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp7_pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_TICA4107"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp8_pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_TICA4108"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    // reactorA-upper TC
    data_model_.SetDataToUiMap("plc.1.temp17_sv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_TICA4121"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp18_sv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_TICA4122"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp19_sv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_TICA4123"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp20_sv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_TICA4124"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp21_sv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_TICA4125"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp22_sv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_TICA4126"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp23_sv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_TICA4127"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp24_sv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_TICA4128"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp17_pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_TICA4121"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp18_pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_TICA4122"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp19_pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_TICA4123"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp20_pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_TICA4124"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp21_pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_TICA4125"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp22_pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_TICA4126"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp23_pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_TICA4127"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp24_pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_TICA4128"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    // reactorA-inner TI
    data_model_.SetDataToUiMap("plc.1.temp65_pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_TIA4141"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp66_pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_TIA4142"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp67_pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_TIA4143"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp68_pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_TIA4144"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp69_pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_TIA4145"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp70_pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_TIA4146"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp71_pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_TIA4147"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp72_pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_TIA4148"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    // reactorA-pipe TC
    data_model_.SetDataToUiMap("plc.1.temp33_sv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_TICA4601"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp34_sv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_TICA4602"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp35_sv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_TICA4603"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp36_sv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_TICA4604"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp37_sv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_TICA4605"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp38_sv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_TICA4606"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp39_sv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_TICA4607"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp40_sv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_TICA4608"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp33_pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_TICA4601"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp34_pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_TICA4602"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp35_pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_TICA4603"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp36_pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_TICA4604"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp37_pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_TICA4605"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp38_pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_TICA4606"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp39_pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_TICA4607"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp40_pv", UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_TICA4608"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 100, 0));
    // reactorA-block busy
    data_model_.SetDataToUiMap("reactor16.1.busy", {UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4101"), RES_MOTOR, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_MOTOR, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.2.busy", {UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4102"), RES_MOTOR, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_MOTOR, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.3.busy", {UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4103"), RES_MOTOR, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_MOTOR, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.4.busy", {UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4104"), RES_MOTOR, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_MOTOR, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.5.busy", {UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4105"), RES_MOTOR, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_MOTOR, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.6.busy", {UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4106"), RES_MOTOR, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                      UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_MOTOR, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.7.busy", {UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4107"), RES_MOTOR, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                      UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_MOTOR, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.8.busy", {UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4108"), RES_MOTOR, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                      UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_MOTOR, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    // reactorA-break_off
    data_model_.SetDataToUiMap("reactor16.1.brk_off", {UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4101_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                        UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.2.brk_off", {UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4102_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                        UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.3.brk_off", {UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4103_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                        UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.4.brk_off", {UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4104_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                        UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.5.brk_off", {UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4105_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                        UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.6.brk_off", {UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4106_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                         UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.7.brk_off", {UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4107_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                         UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.8.brk_off", {UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4108_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                         UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    // reactorA-alarm
    data_model_.SetDataToUiMap("reactor16.1.alm", {UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4101_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                    UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.2.alm", {UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4102_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                    UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.3.alm", {UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4103_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                    UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.4.alm", {UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4104_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                    UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.5.alm", {UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4105_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                    UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.6.alm", {UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4106_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.7.alm", {UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4107_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.8.alm", {UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("label_HC4108_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    // reactorA-speed
    data_model_.SetDataToUiMap("reactor16.1.speed", {UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_HC4101"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::RPM, 0, 600, 0),
                                                           UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::RPM, 0, 600, 0)});
    data_model_.SetDataToUiMap("reactor16.2.speed", {UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_HC4102"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::RPM, 0, 600, 0),
                                                           UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::RPM, 0, 600, 0)});
    data_model_.SetDataToUiMap("reactor16.3.speed", {UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_HC4103"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::RPM, 0, 600, 0),
                                                           UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::RPM, 0, 600, 0)});
    data_model_.SetDataToUiMap("reactor16.4.speed", {UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_HC4104"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::RPM, 0, 600, 0),
                                                           UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::RPM, 0, 600, 0)});
    data_model_.SetDataToUiMap("reactor16.5.speed", {UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_HC4105"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::RPM, 0, 600, 0),
                                                           UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::RPM, 0, 600, 0)});
    data_model_.SetDataToUiMap("reactor16.6.speed", {UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_HC4106"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::RPM, 0, 600, 0),
                                                            UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::RPM, 0, 600, 0)});
    data_model_.SetDataToUiMap("reactor16.7.speed", {UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_HC4107"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::RPM, 0, 600, 0),
                                                            UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::RPM, 0, 600, 0)});
    data_model_.SetDataToUiMap("reactor16.8.speed", {UiInfo(ui_->tabWidget->widget(6), QString::fromUtf8("textEdit_HC4108"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::RPM, 0, 600, 0),
                                                            UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::RPM, 0, 600, 0)});
    // reactorB
    data_model_.SetDataToUiMap("plc.1.smc14_17", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4609"), RES_VALVE_GAS_LEFT, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc14_18", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4610"), RES_VALVE_GAS_LEFT, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc14_19", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4611"), RES_VALVE_GAS_LEFT, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc14_20", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4612"), RES_VALVE_GAS_LEFT, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc13_5", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4613"), RES_VALVE_GAS_LEFT, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc13_6", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4614"), RES_VALVE_GAS_LEFT, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc13_7", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4615"), RES_VALVE_GAS_LEFT, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc13_8", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4616"), RES_VALVE_GAS_LEFT, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    // reactorB-liqudi collction cylinder, using proximity switch signal
    data_model_.SetDataToUiMap("plc.1.di3_9", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_ZI4629"), RES_CYLINDER_DOWN, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.di3_10", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_ZI4630"), RES_CYLINDER_DOWN, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.di3_11", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_ZI4631"), RES_CYLINDER_DOWN, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.di3_12", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_ZI4632"), RES_CYLINDER_DOWN, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.di3_13", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_ZI4633"), RES_CYLINDER_DOWN, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.di3_14", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_ZI4634"), RES_CYLINDER_DOWN, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.di3_15", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_ZI4635"), RES_CYLINDER_DOWN, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.di3_16", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_ZI4636"), RES_CYLINDER_DOWN, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    // reactorB-support cylinder
    data_model_.SetDataToUiMap("plc.1.smc5_19", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4209"), RES_CYLINDER_UP, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc5_20", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4210"), RES_CYLINDER_UP, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc6_19", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4211"), RES_CYLINDER_UP, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc6_20", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4212"), RES_CYLINDER_UP, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc7_19", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4213"), RES_CYLINDER_UP, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc7_20", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4214"), RES_CYLINDER_UP, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc8_19", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4215"), RES_CYLINDER_UP, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc8_20", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4216"), RES_CYLINDER_UP, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    // reactorB-PFC
    data_model_.SetDataToUiMap("mfcpfc.19.sv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_PICA4309"), RES_PFC, WidgetType::PROCESS_VALUE, MeasurementUnit::BARA, 1, 50, 0));
    data_model_.SetDataToUiMap("mfcpfc.20.sv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_PICA4310"), RES_PFC, WidgetType::PROCESS_VALUE, MeasurementUnit::BARA, 1, 50, 0));
    data_model_.SetDataToUiMap("mfcpfc.21.sv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_PICA4311"), RES_PFC, WidgetType::PROCESS_VALUE, MeasurementUnit::BARA, 1, 50, 0));
    data_model_.SetDataToUiMap("mfcpfc.22.sv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_PICA4312"), RES_PFC, WidgetType::PROCESS_VALUE, MeasurementUnit::BARA, 1, 50, 0));
    data_model_.SetDataToUiMap("mfcpfc.23.sv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_PICA4313"), RES_PFC, WidgetType::PROCESS_VALUE, MeasurementUnit::BARA, 1, 50, 0));
    data_model_.SetDataToUiMap("mfcpfc.24.sv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_PICA4314"), RES_PFC, WidgetType::PROCESS_VALUE, MeasurementUnit::BARA, 1, 50, 0));
    data_model_.SetDataToUiMap("mfcpfc.25.sv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_PICA4315"), RES_PFC, WidgetType::PROCESS_VALUE, MeasurementUnit::BARA, 1, 50, 0));
    data_model_.SetDataToUiMap("mfcpfc.26.sv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_PICA4316"), RES_PFC, WidgetType::PROCESS_VALUE, MeasurementUnit::BARA, 1, 50, 0));
    data_model_.SetDataToUiMap("mfcpfc.19.pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_PICA4309"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::BARA, 1, 50, 0));
    data_model_.SetDataToUiMap("mfcpfc.20.pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_PICA4310"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::BARA, 1, 50, 0));
    data_model_.SetDataToUiMap("mfcpfc.21.pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_PICA4311"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::BARA, 1, 50, 0));
    data_model_.SetDataToUiMap("mfcpfc.22.pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_PICA4312"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::BARA, 1, 50, 0));
    data_model_.SetDataToUiMap("mfcpfc.23.pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_PICA4313"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::BARA, 1, 50, 0));
    data_model_.SetDataToUiMap("mfcpfc.24.pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_PICA4314"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::BARA, 1, 50, 0));
    data_model_.SetDataToUiMap("mfcpfc.25.pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_PICA4315"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::BARA, 1, 50, 0));
    data_model_.SetDataToUiMap("mfcpfc.26.pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_PICA4316"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::BARA, 1, 50, 0));
    // reactorB-bottom TC
    data_model_.SetDataToUiMap("plc.1.temp9_sv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_TICA4109"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp10_sv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_TICA4110"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp11_sv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_TICA4111"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp12_sv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_TICA4112"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp13_sv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_TICA4113"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp14_sv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_TICA4114"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp15_sv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_TICA4115"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp16_sv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_TICA4116"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp9_pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_TICA4109"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp10_pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_TICA4110"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp11_pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_TICA4111"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp12_pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_TICA4112"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp13_pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_TICA4113"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp14_pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_TICA4114"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp15_pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_TICA4115"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp16_pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_TICA4116"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    // reactorB-upper TC
    data_model_.SetDataToUiMap("plc.1.temp25_sv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_TICA4129"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp26_sv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_TICA4130"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp27_sv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_TICA4131"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp28_sv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_TICA4132"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp29_sv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_TICA4133"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp30_sv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_TICA4134"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp31_sv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_TICA4135"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp32_sv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_TICA4136"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp25_pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_TICA4129"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp26_pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_TICA4130"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp27_pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_TICA4131"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp28_pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_TICA4132"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp29_pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_TICA4133"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp30_pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_TICA4134"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp31_pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_TICA4135"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp32_pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_TICA4136"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    // reactorB-inner TI
    data_model_.SetDataToUiMap("plc.1.temp73_pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_TIA4149"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp74_pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_TIA4150"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp75_pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_TIA4151"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp76_pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_TIA4152"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp77_pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_TIA4153"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp78_pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_TIA4154"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp79_pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_TIA4155"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    data_model_.SetDataToUiMap("plc.1.temp80_pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_TIA4156"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 200, 0));
    // reactorB-pipe TC
    data_model_.SetDataToUiMap("plc.1.temp41_sv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_TICA4609"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp42_sv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_TICA4610"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp43_sv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_TICA4611"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp44_sv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_TICA4612"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp45_sv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_TICA4613"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp46_sv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_TICA4614"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp47_sv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_TICA4615"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp48_sv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_TICA4616"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp41_pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_TICA4609"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp42_pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_TICA4610"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp43_pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_TICA4611"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp44_pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_TICA4612"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp45_pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_TICA4613"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp46_pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_TICA4614"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp47_pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_TICA4615"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp48_pv", UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_TICA4616"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 100, 0));
    // reactorB-block busy
    data_model_.SetDataToUiMap("reactor16.9.busy", {UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4109"), RES_MOTOR, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_MOTOR, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.10.busy", {UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4110"), RES_MOTOR, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_MOTOR, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.11.busy", {UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4111"), RES_MOTOR, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_MOTOR, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.12.busy", {UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4112"), RES_MOTOR, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_MOTOR, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.13.busy", {UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4113"), RES_MOTOR, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_MOTOR, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.14.busy", {UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4114"), RES_MOTOR, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                      UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_MOTOR, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.15.busy", {UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4115"), RES_MOTOR, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                      UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_MOTOR, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.16.busy", {UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4116"), RES_MOTOR, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                      UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_MOTOR, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    // reactorB-break_off
    data_model_.SetDataToUiMap("reactor16.9.brk_off", {UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4109_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                        UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.10.brk_off", {UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4110_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                        UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.11.brk_off", {UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4111_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                        UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.12.brk_off", {UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4112_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                        UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.13.brk_off", {UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4113_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                        UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.14.brk_off", {UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4114_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                         UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.15.brk_off", {UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4115_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                         UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.16.brk_off", {UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4116_SrvOn"), RES_SRV_ON, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                         UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    // reactorB-alarm
    data_model_.SetDataToUiMap("reactor16.9.alm", {UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4109_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                    UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.10.alm", {UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4110_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                    UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.11.alm", {UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4111_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                    UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.12.alm", {UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4112_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                    UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.13.alm", {UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4113_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                    UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.14.alm", {UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4114_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.15.alm", {UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4115_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    data_model_.SetDataToUiMap("reactor16.16.alm", {UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("label_HC4116_Alarm"), RES_ALARM, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0),
                                                     UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_ELECTRIC_CYLINDER, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0)});
    // reactorB-speed
    data_model_.SetDataToUiMap("reactor16.9.speed", {UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_HC4109"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::RPM, 0, 600, 0),
                                                           UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::RPM, 0, 600, 0)});
    data_model_.SetDataToUiMap("reactor16.10.speed", {UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_HC4110"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::RPM, 0, 600, 0),
                                                           UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::RPM, 0, 600, 0)});
    data_model_.SetDataToUiMap("reactor16.11.speed", {UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_HC4111"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::RPM, 0, 600, 0),
                                                           UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::RPM, 0, 600, 0)});
    data_model_.SetDataToUiMap("reactor16.12.speed", {UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_HC4112"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::RPM, 0, 600, 0),
                                                           UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::RPM, 0, 600, 0)});
    data_model_.SetDataToUiMap("reactor16.13.speed", {UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_HC4113"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::RPM, 0, 600, 0),
                                                           UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::RPM, 0, 600, 0)});
    data_model_.SetDataToUiMap("reactor16.14.speed", {UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_HC4114"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::RPM, 0, 600, 0),
                                                            UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::RPM, 0, 600, 0)});
    data_model_.SetDataToUiMap("reactor16.15.speed", {UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_HC4115"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::RPM, 0, 600, 0),
                                                            UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::RPM, 0, 600, 0)});
    data_model_.SetDataToUiMap("reactor16.16.speed", {UiInfo(ui_->tabWidget->widget(7), QString::fromUtf8("textEdit_HC4116"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::RPM, 0, 600, 0),
                                                            UiInfo(ui_->widget_motor->widget(2), QString::fromUtf8("tableWidget"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::RPM, 0, 600, 0)});
    // gassampling
    data_model_.SetDataToUiMap("plc.1.smc14_10", UiInfo(ui_->tabWidget->widget(8), QString::fromUtf8("label_HC4401"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc14_11", UiInfo(ui_->tabWidget->widget(8), QString::fromUtf8("label_HC4402"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.smc14_12", UiInfo(ui_->tabWidget->widget(8), QString::fromUtf8("label_HC4403"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.mvalve12_sv", UiInfo(ui_->tabWidget->widget(8), QString::fromUtf8("label_HC5101"), RES_SVALVE_8, WidgetType::NONE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve12_pv", UiInfo(ui_->tabWidget->widget(8), QString::fromUtf8("label_HC5101"), RES_SVALVE_8, WidgetType::STATE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_21", UiInfo(ui_->tabWidget->widget(8), QString::fromUtf8("label_HC5102"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_22", UiInfo(ui_->tabWidget->widget(8), QString::fromUtf8("label_HC5103"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_23", UiInfo(ui_->tabWidget->widget(8), QString::fromUtf8("label_HC5104"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve13_sv", UiInfo(ui_->tabWidget->widget(8), QString::fromUtf8("label_HC5201"), RES_SVALVE_8, WidgetType::NONE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve13_pv", UiInfo(ui_->tabWidget->widget(8), QString::fromUtf8("label_HC5201"), RES_SVALVE_8, WidgetType::STATE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_20", UiInfo(ui_->tabWidget->widget(8), QString::fromUtf8("label_HC5202"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_21", UiInfo(ui_->tabWidget->widget(8), QString::fromUtf8("label_HC5203"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_22", UiInfo(ui_->tabWidget->widget(8), QString::fromUtf8("label_HC5204"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    // gassampling - TC
    data_model_.SetDataToUiMap("plc.1.temp49_sv", UiInfo(ui_->tabWidget->widget(8), QString::fromUtf8("label_TICA5501"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp50_sv", UiInfo(ui_->tabWidget->widget(8), QString::fromUtf8("label_TICA5502"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp57_sv", UiInfo(ui_->tabWidget->widget(8), QString::fromUtf8("label_TICA5105"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp58_sv", UiInfo(ui_->tabWidget->widget(8), QString::fromUtf8("label_TICA5205"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp59_sv", UiInfo(ui_->tabWidget->widget(8), QString::fromUtf8("label_TICA5101"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp60_sv", UiInfo(ui_->tabWidget->widget(8), QString::fromUtf8("label_TICA5201"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp49_pv", UiInfo(ui_->tabWidget->widget(8), QString::fromUtf8("textEdit_TICA5501"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp50_pv", UiInfo(ui_->tabWidget->widget(8), QString::fromUtf8("textEdit_TICA5502"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp57_pv", UiInfo(ui_->tabWidget->widget(8), QString::fromUtf8("textEdit_TICA5105"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp58_pv", UiInfo(ui_->tabWidget->widget(8), QString::fromUtf8("textEdit_TICA5205"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp59_pv", UiInfo(ui_->tabWidget->widget(8), QString::fromUtf8("textEdit_TICA5101"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp60_pv", UiInfo(ui_->tabWidget->widget(8), QString::fromUtf8("textEdit_TICA5201"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 100, 0));
    // gassampling - GC ready
    data_model_.SetDataToUiMap("plc.1.di1_21", UiInfo(ui_->tabWidget->widget(8), QString::fromUtf8("label_HC5106"), RES_GC, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.di1_23", UiInfo(ui_->tabWidget->widget(8), QString::fromUtf8("label_HC5206"), RES_GC, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));

    // liquidcollection
    data_model_.SetDataToUiMap("plc.1.smc11_19", UiInfo(ui_->tabWidget->widget(9), QString::fromUtf8("label_HC6101"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve14_sv", UiInfo(ui_->tabWidget->widget(9), QString::fromUtf8("label_HC6102"), RES_SVALVE_7, WidgetType::NONE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve14_pv", UiInfo(ui_->tabWidget->widget(9), QString::fromUtf8("label_HC6102"), RES_SVALVE_7, WidgetType::STATE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve15_sv", UiInfo(ui_->tabWidget->widget(9), QString::fromUtf8("label_HC6103"), RES_SVALVE_8, WidgetType::NONE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve15_pv", UiInfo(ui_->tabWidget->widget(9), QString::fromUtf8("label_HC6103"), RES_SVALVE_8, WidgetType::STATE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve16_sv", UiInfo(ui_->tabWidget->widget(9), QString::fromUtf8("label_HC6104"), RES_SVALVE_8, WidgetType::NONE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve16_pv", UiInfo(ui_->tabWidget->widget(9), QString::fromUtf8("label_HC6104"), RES_SVALVE_8, WidgetType::STATE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.smc11_20", UiInfo(ui_->tabWidget->widget(9), QString::fromUtf8("label_HC6105"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc12_19", UiInfo(ui_->tabWidget->widget(9), QString::fromUtf8("label_HC6201"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve17_sv", UiInfo(ui_->tabWidget->widget(9), QString::fromUtf8("label_HC6202"), RES_SVALVE_7, WidgetType::NONE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve17_pv", UiInfo(ui_->tabWidget->widget(9), QString::fromUtf8("label_HC6202"), RES_SVALVE_7, WidgetType::STATE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve18_sv", UiInfo(ui_->tabWidget->widget(9), QString::fromUtf8("label_HC6203"), RES_SVALVE_8, WidgetType::NONE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve18_pv", UiInfo(ui_->tabWidget->widget(9), QString::fromUtf8("label_HC6203"), RES_SVALVE_8, WidgetType::STATE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve19_sv", UiInfo(ui_->tabWidget->widget(9), QString::fromUtf8("label_HC6204"), RES_SVALVE_8, WidgetType::NONE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve19_pv", UiInfo(ui_->tabWidget->widget(9), QString::fromUtf8("label_HC6204"), RES_SVALVE_8, WidgetType::STATE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.smc12_20", UiInfo(ui_->tabWidget->widget(9), QString::fromUtf8("label_HC6205"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    // liquidcollection - TC
    data_model_.SetDataToUiMap("plc.1.temp51_sv", UiInfo(ui_->tabWidget->widget(9), QString::fromUtf8("label_TICA6501"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp52_sv", UiInfo(ui_->tabWidget->widget(9), QString::fromUtf8("label_TICA6502"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp51_pv", UiInfo(ui_->tabWidget->widget(9), QString::fromUtf8("textEdit_TICA6501"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp52_pv", UiInfo(ui_->tabWidget->widget(9), QString::fromUtf8("textEdit_TICA6502"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 100, 0));

    // liquidsampingA
    data_model_.SetDataToUiMap("plc.1.smc11_1", UiInfo(ui_->tabWidget->widget(10), QString::fromUtf8("label_HC7101"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve20_sv", UiInfo(ui_->tabWidget->widget(10), QString::fromUtf8("label_HC7102"), RES_SVALVE_7, WidgetType::NONE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve20_pv", UiInfo(ui_->tabWidget->widget(10), QString::fromUtf8("label_HC7102"), RES_SVALVE_7, WidgetType::STATE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.smc11_2", UiInfo(ui_->tabWidget->widget(10), QString::fromUtf8("label_HC7201"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc11_3", UiInfo(ui_->tabWidget->widget(10), QString::fromUtf8("label_HC7202"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc11_4", UiInfo(ui_->tabWidget->widget(10), QString::fromUtf8("label_HC7203"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc11_5", UiInfo(ui_->tabWidget->widget(10), QString::fromUtf8("label_HC7204"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc11_6", UiInfo(ui_->tabWidget->widget(10), QString::fromUtf8("label_HC7205"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc11_7", UiInfo(ui_->tabWidget->widget(10), QString::fromUtf8("label_HC7206"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc11_8", UiInfo(ui_->tabWidget->widget(10), QString::fromUtf8("label_HC7207"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc11_9", UiInfo(ui_->tabWidget->widget(10), QString::fromUtf8("label_HC7208"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc11_10", UiInfo(ui_->tabWidget->widget(10), QString::fromUtf8("label_HC7209"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc11_11", UiInfo(ui_->tabWidget->widget(10), QString::fromUtf8("label_HC7210"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc11_12", UiInfo(ui_->tabWidget->widget(10), QString::fromUtf8("label_HC7211"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc11_13", UiInfo(ui_->tabWidget->widget(10), QString::fromUtf8("label_HC7212"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc11_14", UiInfo(ui_->tabWidget->widget(10), QString::fromUtf8("label_HC7213"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc11_15", UiInfo(ui_->tabWidget->widget(10), QString::fromUtf8("label_HC7214"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc11_16", UiInfo(ui_->tabWidget->widget(10), QString::fromUtf8("label_HC7215"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc11_17", UiInfo(ui_->tabWidget->widget(10), QString::fromUtf8("label_HC7216"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc11_18", UiInfo(ui_->tabWidget->widget(10), QString::fromUtf8("label_HC7217"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    // liquidsamplingA - TC
    data_model_.SetDataToUiMap("plc.1.temp53_sv", UiInfo(ui_->tabWidget->widget(10), QString::fromUtf8("label_TICA7501"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp54_sv", UiInfo(ui_->tabWidget->widget(10), QString::fromUtf8("label_TICA7502"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp53_pv", UiInfo(ui_->tabWidget->widget(10), QString::fromUtf8("textEdit_TICA7501"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp54_pv", UiInfo(ui_->tabWidget->widget(10), QString::fromUtf8("textEdit_TICA7502"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 100, 0));
    // liquidsamplingB
    data_model_.SetDataToUiMap("plc.1.smc12_1", UiInfo(ui_->tabWidget->widget(11), QString::fromUtf8("label_HC7301"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve21_sv", UiInfo(ui_->tabWidget->widget(11), QString::fromUtf8("label_HC7302"), RES_SVALVE_7, WidgetType::NONE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve21_pv", UiInfo(ui_->tabWidget->widget(11), QString::fromUtf8("label_HC7302"), RES_SVALVE_7, WidgetType::STATE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.smc12_2", UiInfo(ui_->tabWidget->widget(11), QString::fromUtf8("label_HC7401"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc12_3", UiInfo(ui_->tabWidget->widget(11), QString::fromUtf8("label_HC7402"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc12_4", UiInfo(ui_->tabWidget->widget(11), QString::fromUtf8("label_HC7403"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc12_5", UiInfo(ui_->tabWidget->widget(11), QString::fromUtf8("label_HC7404"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc12_6", UiInfo(ui_->tabWidget->widget(11), QString::fromUtf8("label_HC7405"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc12_7", UiInfo(ui_->tabWidget->widget(11), QString::fromUtf8("label_HC7406"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc12_8", UiInfo(ui_->tabWidget->widget(11), QString::fromUtf8("label_HC7407"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc12_9", UiInfo(ui_->tabWidget->widget(11), QString::fromUtf8("label_HC7408"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc12_10", UiInfo(ui_->tabWidget->widget(11), QString::fromUtf8("label_HC7409"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc12_11", UiInfo(ui_->tabWidget->widget(11), QString::fromUtf8("label_HC7410"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc12_12", UiInfo(ui_->tabWidget->widget(11), QString::fromUtf8("label_HC7411"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc12_13", UiInfo(ui_->tabWidget->widget(11), QString::fromUtf8("label_HC7412"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc12_14", UiInfo(ui_->tabWidget->widget(11), QString::fromUtf8("label_HC7413"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc12_15", UiInfo(ui_->tabWidget->widget(11), QString::fromUtf8("label_HC7414"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc12_16", UiInfo(ui_->tabWidget->widget(11), QString::fromUtf8("label_HC7415"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc12_17", UiInfo(ui_->tabWidget->widget(11), QString::fromUtf8("label_HC7416"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc12_18", UiInfo(ui_->tabWidget->widget(11), QString::fromUtf8("label_HC7417"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    // liquidsamplingB - TC
    data_model_.SetDataToUiMap("plc.1.temp55_sv", UiInfo(ui_->tabWidget->widget(11), QString::fromUtf8("label_TICA7503"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp56_sv", UiInfo(ui_->tabWidget->widget(11), QString::fromUtf8("label_TICA7504"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp55_pv", UiInfo(ui_->tabWidget->widget(11), QString::fromUtf8("textEdit_TICA7503"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 100, 0));
    data_model_.SetDataToUiMap("plc.1.temp56_pv", UiInfo(ui_->tabWidget->widget(11), QString::fromUtf8("textEdit_TICA7504"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 100, 0));

    // ui_to_data
    //data_model_.SetUiToDataMap("gasfeed.svlabel", DataDef("plc.1.writebyte_channel_0", "plc.1.writebyte_channel_0", "plc.1.writebyte_channel_0"));
    //data_model_.SetUiToDataMap("gasfeed.svlabel_2", DataDef("plc.1.out1", "plc.1.out1", "plc.1.out1"));
    //data_model_.SetUiToDataMap("gasfeed.svlabel_3", DataDef("mfcpfc.4.pv", "mfcpfc.4.sv", "mfcpfc.4.sv"));
    // gasfeed
    data_model_.SetUiToDataMap("gasfeed.label_HC1110", DataDef("plc.1.smc14_1", "plc.1.smc14_1", "plc.1.smc14_1"));
    data_model_.SetUiToDataMap("gasfeed.label_HC1120", DataDef("plc.1.smc14_2", "plc.1.smc14_2", "plc.1.smc14_2"));
    data_model_.SetUiToDataMap("gasfeed.label_HC1130", DataDef("plc.1.smc14_3", "plc.1.smc14_3", "plc.1.smc14_3"));
    data_model_.SetUiToDataMap("gasfeed.label_HC1140", DataDef("plc.1.smc14_4", "plc.1.smc14_4", "plc.1.smc14_4"));
    data_model_.SetUiToDataMap("gasfeed.label_HC3110", DataDef("plc.1.smc14_5", "plc.1.smc14_5", "plc.1.smc14_5"));
    data_model_.SetUiToDataMap("gasfeed.label_HC3120", DataDef("plc.1.smc14_6", "plc.1.smc14_6", "plc.1.smc14_6"));
    data_model_.SetUiToDataMap("gasfeed.label_HC1020", DataDef("plc.1.smc14_7", "plc.1.smc14_7", "plc.1.smc14_7"));
    data_model_.SetUiToDataMap("gasfeed.label_HC1021", DataDef("plc.1.smc14_8", "plc.1.smc14_8", "plc.1.smc14_8"));
    data_model_.SetUiToDataMap("gasfeed.label_HC1022", DataDef("plc.1.smc14_9", "plc.1.smc14_9", "plc.1.smc14_9"));
    data_model_.SetUiToDataMap("gasfeed.label_HC1201", DataDef("plc.1.mvalve7_pv", "plc.1.mvalve7_pv", "plc.1.mvalve7_sv"));
    data_model_.SetUiToDataMap("gasfeed.label_HC1202", DataDef("plc.1.smc10_1", "plc.1.smc10_1", "plc.1.smc10_1"));
    data_model_.SetUiToDataMap("gasfeed.label_HC1203", DataDef("plc.1.smc10_2", "plc.1.smc10_2", "plc.1.smc10_2"));
    data_model_.SetUiToDataMap("gasfeed.label_HC1204", DataDef("plc.1.smc10_3", "plc.1.smc10_3", "plc.1.smc10_3"));
    data_model_.SetUiToDataMap("gasfeed.label_HC1205", DataDef("plc.1.mvalve8_pv", "plc.1.mvalve8_pv", "plc.1.mvalve8_sv"));
    data_model_.SetUiToDataMap("gasfeed.label_HC1206", DataDef("plc.1.mvalve9_pv", "plc.1.mvalve9_pv", "plc.1.mvalve9_sv"));
    data_model_.SetUiToDataMap("gasfeed.label_HC1207", DataDef("plc.1.mvalve10_pv", "plc.1.mvalve10_pv", "plc.1.mvalve10_sv"));
    data_model_.SetUiToDataMap("gasfeed.label_HC1208", DataDef("plc.1.mvalve11_pv", "plc.1.mvalve11_pv", "plc.1.mvalve11_sv"));
    //
    data_model_.SetUiToDataMap("gasfeed.label_FICA1110", DataDef("mfcpfc.1.pv", "mfcpfc.1.sv", "mfcpfc.1.sv"));
    data_model_.SetUiToDataMap("gasfeed.label_FICA1120", DataDef("mfcpfc.2.pv", "mfcpfc.2.sv", "mfcpfc.2.sv"));
    data_model_.SetUiToDataMap("gasfeed.label_FICA1130", DataDef("mfcpfc.3.pv", "mfcpfc.3.sv", "mfcpfc.3.sv"));
    data_model_.SetUiToDataMap("gasfeed.label_FICA1140", DataDef("mfcpfc.4.pv", "mfcpfc.4.sv", "mfcpfc.4.sv"));
    // gasfeed - pump,electric valve
    data_model_.SetUiToDataMap("gasfeed.label_HC1400", DataDef("plc.1.dq3_2", "plc.1.dq3_2", "plc.1.dq3_2"));
    data_model_.SetUiToDataMap("gasfeed.label_HC1402", DataDef("plc.1.dq3_7", "plc.1.dq3_7", "plc.1.dq3_7"));

    // liquidswitch
    data_model_.SetUiToDataMap("liquidswitch.label_HC2301", DataDef("plc.1.mvalve1_pv", "plc.1.mvalve1_pv", "plc.1.mvalve1_sv"));
    data_model_.SetUiToDataMap("liquidswitch.label_HC2302", DataDef("plc.1.smc9_1", "plc.1.smc9_1", "plc.1.smc9_1"));
    data_model_.SetUiToDataMap("liquidswitch.label_HC2303", DataDef("plc.1.mvalve2_pv", "plc.1.mvalve2_pv", "plc.1.mvalve2_sv"));
    data_model_.SetUiToDataMap("liquidswitch.label_HC2304", DataDef("plc.1.mvalve3_pv", "plc.1.mvalve3_pv", "plc.1.mvalve3_sv"));
    data_model_.SetUiToDataMap("liquidswitch.label_HC2401", DataDef("plc.1.mvalve4_pv", "plc.1.mvalve4_pv", "plc.1.mvalve4_sv"));
    data_model_.SetUiToDataMap("liquidswitch.label_HC2403", DataDef("plc.1.mvalve5_pv", "plc.1.mvalve5_pv", "plc.1.mvalve5_sv"));
    data_model_.SetUiToDataMap("liquidswitch.label_HC2404", DataDef("plc.1.mvalve6_pv", "plc.1.mvalve6_pv", "plc.1.mvalve6_sv"));
    // liquidswitch - pump
    data_model_.SetUiToDataMap("liquidswitch.label_FICA2305", DataDef("plc.1.pump1_pv", "plc.1.pump1_sv", "plc.1.pump1_sv"));
    data_model_.SetUiToDataMap("liquidswitch.label_FICA2405", DataDef("plc.1.pump2_pv", "plc.1.pump2_sv", "plc.1.pump2_sv"));

    // liquidfeed
    data_model_.SetUiToDataMap("liquidfeeda.label_HC2306", DataDef("plc.1.smc9_2", "plc.1.smc9_2", "plc.1.smc9_2"));
    data_model_.SetUiToDataMap("liquidfeeda.label_HC2307", DataDef("plc.1.smc9_3", "plc.1.smc9_3", "plc.1.smc9_3"));
    data_model_.SetUiToDataMap("liquidfeeda.label_HC2308", DataDef("plc.1.smc9_4", "plc.1.smc9_4", "plc.1.smc9_4"));
    data_model_.SetUiToDataMap("liquidfeeda.label_HC2311", DataDef("plc.1.smc9_5", "plc.1.smc9_5", "plc.1.smc9_5"));
    data_model_.SetUiToDataMap("liquidfeeda.label_HC2312", DataDef("plc.1.smc9_6", "plc.1.smc9_6", "plc.1.smc9_6"));
    data_model_.SetUiToDataMap("liquidfeeda.label_HC2313", DataDef("plc.1.smc9_7", "plc.1.smc9_7", "plc.1.smc9_7"));
    data_model_.SetUiToDataMap("liquidfeeda.label_HC2314", DataDef("plc.1.smc9_8", "plc.1.smc9_8", "plc.1.smc9_8"));
    data_model_.SetUiToDataMap("liquidfeeda.label_HC2315", DataDef("plc.1.smc10_10", "plc.1.smc10_10", "plc.1.smc10_10"));
    data_model_.SetUiToDataMap("liquidfeeda.label_HC2316", DataDef("plc.1.smc10_11", "plc.1.smc10_11", "plc.1.smc10_11"));
    data_model_.SetUiToDataMap("liquidfeeda.label_HC2317", DataDef("plc.1.smc10_12", "plc.1.smc10_12", "plc.1.smc10_12"));
    data_model_.SetUiToDataMap("liquidfeeda.label_HC2318", DataDef("plc.1.smc9_12", "plc.1.smc9_12", "plc.1.smc9_12"));
    data_model_.SetUiToDataMap("liquidfeeda.label_HC2319", DataDef("plc.1.smc9_13", "plc.1.smc9_13", "plc.1.smc9_13"));
    data_model_.SetUiToDataMap("liquidfeeda.label_HC2320", DataDef("plc.1.smc9_14", "plc.1.smc9_14", "plc.1.smc9_14"));
    data_model_.SetUiToDataMap("liquidfeeda.label_HC2321", DataDef("plc.1.smc9_15", "plc.1.smc9_15", "plc.1.smc9_15"));
    data_model_.SetUiToDataMap("liquidfeeda.label_HC2322", DataDef("plc.1.smc10_17", "plc.1.smc10_17", "plc.1.smc10_17"));
    data_model_.SetUiToDataMap("liquidfeeda.label_HC2323", DataDef("plc.1.smc10_18", "plc.1.smc10_18", "plc.1.smc10_18"));
    data_model_.SetUiToDataMap("liquidfeeda.label_HC2324", DataDef("plc.1.smc10_19", "plc.1.smc10_19", "plc.1.smc10_19"));
    data_model_.SetUiToDataMap("liquidfeeda.label_HC2325", DataDef("plc.1.smc10_20", "plc.1.smc10_20", "plc.1.smc10_20"));
    //
    data_model_.SetUiToDataMap("liquidfeedb.label_HC2407", DataDef("plc.1.smc10_4", "plc.1.smc10_4", "plc.1.smc10_4"));
    data_model_.SetUiToDataMap("liquidfeedb.label_HC2408", DataDef("plc.1.smc10_5", "plc.1.smc10_5", "plc.1.smc10_5"));
    data_model_.SetUiToDataMap("liquidfeedb.label_HC2411", DataDef("plc.1.smc10_6", "plc.1.smc10_6", "plc.1.smc10_6"));
    data_model_.SetUiToDataMap("liquidfeedb.label_HC2412", DataDef("plc.1.smc10_7", "plc.1.smc10_7", "plc.1.smc10_7"));
    data_model_.SetUiToDataMap("liquidfeedb.label_HC2413", DataDef("plc.1.smc10_8", "plc.1.smc10_8", "plc.1.smc10_8"));
    data_model_.SetUiToDataMap("liquidfeedb.label_HC2414", DataDef("plc.1.smc10_9", "plc.1.smc10_9", "plc.1.smc10_9"));
    data_model_.SetUiToDataMap("liquidfeedb.label_HC2415", DataDef("plc.1.smc9_9", "plc.1.smc9_9", "plc.1.smc9_9"));
    data_model_.SetUiToDataMap("liquidfeedb.label_HC2416", DataDef("plc.1.smc9_10", "plc.1.smc9_10", "plc.1.smc9_10"));
    data_model_.SetUiToDataMap("liquidfeedb.label_HC2417", DataDef("plc.1.smc9_11", "plc.1.smc9_11", "plc.1.smc9_11"));
    data_model_.SetUiToDataMap("liquidfeedb.label_HC2418", DataDef("plc.1.smc10_13", "plc.1.smc10_13", "plc.1.smc10_13"));
    data_model_.SetUiToDataMap("liquidfeedb.label_HC2419", DataDef("plc.1.smc10_14", "plc.1.smc10_14", "plc.1.smc10_14"));
    data_model_.SetUiToDataMap("liquidfeedb.label_HC2420", DataDef("plc.1.smc10_15", "plc.1.smc10_15", "plc.1.smc10_15"));
    data_model_.SetUiToDataMap("liquidfeedb.label_HC2421", DataDef("plc.1.smc10_16", "plc.1.smc10_16", "plc.1.smc10_16"));
    data_model_.SetUiToDataMap("liquidfeedb.label_HC2422", DataDef("plc.1.smc9_16", "plc.1.smc9_16", "plc.1.smc9_16"));
    data_model_.SetUiToDataMap("liquidfeedb.label_HC2423", DataDef("plc.1.smc9_17", "plc.1.smc9_17", "plc.1.smc9_17"));
    data_model_.SetUiToDataMap("liquidfeedb.label_HC2424", DataDef("plc.1.smc9_18", "plc.1.smc9_18", "plc.1.smc9_18"));
    data_model_.SetUiToDataMap("liquidfeedb.label_HC2425", DataDef("plc.1.smc9_19", "plc.1.smc9_19", "plc.1.smc9_19"));
    //
    data_model_.SetUiToDataMap("cylindera.label_HC3101", DataDef("plc.1.smc1_1", "plc.1.smc1_1", "plc.1.smc1_1"));
    data_model_.SetUiToDataMap("cylindera.label_HC3201", DataDef("plc.1.smc1_2", "plc.1.smc1_2", "plc.1.smc1_2"));
    data_model_.SetUiToDataMap("cylindera.label_HC3301", DataDef("plc.1.smc1_3", "plc.1.smc1_3", "plc.1.smc1_3"));
    data_model_.SetUiToDataMap("cylindera.label_HC3401", DataDef("plc.1.smc1_4", "plc.1.smc1_4", "plc.1.smc1_4"));
    data_model_.SetUiToDataMap("cylindera.label_HC3501", DataDef("plc.1.smc1_5", "plc.1.smc1_5", "plc.1.smc1_5"));
    data_model_.SetUiToDataMap("cylindera.label_HC3601", DataDef("plc.1.smc1_6", "plc.1.smc1_6", "plc.1.smc1_6"));
    data_model_.SetUiToDataMap("cylindera.label_HC3701", DataDef("plc.1.smc1_7", "plc.1.smc1_7", "plc.1.smc1_7"));
    data_model_.SetUiToDataMap("cylindera.label_HC3801", DataDef("plc.1.smc1_8", "plc.1.smc1_8", "plc.1.smc1_8"));
    data_model_.SetUiToDataMap("cylindera.label_HC3102", DataDef("plc.1.smc1_9", "plc.1.smc1_9", "plc.1.smc1_9"));
    data_model_.SetUiToDataMap("cylindera.label_HC3202", DataDef("plc.1.smc1_10", "plc.1.smc1_10", "plc.1.smc1_10"));
    data_model_.SetUiToDataMap("cylindera.label_HC3302", DataDef("plc.1.smc1_11", "plc.1.smc1_11", "plc.1.smc1_11"));
    data_model_.SetUiToDataMap("cylindera.label_HC3402", DataDef("plc.1.smc1_12", "plc.1.smc1_12", "plc.1.smc1_12"));
    data_model_.SetUiToDataMap("cylindera.label_HC3502", DataDef("plc.1.smc1_13", "plc.1.smc1_13", "plc.1.smc1_13"));
    data_model_.SetUiToDataMap("cylindera.label_HC3602", DataDef("plc.1.smc1_14", "plc.1.smc1_14", "plc.1.smc1_14"));
    data_model_.SetUiToDataMap("cylindera.label_HC3702", DataDef("plc.1.smc1_15", "plc.1.smc1_15", "plc.1.smc1_15"));
    data_model_.SetUiToDataMap("cylindera.label_HC3802", DataDef("plc.1.smc1_16", "plc.1.smc1_16", "plc.1.smc1_16"));
    //
    data_model_.SetUiToDataMap("cylindera.label_HC3103", DataDef("plc.1.smc2_1", "plc.1.smc2_1", "plc.1.smc2_1"));
    data_model_.SetUiToDataMap("cylindera.label_HC3203", DataDef("plc.1.smc2_2", "plc.1.smc2_2", "plc.1.smc2_2"));
    data_model_.SetUiToDataMap("cylindera.label_HC3303", DataDef("plc.1.smc2_3", "plc.1.smc2_3", "plc.1.smc2_3"));
    data_model_.SetUiToDataMap("cylindera.label_HC3403", DataDef("plc.1.smc2_4", "plc.1.smc2_4", "plc.1.smc2_4"));
    data_model_.SetUiToDataMap("cylindera.label_HC3503", DataDef("plc.1.smc2_5", "plc.1.smc2_5", "plc.1.smc2_5"));
    data_model_.SetUiToDataMap("cylindera.label_HC3603", DataDef("plc.1.smc2_6", "plc.1.smc2_6", "plc.1.smc2_6"));
    data_model_.SetUiToDataMap("cylindera.label_HC3703", DataDef("plc.1.smc2_7", "plc.1.smc2_7", "plc.1.smc2_7"));
    data_model_.SetUiToDataMap("cylindera.label_HC3803", DataDef("plc.1.smc2_8", "plc.1.smc2_8", "plc.1.smc2_8"));
    data_model_.SetUiToDataMap("cylindera.label_HC3104", DataDef("plc.1.smc2_9", "plc.1.smc2_9", "plc.1.smc2_9"));
    data_model_.SetUiToDataMap("cylindera.label_HC3204", DataDef("plc.1.smc2_10", "plc.1.smc2_10", "plc.1.smc2_10"));
    data_model_.SetUiToDataMap("cylindera.label_HC3304", DataDef("plc.1.smc2_11", "plc.1.smc2_11", "plc.1.smc2_11"));
    data_model_.SetUiToDataMap("cylindera.label_HC3404", DataDef("plc.1.smc2_12", "plc.1.smc2_12", "plc.1.smc2_12"));
    data_model_.SetUiToDataMap("cylindera.label_HC3504", DataDef("plc.1.smc2_13", "plc.1.smc2_13", "plc.1.smc2_13"));
    data_model_.SetUiToDataMap("cylindera.label_HC3604", DataDef("plc.1.smc2_14", "plc.1.smc2_14", "plc.1.smc2_14"));
    data_model_.SetUiToDataMap("cylindera.label_HC3704", DataDef("plc.1.smc2_15", "plc.1.smc2_15", "plc.1.smc2_15"));
    data_model_.SetUiToDataMap("cylindera.label_HC3804", DataDef("plc.1.smc2_16", "plc.1.smc2_16", "plc.1.smc2_16"));
    //
    data_model_.SetUiToDataMap("cylindera.label_HC3105", DataDef("plc.1.smc3_1", "plc.1.smc3_1", "plc.1.smc3_1"));
    data_model_.SetUiToDataMap("cylindera.label_HC3205", DataDef("plc.1.smc3_2", "plc.1.smc3_2", "plc.1.smc3_2"));
    data_model_.SetUiToDataMap("cylindera.label_HC3305", DataDef("plc.1.smc3_3", "plc.1.smc3_3", "plc.1.smc3_3"));
    data_model_.SetUiToDataMap("cylindera.label_HC3405", DataDef("plc.1.smc3_4", "plc.1.smc3_4", "plc.1.smc3_4"));
    data_model_.SetUiToDataMap("cylindera.label_HC3505", DataDef("plc.1.smc3_5", "plc.1.smc3_5", "plc.1.smc3_5"));
    data_model_.SetUiToDataMap("cylindera.label_HC3605", DataDef("plc.1.smc3_6", "plc.1.smc3_6", "plc.1.smc3_6"));
    data_model_.SetUiToDataMap("cylindera.label_HC3705", DataDef("plc.1.smc3_7", "plc.1.smc3_7", "plc.1.smc3_7"));
    data_model_.SetUiToDataMap("cylindera.label_HC3805", DataDef("plc.1.smc3_8", "plc.1.smc3_8", "plc.1.smc3_8"));
    data_model_.SetUiToDataMap("cylindera.label_HC3106", DataDef("plc.1.smc3_9", "plc.1.smc3_9", "plc.1.smc3_9"));
    data_model_.SetUiToDataMap("cylindera.label_HC3206", DataDef("plc.1.smc3_10", "plc.1.smc3_10", "plc.1.smc3_10"));
    data_model_.SetUiToDataMap("cylindera.label_HC3306", DataDef("plc.1.smc3_11", "plc.1.smc3_11", "plc.1.smc3_11"));
    data_model_.SetUiToDataMap("cylindera.label_HC3406", DataDef("plc.1.smc3_12", "plc.1.smc3_12", "plc.1.smc3_12"));
    data_model_.SetUiToDataMap("cylindera.label_HC3506", DataDef("plc.1.smc3_13", "plc.1.smc3_13", "plc.1.smc3_13"));
    data_model_.SetUiToDataMap("cylindera.label_HC3606", DataDef("plc.1.smc3_14", "plc.1.smc3_14", "plc.1.smc3_14"));
    data_model_.SetUiToDataMap("cylindera.label_HC3706", DataDef("plc.1.smc3_15", "plc.1.smc3_15", "plc.1.smc3_15"));
    data_model_.SetUiToDataMap("cylindera.label_HC3806", DataDef("plc.1.smc3_16", "plc.1.smc3_16", "plc.1.smc3_16"));
    //
    data_model_.SetUiToDataMap("cylindera.label_HC3107", DataDef("plc.1.smc4_1", "plc.1.smc4_1", "plc.1.smc4_1"));
    data_model_.SetUiToDataMap("cylindera.label_HC3207", DataDef("plc.1.smc4_2", "plc.1.smc4_2", "plc.1.smc4_2"));
    data_model_.SetUiToDataMap("cylindera.label_HC3307", DataDef("plc.1.smc4_3", "plc.1.smc4_3", "plc.1.smc4_3"));
    data_model_.SetUiToDataMap("cylindera.label_HC3407", DataDef("plc.1.smc4_4", "plc.1.smc4_4", "plc.1.smc4_4"));
    data_model_.SetUiToDataMap("cylindera.label_HC3507", DataDef("plc.1.smc4_5", "plc.1.smc4_5", "plc.1.smc4_5"));
    data_model_.SetUiToDataMap("cylindera.label_HC3607", DataDef("plc.1.smc4_6", "plc.1.smc4_6", "plc.1.smc4_6"));
    data_model_.SetUiToDataMap("cylindera.label_HC3707", DataDef("plc.1.smc4_7", "plc.1.smc4_7", "plc.1.smc4_7"));
    data_model_.SetUiToDataMap("cylindera.label_HC3807", DataDef("plc.1.smc4_8", "plc.1.smc4_8", "plc.1.smc4_8"));
    data_model_.SetUiToDataMap("cylindera.label_HC3108", DataDef("plc.1.smc4_9", "plc.1.smc4_9", "plc.1.smc4_9"));
    data_model_.SetUiToDataMap("cylindera.label_HC3208", DataDef("plc.1.smc4_10", "plc.1.smc4_10", "plc.1.smc4_10"));
    data_model_.SetUiToDataMap("cylindera.label_HC3308", DataDef("plc.1.smc4_11", "plc.1.smc4_11", "plc.1.smc4_11"));
    data_model_.SetUiToDataMap("cylindera.label_HC3408", DataDef("plc.1.smc4_12", "plc.1.smc4_12", "plc.1.smc4_12"));
    data_model_.SetUiToDataMap("cylindera.label_HC3508", DataDef("plc.1.smc4_13", "plc.1.smc4_13", "plc.1.smc4_13"));
    data_model_.SetUiToDataMap("cylindera.label_HC3608", DataDef("plc.1.smc4_14", "plc.1.smc4_14", "plc.1.smc4_14"));
    data_model_.SetUiToDataMap("cylindera.label_HC3708", DataDef("plc.1.smc4_15", "plc.1.smc4_15", "plc.1.smc4_15"));
    data_model_.SetUiToDataMap("cylindera.label_HC3808", DataDef("plc.1.smc4_16", "plc.1.smc4_16", "plc.1.smc4_16"));
    //
    data_model_.SetUiToDataMap("cylinderb.label_HC3109", DataDef("plc.1.smc5_1", "plc.1.smc5_1", "plc.1.smc5_1"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3209", DataDef("plc.1.smc5_2", "plc.1.smc5_2", "plc.1.smc5_2"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3309", DataDef("plc.1.smc5_3", "plc.1.smc5_3", "plc.1.smc5_3"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3409", DataDef("plc.1.smc5_4", "plc.1.smc5_4", "plc.1.smc5_4"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3509", DataDef("plc.1.smc5_5", "plc.1.smc5_5", "plc.1.smc5_5"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3609", DataDef("plc.1.smc5_6", "plc.1.smc5_6", "plc.1.smc5_6"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3709", DataDef("plc.1.smc5_7", "plc.1.smc5_7", "plc.1.smc5_7"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3809", DataDef("plc.1.smc5_8", "plc.1.smc5_8", "plc.1.smc5_8"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3110", DataDef("plc.1.smc5_9", "plc.1.smc5_9", "plc.1.smc5_9"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3210", DataDef("plc.1.smc5_10", "plc.1.smc5_10", "plc.1.smc5_10"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3310", DataDef("plc.1.smc5_11", "plc.1.smc5_11", "plc.1.smc5_11"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3410", DataDef("plc.1.smc5_12", "plc.1.smc5_12", "plc.1.smc5_12"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3510", DataDef("plc.1.smc5_13", "plc.1.smc5_13", "plc.1.smc5_13"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3610", DataDef("plc.1.smc5_14", "plc.1.smc5_14", "plc.1.smc5_14"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3710", DataDef("plc.1.smc5_15", "plc.1.smc5_15", "plc.1.smc5_15"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3810", DataDef("plc.1.smc5_16", "plc.1.smc5_16", "plc.1.smc5_16"));
    //
    data_model_.SetUiToDataMap("cylinderb.label_HC3111", DataDef("plc.1.smc6_1", "plc.1.smc6_1", "plc.1.smc6_1"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3211", DataDef("plc.1.smc6_2", "plc.1.smc6_2", "plc.1.smc6_2"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3311", DataDef("plc.1.smc6_3", "plc.1.smc6_3", "plc.1.smc6_3"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3411", DataDef("plc.1.smc6_4", "plc.1.smc6_4", "plc.1.smc6_4"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3511", DataDef("plc.1.smc6_5", "plc.1.smc6_5", "plc.1.smc6_5"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3611", DataDef("plc.1.smc6_6", "plc.1.smc6_6", "plc.1.smc6_6"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3711", DataDef("plc.1.smc6_7", "plc.1.smc6_7", "plc.1.smc6_7"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3811", DataDef("plc.1.smc6_8", "plc.1.smc6_8", "plc.1.smc6_8"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3112", DataDef("plc.1.smc6_9", "plc.1.smc6_9", "plc.1.smc6_9"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3212", DataDef("plc.1.smc6_10", "plc.1.smc6_10", "plc.1.smc6_10"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3312", DataDef("plc.1.smc6_11", "plc.1.smc6_11", "plc.1.smc6_11"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3412", DataDef("plc.1.smc6_12", "plc.1.smc6_12", "plc.1.smc6_12"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3512", DataDef("plc.1.smc6_13", "plc.1.smc6_13", "plc.1.smc6_13"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3612", DataDef("plc.1.smc6_14", "plc.1.smc6_14", "plc.1.smc6_14"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3712", DataDef("plc.1.smc6_15", "plc.1.smc6_15", "plc.1.smc6_15"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3812", DataDef("plc.1.smc6_16", "plc.1.smc6_16", "plc.1.smc6_16"));
    //
    data_model_.SetUiToDataMap("cylinderb.label_HC3113", DataDef("plc.1.smc7_1", "plc.1.smc7_1", "plc.1.smc7_1"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3213", DataDef("plc.1.smc7_2", "plc.1.smc7_2", "plc.1.smc7_2"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3313", DataDef("plc.1.smc7_3", "plc.1.smc7_3", "plc.1.smc7_3"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3413", DataDef("plc.1.smc7_4", "plc.1.smc7_4", "plc.1.smc7_4"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3513", DataDef("plc.1.smc7_5", "plc.1.smc7_5", "plc.1.smc7_5"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3613", DataDef("plc.1.smc7_6", "plc.1.smc7_6", "plc.1.smc7_6"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3713", DataDef("plc.1.smc7_7", "plc.1.smc7_7", "plc.1.smc7_7"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3813", DataDef("plc.1.smc7_8", "plc.1.smc7_8", "plc.1.smc7_8"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3114", DataDef("plc.1.smc7_9", "plc.1.smc7_9", "plc.1.smc7_9"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3214", DataDef("plc.1.smc7_10", "plc.1.smc7_10", "plc.1.smc7_10"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3314", DataDef("plc.1.smc7_11", "plc.1.smc7_11", "plc.1.smc7_11"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3414", DataDef("plc.1.smc7_12", "plc.1.smc7_12", "plc.1.smc7_12"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3514", DataDef("plc.1.smc7_13", "plc.1.smc7_13", "plc.1.smc7_13"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3614", DataDef("plc.1.smc7_14", "plc.1.smc7_14", "plc.1.smc7_14"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3714", DataDef("plc.1.smc7_15", "plc.1.smc7_15", "plc.1.smc7_15"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3814", DataDef("plc.1.smc7_16", "plc.1.smc7_16", "plc.1.smc7_16"));
    //
    data_model_.SetUiToDataMap("cylinderb.label_HC3115", DataDef("plc.1.smc8_1", "plc.1.smc8_1", "plc.1.smc8_1"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3215", DataDef("plc.1.smc8_2", "plc.1.smc8_2", "plc.1.smc8_2"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3315", DataDef("plc.1.smc8_3", "plc.1.smc8_3", "plc.1.smc8_3"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3415", DataDef("plc.1.smc8_4", "plc.1.smc8_4", "plc.1.smc8_4"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3515", DataDef("plc.1.smc8_5", "plc.1.smc8_5", "plc.1.smc8_5"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3615", DataDef("plc.1.smc8_6", "plc.1.smc8_6", "plc.1.smc8_6"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3715", DataDef("plc.1.smc8_7", "plc.1.smc8_7", "plc.1.smc8_7"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3815", DataDef("plc.1.smc8_8", "plc.1.smc8_8", "plc.1.smc8_8"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3116", DataDef("plc.1.smc8_9", "plc.1.smc8_9", "plc.1.smc8_9"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3216", DataDef("plc.1.smc8_10", "plc.1.smc8_10", "plc.1.smc8_10"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3316", DataDef("plc.1.smc8_11", "plc.1.smc8_11", "plc.1.smc8_11"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3416", DataDef("plc.1.smc8_12", "plc.1.smc8_12", "plc.1.smc8_12"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3516", DataDef("plc.1.smc8_13", "plc.1.smc8_13", "plc.1.smc8_13"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3616", DataDef("plc.1.smc8_14", "plc.1.smc8_14", "plc.1.smc8_14"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3716", DataDef("plc.1.smc8_15", "plc.1.smc8_15", "plc.1.smc8_15"));
    data_model_.SetUiToDataMap("cylinderb.label_HC3816", DataDef("plc.1.smc8_16", "plc.1.smc8_16", "plc.1.smc8_16"));
    // reactorA
    data_model_.SetUiToDataMap("reactora.label_HC4601", DataDef("plc.1.smc13_1", "plc.1.smc13_1", "plc.1.smc13_1"));
    data_model_.SetUiToDataMap("reactora.label_HC4602", DataDef("plc.1.smc13_2", "plc.1.smc13_2", "plc.1.smc13_2"));
    data_model_.SetUiToDataMap("reactora.label_HC4603", DataDef("plc.1.smc13_3", "plc.1.smc13_3", "plc.1.smc13_3"));
    data_model_.SetUiToDataMap("reactora.label_HC4604", DataDef("plc.1.smc13_4", "plc.1.smc13_4", "plc.1.smc13_4"));
    data_model_.SetUiToDataMap("reactora.label_HC4605", DataDef("plc.1.smc14_13", "plc.1.smc14_13", "plc.1.smc14_13"));
    data_model_.SetUiToDataMap("reactora.label_HC4606", DataDef("plc.1.smc14_14", "plc.1.smc14_14", "plc.1.smc14_14"));
    data_model_.SetUiToDataMap("reactora.label_HC4607", DataDef("plc.1.smc14_15", "plc.1.smc14_15", "plc.1.smc14_15"));
    data_model_.SetUiToDataMap("reactora.label_HC4608", DataDef("plc.1.smc14_16", "plc.1.smc14_16", "plc.1.smc14_16"));
    // reactorA - support cylinder
    data_model_.SetUiToDataMap("reactora.label_HC4201", DataDef("plc.1.smc1_19", "plc.1.smc1_19", "plc.1.smc1_19"));
    data_model_.SetUiToDataMap("reactora.label_HC4202", DataDef("plc.1.smc1_20", "plc.1.smc1_20", "plc.1.smc1_20"));
    data_model_.SetUiToDataMap("reactora.label_HC4203", DataDef("plc.1.smc2_19", "plc.1.smc2_19", "plc.1.smc2_19"));
    data_model_.SetUiToDataMap("reactora.label_HC4204", DataDef("plc.1.smc2_20", "plc.1.smc2_20", "plc.1.smc2_20"));
    data_model_.SetUiToDataMap("reactora.label_HC4205", DataDef("plc.1.smc3_19", "plc.1.smc3_19", "plc.1.smc3_19"));
    data_model_.SetUiToDataMap("reactora.label_HC4206", DataDef("plc.1.smc3_20", "plc.1.smc3_20", "plc.1.smc3_20"));
    data_model_.SetUiToDataMap("reactora.label_HC4207", DataDef("plc.1.smc4_19", "plc.1.smc4_19", "plc.1.smc4_19"));
    data_model_.SetUiToDataMap("reactora.label_HC4208", DataDef("plc.1.smc4_20", "plc.1.smc4_20", "plc.1.smc4_20"));
    // reactorA - PFC
    data_model_.SetUiToDataMap("reactora.label_PICA4301", DataDef("mfcpfc.11.pv", "mfcpfc.11.sv", "mfcpfc.11.sv"));
    data_model_.SetUiToDataMap("reactora.label_PICA4302", DataDef("mfcpfc.12.pv", "mfcpfc.12.sv", "mfcpfc.12.sv"));
    data_model_.SetUiToDataMap("reactora.label_PICA4303", DataDef("mfcpfc.13.pv", "mfcpfc.13.sv", "mfcpfc.13.sv"));
    data_model_.SetUiToDataMap("reactora.label_PICA4304", DataDef("mfcpfc.14.pv", "mfcpfc.14.sv", "mfcpfc.14.sv"));
    data_model_.SetUiToDataMap("reactora.label_PICA4305", DataDef("mfcpfc.15.pv", "mfcpfc.15.sv", "mfcpfc.15.sv"));
    data_model_.SetUiToDataMap("reactora.label_PICA4306", DataDef("mfcpfc.16.pv", "mfcpfc.16.sv", "mfcpfc.16.sv"));
    data_model_.SetUiToDataMap("reactora.label_PICA4307", DataDef("mfcpfc.17.pv", "mfcpfc.17.sv", "mfcpfc.17.sv"));
    data_model_.SetUiToDataMap("reactora.label_PICA4308", DataDef("mfcpfc.18.pv", "mfcpfc.18.sv", "mfcpfc.18.sv"));
    // reactorA - bottom TC
    data_model_.SetUiToDataMap("reactora.label_TICA4101", DataDef("plc.1.temp1_pv", "plc.1.temp1_sv", "plc.1.temp1_sv"));
    data_model_.SetUiToDataMap("reactora.label_TICA4102", DataDef("plc.1.temp2_pv", "plc.1.temp2_sv", "plc.1.temp2_sv"));
    data_model_.SetUiToDataMap("reactora.label_TICA4103", DataDef("plc.1.temp3_pv", "plc.1.temp3_sv", "plc.1.temp3_sv"));
    data_model_.SetUiToDataMap("reactora.label_TICA4104", DataDef("plc.1.temp4_pv", "plc.1.temp4_sv", "plc.1.temp4_sv"));
    data_model_.SetUiToDataMap("reactora.label_TICA4105", DataDef("plc.1.temp5_pv", "plc.1.temp5_sv", "plc.1.temp5_sv"));
    data_model_.SetUiToDataMap("reactora.label_TICA4106", DataDef("plc.1.temp6_pv", "plc.1.temp6_sv", "plc.1.temp6_sv"));
    data_model_.SetUiToDataMap("reactora.label_TICA4107", DataDef("plc.1.temp7_pv", "plc.1.temp7_sv", "plc.1.temp7_sv"));
    data_model_.SetUiToDataMap("reactora.label_TICA4108", DataDef("plc.1.temp8_pv", "plc.1.temp8_sv", "plc.1.temp8_sv"));
    // reactorA - upper TC
    data_model_.SetUiToDataMap("reactora.label_TICA4121", DataDef("plc.1.temp17_pv", "plc.1.temp17_sv", "plc.1.temp17_sv"));
    data_model_.SetUiToDataMap("reactora.label_TICA4122", DataDef("plc.1.temp18_pv", "plc.1.temp18_sv", "plc.1.temp18_sv"));
    data_model_.SetUiToDataMap("reactora.label_TICA4123", DataDef("plc.1.temp19_pv", "plc.1.temp19_sv", "plc.1.temp19_sv"));
    data_model_.SetUiToDataMap("reactora.label_TICA4124", DataDef("plc.1.temp20_pv", "plc.1.temp20_sv", "plc.1.temp20_sv"));
    data_model_.SetUiToDataMap("reactora.label_TICA4125", DataDef("plc.1.temp21_pv", "plc.1.temp21_sv", "plc.1.temp21_sv"));
    data_model_.SetUiToDataMap("reactora.label_TICA4126", DataDef("plc.1.temp22_pv", "plc.1.temp22_sv", "plc.1.temp22_sv"));
    data_model_.SetUiToDataMap("reactora.label_TICA4127", DataDef("plc.1.temp23_pv", "plc.1.temp23_sv", "plc.1.temp23_sv"));
    data_model_.SetUiToDataMap("reactora.label_TICA4128", DataDef("plc.1.temp24_pv", "plc.1.temp24_sv", "plc.1.temp24_sv"));
    // reactorA - inner TI
    data_model_.SetUiToDataMap("reactora.label_TIA4141", DataDef("plc.1.temp33_pv", "plc.1.temp33_sv", "plc.1.temp33_sv"));
    data_model_.SetUiToDataMap("reactora.label_TIA4142", DataDef("plc.1.temp34_pv", "plc.1.temp34_sv", "plc.1.temp34_sv"));
    data_model_.SetUiToDataMap("reactora.label_TIA4143", DataDef("plc.1.temp35_pv", "plc.1.temp35_sv", "plc.1.temp35_sv"));
    data_model_.SetUiToDataMap("reactora.label_TIA4144", DataDef("plc.1.temp36_pv", "plc.1.temp36_sv", "plc.1.temp36_sv"));
    data_model_.SetUiToDataMap("reactora.label_TIA4145", DataDef("plc.1.temp37_pv", "plc.1.temp37_sv", "plc.1.temp37_sv"));
    data_model_.SetUiToDataMap("reactora.label_TIA4146", DataDef("plc.1.temp38_pv", "plc.1.temp38_sv", "plc.1.temp38_sv"));
    data_model_.SetUiToDataMap("reactora.label_TIA4147", DataDef("plc.1.temp39_pv", "plc.1.temp39_sv", "plc.1.temp39_sv"));
    data_model_.SetUiToDataMap("reactora.label_TIA4148", DataDef("plc.1.temp40_pv", "plc.1.temp40_sv", "plc.1.temp40_sv"));
    // reactorA - pipe TC
    data_model_.SetUiToDataMap("reactora.label_TICA4601", DataDef("plc.1.temp49_pv", "plc.1.temp49_sv", "plc.1.temp49_sv"));
    data_model_.SetUiToDataMap("reactora.label_TICA4602", DataDef("plc.1.temp50_pv", "plc.1.temp50_sv", "plc.1.temp50_sv"));
    data_model_.SetUiToDataMap("reactora.label_TICA4603", DataDef("plc.1.temp51_pv", "plc.1.temp51_sv", "plc.1.temp51_sv"));
    data_model_.SetUiToDataMap("reactora.label_TICA4604", DataDef("plc.1.temp52_pv", "plc.1.temp52_sv", "plc.1.temp52_sv"));
    data_model_.SetUiToDataMap("reactora.label_TICA4605", DataDef("plc.1.temp53_pv", "plc.1.temp53_sv", "plc.1.temp53_sv"));
    data_model_.SetUiToDataMap("reactora.label_TICA4606", DataDef("plc.1.temp54_pv", "plc.1.temp54_sv", "plc.1.temp54_sv"));
    data_model_.SetUiToDataMap("reactora.label_TICA4607", DataDef("plc.1.temp55_pv", "plc.1.temp55_sv", "plc.1.temp55_sv"));
    data_model_.SetUiToDataMap("reactora.label_TICA4608", DataDef("plc.1.temp56_pv", "plc.1.temp56_sv", "plc.1.temp56_sv"));
    // reactorB
    data_model_.SetUiToDataMap("reactorb.label_HC4609", DataDef("plc.1.smc14_17", "plc.1.smc14_17", "plc.1.smc14_17"));
    data_model_.SetUiToDataMap("reactorb.label_HC4610", DataDef("plc.1.smc14_18", "plc.1.smc14_18", "plc.1.smc14_18"));
    data_model_.SetUiToDataMap("reactorb.label_HC4611", DataDef("plc.1.smc14_19", "plc.1.smc14_19", "plc.1.smc14_19"));
    data_model_.SetUiToDataMap("reactorb.label_HC4612", DataDef("plc.1.smc14_20", "plc.1.smc14_20", "plc.1.smc14_20"));
    data_model_.SetUiToDataMap("reactorb.label_HC4613", DataDef("plc.1.smc13_5", "plc.1.smc13_5", "plc.1.smc13_5"));
    data_model_.SetUiToDataMap("reactorb.label_HC4614", DataDef("plc.1.smc13_6", "plc.1.smc13_6", "plc.1.smc13_6"));
    data_model_.SetUiToDataMap("reactorb.label_HC4615", DataDef("plc.1.smc13_7", "plc.1.smc13_7", "plc.1.smc13_7"));
    data_model_.SetUiToDataMap("reactorb.label_HC4616", DataDef("plc.1.smc13_8", "plc.1.smc13_8", "plc.1.smc13_8"));
    // reactorB - support cylinder
    data_model_.SetUiToDataMap("reactorb.label_HC4209", DataDef("plc.1.smc5_19", "plc.1.smc5_19", "plc.1.smc5_19"));
    data_model_.SetUiToDataMap("reactorb.label_HC4210", DataDef("plc.1.smc5_20", "plc.1.smc5_20", "plc.1.smc5_20"));
    data_model_.SetUiToDataMap("reactorb.label_HC4211", DataDef("plc.1.smc6_19", "plc.1.smc6_19", "plc.1.smc6_19"));
    data_model_.SetUiToDataMap("reactorb.label_HC4212", DataDef("plc.1.smc6_20", "plc.1.smc6_20", "plc.1.smc6_20"));
    data_model_.SetUiToDataMap("reactorb.label_HC4213", DataDef("plc.1.smc7_19", "plc.1.smc7_19", "plc.1.smc7_19"));
    data_model_.SetUiToDataMap("reactorb.label_HC4214", DataDef("plc.1.smc7_20", "plc.1.smc7_20", "plc.1.smc7_20"));
    data_model_.SetUiToDataMap("reactorb.label_HC4215", DataDef("plc.1.smc8_19", "plc.1.smc8_19", "plc.1.smc8_19"));
    data_model_.SetUiToDataMap("reactorb.label_HC4216", DataDef("plc.1.smc8_20", "plc.1.smc8_20", "plc.1.smc8_20"));
    // reactorB - PFC
    data_model_.SetUiToDataMap("reactorb.label_PICA4309", DataDef("mfcpfc.19.pv", "mfcpfc.19.sv", "mfcpfc.19.sv"));
    data_model_.SetUiToDataMap("reactorb.label_PICA4310", DataDef("mfcpfc.20.pv", "mfcpfc.20.sv", "mfcpfc.20.sv"));
    data_model_.SetUiToDataMap("reactorb.label_PICA4311", DataDef("mfcpfc.21.pv", "mfcpfc.21.sv", "mfcpfc.21.sv"));
    data_model_.SetUiToDataMap("reactorb.label_PICA4312", DataDef("mfcpfc.22.pv", "mfcpfc.22.sv", "mfcpfc.22.sv"));
    data_model_.SetUiToDataMap("reactorb.label_PICA4313", DataDef("mfcpfc.23.pv", "mfcpfc.23.sv", "mfcpfc.23.sv"));
    data_model_.SetUiToDataMap("reactorb.label_PICA4314", DataDef("mfcpfc.24.pv", "mfcpfc.24.sv", "mfcpfc.24.sv"));
    data_model_.SetUiToDataMap("reactorb.label_PICA4315", DataDef("mfcpfc.25.pv", "mfcpfc.25.sv", "mfcpfc.25.sv"));
    data_model_.SetUiToDataMap("reactorb.label_PICA4316", DataDef("mfcpfc.26.pv", "mfcpfc.26.sv", "mfcpfc.26.sv"));
    // reactorB - bottom TC
    data_model_.SetUiToDataMap("reactorb.label_TICA4109", DataDef("plc.1.temp9_pv", "plc.1.temp9_sv", "plc.1.temp9_sv"));
    data_model_.SetUiToDataMap("reactorb.label_TICA4110", DataDef("plc.1.temp10_pv", "plc.1.temp10_sv", "plc.1.temp10_sv"));
    data_model_.SetUiToDataMap("reactorb.label_TICA4111", DataDef("plc.1.temp11_pv", "plc.1.temp11_sv", "plc.1.temp11_sv"));
    data_model_.SetUiToDataMap("reactorb.label_TICA4112", DataDef("plc.1.temp12_pv", "plc.1.temp12_sv", "plc.1.temp12_sv"));
    data_model_.SetUiToDataMap("reactorb.label_TICA4113", DataDef("plc.1.temp13_pv", "plc.1.temp13_sv", "plc.1.temp13_sv"));
    data_model_.SetUiToDataMap("reactorb.label_TICA4114", DataDef("plc.1.temp14_pv", "plc.1.temp14_sv", "plc.1.temp14_sv"));
    data_model_.SetUiToDataMap("reactorb.label_TICA4115", DataDef("plc.1.temp15_pv", "plc.1.temp15_sv", "plc.1.temp15_sv"));
    data_model_.SetUiToDataMap("reactorb.label_TICA4116", DataDef("plc.1.temp16_pv", "plc.1.temp16_sv", "plc.1.temp16_sv"));
    // reactorB - upper TC
    data_model_.SetUiToDataMap("reactorb.label_TICA4129", DataDef("plc.1.temp25_pv", "plc.1.temp25_sv", "plc.1.temp25_sv"));
    data_model_.SetUiToDataMap("reactorb.label_TICA4130", DataDef("plc.1.temp26_pv", "plc.1.temp26_sv", "plc.1.temp26_sv"));
    data_model_.SetUiToDataMap("reactorb.label_TICA4131", DataDef("plc.1.temp27_pv", "plc.1.temp27_sv", "plc.1.temp27_sv"));
    data_model_.SetUiToDataMap("reactorb.label_TICA4132", DataDef("plc.1.temp28_pv", "plc.1.temp28_sv", "plc.1.temp28_sv"));
    data_model_.SetUiToDataMap("reactorb.label_TICA4133", DataDef("plc.1.temp29_pv", "plc.1.temp29_sv", "plc.1.temp29_sv"));
    data_model_.SetUiToDataMap("reactorb.label_TICA4134", DataDef("plc.1.temp30_pv", "plc.1.temp30_sv", "plc.1.temp30_sv"));
    data_model_.SetUiToDataMap("reactorb.label_TICA4135", DataDef("plc.1.temp31_pv", "plc.1.temp31_sv", "plc.1.temp31_sv"));
    data_model_.SetUiToDataMap("reactorb.label_TICA4136", DataDef("plc.1.temp32_pv", "plc.1.temp32_sv", "plc.1.temp32_sv"));
    // reactorB - pipe TC
    data_model_.SetUiToDataMap("reactorb.label_TICA4609", DataDef("plc.1.temp41_pv", "plc.1.temp41_sv", "plc.1.temp41_sv"));
    data_model_.SetUiToDataMap("reactorb.label_TICA4610", DataDef("plc.1.temp42_pv", "plc.1.temp42_sv", "plc.1.temp42_sv"));
    data_model_.SetUiToDataMap("reactorb.label_TICA4611", DataDef("plc.1.temp43_pv", "plc.1.temp43_sv", "plc.1.temp43_sv"));
    data_model_.SetUiToDataMap("reactorb.label_TICA4612", DataDef("plc.1.temp44_pv", "plc.1.temp44_sv", "plc.1.temp44_sv"));
    data_model_.SetUiToDataMap("reactorb.label_TICA4613", DataDef("plc.1.temp45_pv", "plc.1.temp45_sv", "plc.1.temp45_sv"));
    data_model_.SetUiToDataMap("reactorb.label_TICA4614", DataDef("plc.1.temp46_pv", "plc.1.temp46_sv", "plc.1.temp46_sv"));
    data_model_.SetUiToDataMap("reactorb.label_TICA4615", DataDef("plc.1.temp47_pv", "plc.1.temp47_sv", "plc.1.temp47_sv"));
    data_model_.SetUiToDataMap("reactorb.label_TICA4616", DataDef("plc.1.temp48_pv", "plc.1.temp48_sv", "plc.1.temp48_sv"));

    // gassampling
    data_model_.SetUiToDataMap("gassampling.label_HC4401", DataDef("plc.1.smc14_10", "plc.1.smc14_10", "plc.1.smc14_10"));
    data_model_.SetUiToDataMap("gassampling.label_HC4402", DataDef("plc.1.smc14_11", "plc.1.smc14_11", "plc.1.smc14_11"));
    data_model_.SetUiToDataMap("gassampling.label_HC4403", DataDef("plc.1.smc14_12", "plc.1.smc14_12", "plc.1.smc14_12"));
    data_model_.SetUiToDataMap("gassampling.label_HC5101", DataDef("plc.1.mvalve12_pv", "plc.1.mvalve12_pv", "plc.1.mvalve12_sv"));
    data_model_.SetUiToDataMap("gassampling.label_HC5102", DataDef("plc.1.smc10_21", "plc.1.smc10_21", "plc.1.smc10_21"));
    data_model_.SetUiToDataMap("gassampling.label_HC5103", DataDef("plc.1.smc10_22", "plc.1.smc10_22", "plc.1.smc10_22"));
    data_model_.SetUiToDataMap("gassampling.label_HC5104", DataDef("plc.1.smc10_23", "plc.1.smc10_23", "plc.1.smc10_23"));
    data_model_.SetUiToDataMap("gassampling.label_HC5201", DataDef("plc.1.mvalve13_pv", "plc.1.mvalve13_pv", "plc.1.mvalve13_sv"));
    data_model_.SetUiToDataMap("gassampling.label_HC5202", DataDef("plc.1.smc9_20", "plc.1.smc9_20", "plc.1.smc9_20"));
    data_model_.SetUiToDataMap("gassampling.label_HC5203", DataDef("plc.1.smc9_21", "plc.1.smc9_21", "plc.1.smc9_21"));
    data_model_.SetUiToDataMap("gassampling.label_HC5204", DataDef("plc.1.smc9_22", "plc.1.smc9_22", "plc.1.smc9_22"));
    // gassampling - TC
    data_model_.SetUiToDataMap("gassampling.label_TICA5501", DataDef("plc.1.temp49_pv", "plc.1.temp49_sv", "plc.1.temp49_sv"));
    data_model_.SetUiToDataMap("gassampling.label_TICA5502", DataDef("plc.1.temp50_pv", "plc.1.temp50_sv", "plc.1.temp50_sv"));
    data_model_.SetUiToDataMap("gassampling.label_TICA5105", DataDef("plc.1.temp57_pv", "plc.1.temp57_sv", "plc.1.temp57_sv"));
    data_model_.SetUiToDataMap("gassampling.label_TICA5205", DataDef("plc.1.temp58_pv", "plc.1.temp58_sv", "plc.1.temp58_sv"));
    data_model_.SetUiToDataMap("gassampling.label_TICA5101", DataDef("plc.1.temp59_pv", "plc.1.temp59_sv", "plc.1.temp59_sv"));
    data_model_.SetUiToDataMap("gassampling.label_TICA5201", DataDef("plc.1.temp60_pv", "plc.1.temp60_sv", "plc.1.temp60_sv"));

    // liquidcollection
    data_model_.SetUiToDataMap("liquidcollection.label_HC6101", DataDef("plc.1.smc11_19", "plc.1.smc11_19", "plc.1.smc11_19"));
    data_model_.SetUiToDataMap("liquidcollection.label_HC6102", DataDef("plc.1.mvalve14_pv", "plc.1.mvalve14_pv", "plc.1.mvalve14_sv"));
    data_model_.SetUiToDataMap("liquidcollection.label_HC6103", DataDef("plc.1.mvalve15_pv", "plc.1.mvalve15_pv", "plc.1.mvalve15_sv"));
    data_model_.SetUiToDataMap("liquidcollection.label_HC6104", DataDef("plc.1.mvalve16_pv", "plc.1.mvalve16_pv", "plc.1.mvalve16_sv"));
    data_model_.SetUiToDataMap("liquidcollection.label_HC6105", DataDef("plc.1.smc11_20", "plc.1.smc11_20", "plc.1.smc11_20"));
    data_model_.SetUiToDataMap("liquidcollection.label_HC6201", DataDef("plc.1.smc12_19", "plc.1.smc12_19", "plc.1.smc12_19"));
    data_model_.SetUiToDataMap("liquidcollection.label_HC6202", DataDef("plc.1.mvalve17_pv", "plc.1.mvalve17_pv", "plc.1.mvalve17_sv"));
    data_model_.SetUiToDataMap("liquidcollection.label_HC6203", DataDef("plc.1.mvalve18_pv", "plc.1.mvalve18_pv", "plc.1.mvalve18_sv"));
    data_model_.SetUiToDataMap("liquidcollection.label_HC6204", DataDef("plc.1.mvalve19_pv", "plc.1.mvalve19_pv", "plc.1.mvalve19_sv"));
    data_model_.SetUiToDataMap("liquidcollection.label_HC6205", DataDef("plc.1.smc12_20", "plc.1.smc12_20", "plc.1.smc12_20"));
    // liquidcollection - TC
    data_model_.SetUiToDataMap("liquidcollection.label_TICA6501", DataDef("plc.1.temp51_pv", "plc.1.temp51_sv", "plc.1.temp51_sv"));
    data_model_.SetUiToDataMap("liquidcollection.label_TICA6502", DataDef("plc.1.temp52_pv", "plc.1.temp52_sv", "plc.1.temp52_sv"));

    // liquidsamplinga
    data_model_.SetUiToDataMap("liquidsamplinga.label_HC7101", DataDef("plc.1.smc11_1", "plc.1.smc11_1", "plc.1.smc11_1"));
    data_model_.SetUiToDataMap("liquidsamplinga.label_HC7102", DataDef("plc.1.mvalve20_pv", "plc.1.mvalve20_pv", "plc.1.mvalve20_sv"));
    data_model_.SetUiToDataMap("liquidsamplinga.label_HC7201", DataDef("plc.1.smc11_2", "plc.1.smc11_2", "plc.1.smc11_2"));
    data_model_.SetUiToDataMap("liquidsamplinga.label_HC7202", DataDef("plc.1.smc11_3", "plc.1.smc11_3", "plc.1.smc11_3"));
    data_model_.SetUiToDataMap("liquidsamplinga.label_HC7203", DataDef("plc.1.smc11_4", "plc.1.smc11_4", "plc.1.smc11_4"));
    data_model_.SetUiToDataMap("liquidsamplinga.label_HC7204", DataDef("plc.1.smc11_5", "plc.1.smc11_5", "plc.1.smc11_5"));
    data_model_.SetUiToDataMap("liquidsamplinga.label_HC7205", DataDef("plc.1.smc11_6", "plc.1.smc11_6", "plc.1.smc11_6"));
    data_model_.SetUiToDataMap("liquidsamplinga.label_HC7206", DataDef("plc.1.smc11_7", "plc.1.smc11_7", "plc.1.smc11_7"));
    data_model_.SetUiToDataMap("liquidsamplinga.label_HC7207", DataDef("plc.1.smc11_8", "plc.1.smc11_8", "plc.1.smc11_8"));
    data_model_.SetUiToDataMap("liquidsamplinga.label_HC7208", DataDef("plc.1.smc11_9", "plc.1.smc11_9", "plc.1.smc11_9"));
    data_model_.SetUiToDataMap("liquidsamplinga.label_HC7209", DataDef("plc.1.smc11_10", "plc.1.smc11_10", "plc.1.smc11_10"));
    data_model_.SetUiToDataMap("liquidsamplinga.label_HC7210", DataDef("plc.1.smc11_11", "plc.1.smc11_11", "plc.1.smc11_11"));
    data_model_.SetUiToDataMap("liquidsamplinga.label_HC7211", DataDef("plc.1.smc11_12", "plc.1.smc11_12", "plc.1.smc11_12"));
    data_model_.SetUiToDataMap("liquidsamplinga.label_HC7212", DataDef("plc.1.smc11_13", "plc.1.smc11_13", "plc.1.smc11_13"));
    data_model_.SetUiToDataMap("liquidsamplinga.label_HC7213", DataDef("plc.1.smc11_14", "plc.1.smc11_14", "plc.1.smc11_14"));
    data_model_.SetUiToDataMap("liquidsamplinga.label_HC7214", DataDef("plc.1.smc11_15", "plc.1.smc11_15", "plc.1.smc11_15"));
    data_model_.SetUiToDataMap("liquidsamplinga.label_HC7215", DataDef("plc.1.smc11_16", "plc.1.smc11_16", "plc.1.smc11_16"));
    data_model_.SetUiToDataMap("liquidsamplinga.label_HC7216", DataDef("plc.1.smc11_17", "plc.1.smc11_17", "plc.1.smc11_17"));
    data_model_.SetUiToDataMap("liquidsamplinga.label_HC7217", DataDef("plc.1.smc11_18", "plc.1.smc11_18", "plc.1.smc11_18"));
    // liquidsamplinga - TC
    data_model_.SetUiToDataMap("liquidsamplinga.label_TICA7501", DataDef("plc.1.temp53_pv", "plc.1.temp53_sv", "plc.1.temp53_sv"));
    data_model_.SetUiToDataMap("liquidsamplinga.label_TICA7502", DataDef("plc.1.temp54_pv", "plc.1.temp54_sv", "plc.1.temp54_sv"));

    // liquidsamplingb
    data_model_.SetUiToDataMap("liquidsamplingb.label_HC7301", DataDef("plc.1.smc12_1", "plc.1.smc12_1", "plc.1.smc12_1"));
    data_model_.SetUiToDataMap("liquidsamplingb.label_HC7302", DataDef("plc.1.mvalve21_pv", "plc.1.mvalve21_pv", "plc.1.mvalve21_sv"));
    data_model_.SetUiToDataMap("liquidsamplingb.label_HC7401", DataDef("plc.1.smc12_2", "plc.1.smc12_2", "plc.1.smc12_2"));
    data_model_.SetUiToDataMap("liquidsamplingb.label_HC7402", DataDef("plc.1.smc12_3", "plc.1.smc12_3", "plc.1.smc12_3"));
    data_model_.SetUiToDataMap("liquidsamplingb.label_HC7403", DataDef("plc.1.smc12_4", "plc.1.smc12_4", "plc.1.smc12_4"));
    data_model_.SetUiToDataMap("liquidsamplingb.label_HC7404", DataDef("plc.1.smc12_5", "plc.1.smc12_5", "plc.1.smc12_5"));
    data_model_.SetUiToDataMap("liquidsamplingb.label_HC7405", DataDef("plc.1.smc12_6", "plc.1.smc12_6", "plc.1.smc12_6"));
    data_model_.SetUiToDataMap("liquidsamplingb.label_HC7406", DataDef("plc.1.smc12_7", "plc.1.smc12_7", "plc.1.smc12_7"));
    data_model_.SetUiToDataMap("liquidsamplingb.label_HC7407", DataDef("plc.1.smc12_8", "plc.1.smc12_8", "plc.1.smc12_8"));
    data_model_.SetUiToDataMap("liquidsamplingb.label_HC7408", DataDef("plc.1.smc12_9", "plc.1.smc12_9", "plc.1.smc12_9"));
    data_model_.SetUiToDataMap("liquidsamplingb.label_HC7409", DataDef("plc.1.smc12_10", "plc.1.smc12_10", "plc.1.smc12_10"));
    data_model_.SetUiToDataMap("liquidsamplingb.label_HC7410", DataDef("plc.1.smc12_11", "plc.1.smc12_11", "plc.1.smc12_11"));
    data_model_.SetUiToDataMap("liquidsamplingb.label_HC7411", DataDef("plc.1.smc12_12", "plc.1.smc12_12", "plc.1.smc12_12"));
    data_model_.SetUiToDataMap("liquidsamplingb.label_HC7412", DataDef("plc.1.smc12_13", "plc.1.smc12_13", "plc.1.smc12_13"));
    data_model_.SetUiToDataMap("liquidsamplingb.label_HC7413", DataDef("plc.1.smc12_14", "plc.1.smc12_14", "plc.1.smc12_14"));
    data_model_.SetUiToDataMap("liquidsamplingb.label_HC7414", DataDef("plc.1.smc12_15", "plc.1.smc12_15", "plc.1.smc12_15"));
    data_model_.SetUiToDataMap("liquidsamplingb.label_HC7415", DataDef("plc.1.smc12_16", "plc.1.smc12_16", "plc.1.smc12_16"));
    data_model_.SetUiToDataMap("liquidsamplingb.label_HC7416", DataDef("plc.1.smc12_17", "plc.1.smc12_17", "plc.1.smc12_17"));
    data_model_.SetUiToDataMap("liquidsamplingb.label_HC7417", DataDef("plc.1.smc12_18", "plc.1.smc12_18", "plc.1.smc12_18"));
    // liquidsamplingb - TC
    data_model_.SetUiToDataMap("liquidsamplingb.label_TICA7503", DataDef("plc.1.temp55_pv", "plc.1.temp55_sv", "plc.1.temp55_sv"));
    data_model_.SetUiToDataMap("liquidsamplingb.label_TICA7504", DataDef("plc.1.temp56_pv", "plc.1.temp56_sv", "plc.1.temp56_sv"));
    // motorcontrol - device id: cylinder16 1-8 channel
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_1_enable", DataDef("cylinder16.1.srv_on", "cylinder16.1.srv_on", "cylinder16.1.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_1_disable", DataDef("cylinder16.1.srv_on", "cylinder16.1.srv_on", "cylinder16.1.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_1_alarm_clear", DataDef("cylinder16.1.a_clr", "cylinder16.1.a_clr", "cylinder16.1.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.cell_1_speed", DataDef("cylinder16.1.block_v0", "cylinder16.1.block_v0", "cylinder16.1.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.cell_1_sv", DataDef("cylinder16.1.block_data0", "cylinder16.1.block_data0", "cylinder16.1.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_1_start", DataDef("cylinder16.1.stb", "cylinder16.1.stb", "cylinder16.1.stb"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_1_stop", DataDef("cylinder16.1.s_stop", "cylinder16.1.s_stop", "cylinder16.1.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_2_enable", DataDef("cylinder16.2.srv_on", "cylinder16.2.srv_on", "cylinder16.2.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_2_disable", DataDef("cylinder16.2.srv_on", "cylinder16.2.srv_on", "cylinder16.2.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_2_alarm_clear", DataDef("cylinder16.2.a_clr", "cylinder16.2.a_clr", "cylinder16.2.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.cell_2_speed", DataDef("cylinder16.2.block_v0", "cylinder16.2.block_v0", "cylinder16.2.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.cell_2_sv", DataDef("cylinder16.2.block_data0", "cylinder16.2.block_data0", "cylinder16.2.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_2_start", DataDef("cylinder16.2.stb", "cylinder16.2.stb", "cylinder16.2.stb"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_2_stop", DataDef("cylinder16.2.s_stop", "cylinder16.2.s_stop", "cylinder16.2.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_3_enable", DataDef("cylinder16.3.srv_on", "cylinder16.3.srv_on", "cylinder16.3.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_3_disable", DataDef("cylinder16.3.srv_on", "cylinder16.3.srv_on", "cylinder16.3.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_3_alarm_clear", DataDef("cylinder16.3.a_clr", "cylinder16.3.a_clr", "cylinder16.3.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.cell_3_speed", DataDef("cylinder16.3.block_v0", "cylinder16.3.block_v0", "cylinder16.3.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.cell_3_sv", DataDef("cylinder16.3.block_data0", "cylinder16.3.block_data0", "cylinder16.3.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_3_start", DataDef("cylinder16.3.stb", "cylinder16.3.stb", "cylinder16.3.stb"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_3_stop", DataDef("cylinder16.3.s_stop", "cylinder16.3.s_stop", "cylinder16.3.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_4_enable", DataDef("cylinder16.4.srv_on", "cylinder16.4.srv_on", "cylinder16.4.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_4_disable", DataDef("cylinder16.4.srv_on", "cylinder16.4.srv_on", "cylinder16.4.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_4_alarm_clear", DataDef("cylinder16.4.a_clr", "cylinder16.4.a_clr", "cylinder16.4.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.cell_4_speed", DataDef("cylinder16.4.block_v0", "cylinder16.4.block_v0", "cylinder16.4.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.cell_4_sv", DataDef("cylinder16.4.block_data0", "cylinder16.4.block_data0", "cylinder16.4.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_4_start", DataDef("cylinder16.4.stb", "cylinder16.4.stb", "cylinder16.4.stb"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_4_stop", DataDef("cylinder16.4.s_stop", "cylinder16.4.s_stop", "cylinder16.4.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_5_enable", DataDef("cylinder16.5.srv_on", "cylinder16.5.srv_on", "cylinder16.5.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_5_disable", DataDef("cylinder16.5.srv_on", "cylinder16.5.srv_on", "cylinder16.5.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_5_alarm_clear", DataDef("cylinder16.5.a_clr", "cylinder16.5.a_clr", "cylinder16.5.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.cell_5_speed", DataDef("cylinder16.5.block_v0", "cylinder16.5.block_v0", "cylinder16.5.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.cell_5_sv", DataDef("cylinder16.5.block_data0", "cylinder16.5.block_data0", "cylinder16.5.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_5_start", DataDef("cylinder16.5.stb", "cylinder16.5.stb", "cylinder16.5.stb"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_5_stop", DataDef("cylinder16.5.s_stop", "cylinder16.5.s_stop", "cylinder16.5.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_6_enable", DataDef("cylinder16.6.srv_on", "cylinder16.6.srv_on", "cylinder16.6.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_6_disable", DataDef("cylinder16.6.srv_on", "cylinder16.6.srv_on", "cylinder16.6.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_6_alarm_clear", DataDef("cylinder16.6.a_clr", "cylinder16.6.a_clr", "cylinder16.6.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.cell_6_speed", DataDef("cylinder16.6.block_v0", "cylinder16.6.block_v0", "cylinder16.6.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.cell_6_sv", DataDef("cylinder16.6.block_data0", "cylinder16.6.block_data0", "cylinder16.6.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_6_start", DataDef("cylinder16.6.stb", "cylinder16.6.stb", "cylinder16.6.stb"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_6_stop", DataDef("cylinder16.6.s_stop", "cylinder16.6.s_stop", "cylinder16.6.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_7_enable", DataDef("cylinder16.7.srv_on", "cylinder16.7.srv_on", "cylinder16.7.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_7_disable", DataDef("cylinder16.7.srv_on", "cylinder16.7.srv_on", "cylinder16.7.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_7_alarm_clear", DataDef("cylinder16.7.a_clr", "cylinder16.7.a_clr", "cylinder16.7.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.cell_7_speed", DataDef("cylinder16.7.block_v0", "cylinder16.7.block_v0", "cylinder16.7.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.cell_7_sv", DataDef("cylinder16.7.block_data0", "cylinder16.7.block_data0", "cylinder16.7.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_7_start", DataDef("cylinder16.7.stb", "cylinder16.7.stb", "cylinder16.7.stb"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_7_stop", DataDef("cylinder16.7.s_stop", "cylinder16.7.s_stop", "cylinder16.7.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_8_enable", DataDef("cylinder16.8.srv_on", "cylinder16.8.srv_on", "cylinder16.8.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_8_disable", DataDef("cylinder16.8.srv_on", "cylinder16.8.srv_on", "cylinder16.8.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_8_alarm_clear", DataDef("cylinder16.8.a_clr", "cylinder16.8.a_clr", "cylinder16.8.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.cell_8_speed", DataDef("cylinder16.8.block_v0", "cylinder16.8.block_v0", "cylinder16.8.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.cell_8_sv", DataDef("cylinder16.8.block_data0", "cylinder16.8.block_data0", "cylinder16.8.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_8_start", DataDef("cylinder16.8.stb", "cylinder16.8.stb", "cylinder16.8.stb"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_8_stop", DataDef("cylinder16.8.s_stop", "cylinder16.8.s_stop", "cylinder16.8.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_9_enable", DataDef("cylinder16.9.srv_on", "cylinder16.9.srv_on", "cylinder16.9.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_9_disable", DataDef("cylinder16.9.srv_on", "cylinder16.9.srv_on", "cylinder16.9.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_9_alarm_clear", DataDef("cylinder16.9.a_clr", "cylinder16.9.a_clr", "cylinder16.9.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.cell_9_speed", DataDef("cylinder16.9.block_v0", "cylinder16.9.block_v0", "cylinder16.9.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.cell_9_sv", DataDef("cylinder16.9.block_data0", "cylinder16.9.block_data0", "cylinder16.9.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_9_start", DataDef("cylinder16.9.stb", "cylinder16.9.stb", "cylinder16.9.stb"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_9_stop", DataDef("cylinder16.9.s_stop", "cylinder16.9.s_stop", "cylinder16.9.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_10_enable", DataDef("cylinder16.10.srv_on", "cylinder16.10.srv_on", "cylinder16.10.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_10_disable", DataDef("cylinder16.10.srv_on", "cylinder16.10.srv_on", "cylinder16.10.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_10_alarm_clear", DataDef("cylinder16.10.a_clr", "cylinder16.10.a_clr", "cylinder16.10.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.cell_10_speed", DataDef("cylinder16.10.block_v0", "cylinder16.10.block_v0", "cylinder16.10.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.cell_10_sv", DataDef("cylinder16.10.block_data0", "cylinder16.10.block_data0", "cylinder16.10.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_10_start", DataDef("cylinder16.10.stb", "cylinder16.10.stb", "cylinder16.10.stb"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_10_stop", DataDef("cylinder16.10.s_stop", "cylinder16.10.s_stop", "cylinder16.10.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_11_enable", DataDef("cylinder16.11.srv_on", "cylinder16.11.srv_on", "cylinder16.11.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_11_disable", DataDef("cylinder16.11.srv_on", "cylinder16.11.srv_on", "cylinder16.11.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_11_alarm_clear", DataDef("cylinder16.11.a_clr", "cylinder16.11.a_clr", "cylinder16.11.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.cell_11_speed", DataDef("cylinder16.11.block_v0", "cylinder16.11.block_v0", "cylinder16.11.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.cell_11_sv", DataDef("cylinder16.11.block_data0", "cylinder16.11.block_data0", "cylinder16.11.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_11_start", DataDef("cylinder16.11.stb", "cylinder16.11.stb", "cylinder16.11.stb"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_11_stop", DataDef("cylinder16.11.s_stop", "cylinder16.11.s_stop", "cylinder16.11.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_12_enable", DataDef("cylinder16.12.srv_on", "cylinder16.12.srv_on", "cylinder16.12.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_12_disable", DataDef("cylinder16.12.srv_on", "cylinder16.12.srv_on", "cylinder16.12.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_12_alarm_clear", DataDef("cylinder16.12.a_clr", "cylinder16.12.a_clr", "cylinder16.12.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.cell_12_speed", DataDef("cylinder16.12.block_v0", "cylinder16.12.block_v0", "cylinder16.12.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.cell_12_sv", DataDef("cylinder16.12.block_data0", "cylinder16.12.block_data0", "cylinder16.12.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_12_start", DataDef("cylinder16.12.stb", "cylinder16.12.stb", "cylinder16.12.stb"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_12_stop", DataDef("cylinder16.12.s_stop", "cylinder16.12.s_stop", "cylinder16.12.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_13_enable", DataDef("cylinder16.13.srv_on", "cylinder16.13.srv_on", "cylinder16.13.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_13_disable", DataDef("cylinder16.13.srv_on", "cylinder16.13.srv_on", "cylinder16.13.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_13_alarm_clear", DataDef("cylinder16.13.a_clr", "cylinder16.13.a_clr", "cylinder16.13.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.cell_13_speed", DataDef("cylinder16.13.block_v0", "cylinder16.13.block_v0", "cylinder16.13.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.cell_13_sv", DataDef("cylinder16.13.block_data0", "cylinder16.13.block_data0", "cylinder16.13.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_13_start", DataDef("cylinder16.13.stb", "cylinder16.13.stb", "cylinder16.13.stb"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_13_stop", DataDef("cylinder16.13.s_stop", "cylinder16.13.s_stop", "cylinder16.13.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_14_enable", DataDef("cylinder16.14.srv_on", "cylinder16.14.srv_on", "cylinder16.14.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_14_disable", DataDef("cylinder16.14.srv_on", "cylinder16.14.srv_on", "cylinder16.14.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_14_alarm_clear", DataDef("cylinder16.14.a_clr", "cylinder16.14.a_clr", "cylinder16.14.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.cell_14_speed", DataDef("cylinder16.14.block_v0", "cylinder16.14.block_v0", "cylinder16.14.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.cell_14_sv", DataDef("cylinder16.14.block_data0", "cylinder16.14.block_data0", "cylinder16.14.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_14_start", DataDef("cylinder16.14.stb", "cylinder16.14.stb", "cylinder16.14.stb"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_14_stop", DataDef("cylinder16.14.s_stop", "cylinder16.14.s_stop", "cylinder16.14.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_15_enable", DataDef("cylinder16.15.srv_on", "cylinder16.15.srv_on", "cylinder16.15.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_15_disable", DataDef("cylinder16.15.srv_on", "cylinder16.15.srv_on", "cylinder16.15.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_15_alarm_clear", DataDef("cylinder16.15.a_clr", "cylinder16.15.a_clr", "cylinder16.15.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.cell_15_speed", DataDef("cylinder16.15.block_v0", "cylinder16.15.block_v0", "cylinder16.15.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.cell_15_sv", DataDef("cylinder16.15.block_data0", "cylinder16.15.block_data0", "cylinder16.15.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_15_start", DataDef("cylinder16.15.stb", "cylinder16.15.stb", "cylinder16.15.stb"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_15_stop", DataDef("cylinder16.15.s_stop", "cylinder16.15.s_stop", "cylinder16.15.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_16_enable", DataDef("cylinder16.16.srv_on", "cylinder16.16.srv_on", "cylinder16.16.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_16_disable", DataDef("cylinder16.16.srv_on", "cylinder16.16.srv_on", "cylinder16.16.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_16_alarm_clear", DataDef("cylinder16.16.a_clr", "cylinder16.16.a_clr", "cylinder16.16.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.cell_16_speed", DataDef("cylinder16.16.block_v0", "cylinder16.16.block_v0", "cylinder16.16.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.cell_16_sv", DataDef("cylinder16.16.block_data0", "cylinder16.16.block_data0", "cylinder16.16.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_16_start", DataDef("cylinder16.16.stb", "cylinder16.16.stb", "cylinder16.16.stb"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder16.button_16_stop", DataDef("cylinder16.16.s_stop", "cylinder16.16.s_stop", "cylinder16.16.s_stop"));

    // motorcontrol - device id: cylinder16 9-16 channel
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_1_enable", DataDef("cylinder32.1.srv_on", "cylinder32.1.srv_on", "cylinder32.1.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_1_disable", DataDef("cylinder32.1.srv_on", "cylinder32.1.srv_on", "cylinder32.1.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_1_alarm_clear", DataDef("cylinder32.1.a_clr", "cylinder32.1.a_clr", "cylinder32.1.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.cell_1_speed", DataDef("cylinder32.1.block_v0", "cylinder32.1.block_v0", "cylinder32.1.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.cell_1_sv", DataDef("cylinder32.1.block_data0", "cylinder32.1.block_data0", "cylinder32.1.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_1_start", DataDef("cylinder32.1.stb", "cylinder32.1.stb", "cylinder32.1.stb"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_1_stop", DataDef("cylinder32.1.s_stop", "cylinder32.1.s_stop", "cylinder32.1.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_2_enable", DataDef("cylinder32.2.srv_on", "cylinder32.2.srv_on", "cylinder32.2.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_2_disable", DataDef("cylinder32.2.srv_on", "cylinder32.2.srv_on", "cylinder32.2.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_2_alarm_clear", DataDef("cylinder32.2.a_clr", "cylinder32.2.a_clr", "cylinder32.2.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.cell_2_speed", DataDef("cylinder32.2.block_v0", "cylinder32.2.block_v0", "cylinder32.2.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.cell_2_sv", DataDef("cylinder32.2.block_data0", "cylinder32.2.block_data0", "cylinder32.2.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_2_start", DataDef("cylinder32.2.stb", "cylinder32.2.stb", "cylinder32.2.stb"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_2_stop", DataDef("cylinder32.2.s_stop", "cylinder32.2.s_stop", "cylinder32.2.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_3_enable", DataDef("cylinder32.3.srv_on", "cylinder32.3.srv_on", "cylinder32.3.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_3_disable", DataDef("cylinder32.3.srv_on", "cylinder32.3.srv_on", "cylinder32.3.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_3_alarm_clear", DataDef("cylinder32.3.a_clr", "cylinder32.3.a_clr", "cylinder32.3.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.cell_3_speed", DataDef("cylinder32.3.block_v0", "cylinder32.3.block_v0", "cylinder32.3.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.cell_3_sv", DataDef("cylinder32.3.block_data0", "cylinder32.3.block_data0", "cylinder32.3.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_3_start", DataDef("cylinder32.3.stb", "cylinder32.3.stb", "cylinder32.3.stb"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_3_stop", DataDef("cylinder32.3.s_stop", "cylinder32.3.s_stop", "cylinder32.3.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_4_enable", DataDef("cylinder32.4.srv_on", "cylinder32.4.srv_on", "cylinder32.4.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_4_disable", DataDef("cylinder32.4.srv_on", "cylinder32.4.srv_on", "cylinder32.4.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_4_alarm_clear", DataDef("cylinder32.4.a_clr", "cylinder32.4.a_clr", "cylinder32.4.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.cell_4_speed", DataDef("cylinder32.4.block_v0", "cylinder32.4.block_v0", "cylinder32.4.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.cell_4_sv", DataDef("cylinder32.4.block_data0", "cylinder32.4.block_data0", "cylinder32.4.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_4_start", DataDef("cylinder32.4.stb", "cylinder32.4.stb", "cylinder32.4.stb"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_4_stop", DataDef("cylinder32.4.s_stop", "cylinder32.4.s_stop", "cylinder32.4.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_5_enable", DataDef("cylinder32.5.srv_on", "cylinder32.5.srv_on", "cylinder32.5.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_5_disable", DataDef("cylinder32.5.srv_on", "cylinder32.5.srv_on", "cylinder32.5.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_5_alarm_clear", DataDef("cylinder32.5.a_clr", "cylinder32.5.a_clr", "cylinder32.5.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.cell_5_speed", DataDef("cylinder32.5.block_v0", "cylinder32.5.block_v0", "cylinder32.5.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.cell_5_sv", DataDef("cylinder32.5.block_data0", "cylinder32.5.block_data0", "cylinder32.5.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_5_start", DataDef("cylinder32.5.stb", "cylinder32.5.stb", "cylinder32.5.stb"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_5_stop", DataDef("cylinder32.5.s_stop", "cylinder32.5.s_stop", "cylinder32.5.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_6_enable", DataDef("cylinder32.6.srv_on", "cylinder32.6.srv_on", "cylinder32.6.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_6_disable", DataDef("cylinder32.6.srv_on", "cylinder32.6.srv_on", "cylinder32.6.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_6_alarm_clear", DataDef("cylinder32.6.a_clr", "cylinder32.6.a_clr", "cylinder32.6.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.cell_6_speed", DataDef("cylinder32.6.block_v0", "cylinder32.6.block_v0", "cylinder32.6.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.cell_6_sv", DataDef("cylinder32.6.block_data0", "cylinder32.6.block_data0", "cylinder32.6.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_6_start", DataDef("cylinder32.6.stb", "cylinder32.6.stb", "cylinder32.6.stb"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_6_stop", DataDef("cylinder32.6.s_stop", "cylinder32.6.s_stop", "cylinder32.6.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_7_enable", DataDef("cylinder32.7.srv_on", "cylinder32.7.srv_on", "cylinder32.7.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_7_disable", DataDef("cylinder32.7.srv_on", "cylinder32.7.srv_on", "cylinder32.7.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_7_alarm_clear", DataDef("cylinder32.7.a_clr", "cylinder32.7.a_clr", "cylinder32.7.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.cell_7_speed", DataDef("cylinder32.7.block_v0", "cylinder32.7.block_v0", "cylinder32.7.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.cell_7_sv", DataDef("cylinder32.7.block_data0", "cylinder32.7.block_data0", "cylinder32.7.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_7_start", DataDef("cylinder32.7.stb", "cylinder32.7.stb", "cylinder32.7.stb"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_7_stop", DataDef("cylinder32.7.s_stop", "cylinder32.7.s_stop", "cylinder32.7.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_8_enable", DataDef("cylinder32.8.srv_on", "cylinder32.8.srv_on", "cylinder32.8.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_8_disable", DataDef("cylinder32.8.srv_on", "cylinder32.8.srv_on", "cylinder32.8.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_8_alarm_clear", DataDef("cylinder32.8.a_clr", "cylinder32.8.a_clr", "cylinder32.8.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.cell_8_speed", DataDef("cylinder32.8.block_v0", "cylinder32.8.block_v0", "cylinder32.8.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.cell_8_sv", DataDef("cylinder32.8.block_data0", "cylinder32.8.block_data0", "cylinder32.8.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_8_start", DataDef("cylinder32.8.stb", "cylinder32.8.stb", "cylinder32.8.stb"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_8_stop", DataDef("cylinder32.8.s_stop", "cylinder32.8.s_stop", "cylinder32.8.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_9_enable", DataDef("cylinder32.9.srv_on", "cylinder32.9.srv_on", "cylinder32.9.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_9_disable", DataDef("cylinder32.9.srv_on", "cylinder32.9.srv_on", "cylinder32.9.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_9_alarm_clear", DataDef("cylinder32.9.a_clr", "cylinder32.9.a_clr", "cylinder32.9.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.cell_9_speed", DataDef("cylinder32.9.block_v0", "cylinder32.9.block_v0", "cylinder32.9.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.cell_9_sv", DataDef("cylinder32.9.block_data0", "cylinder32.9.block_data0", "cylinder32.9.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_9_start", DataDef("cylinder32.9.stb", "cylinder32.9.stb", "cylinder32.9.stb"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_9_stop", DataDef("cylinder32.9.s_stop", "cylinder32.9.s_stop", "cylinder32.9.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_10_enable", DataDef("cylinder32.10.srv_on", "cylinder32.10.srv_on", "cylinder32.10.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_10_disable", DataDef("cylinder32.10.srv_on", "cylinder32.10.srv_on", "cylinder32.10.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_10_alarm_clear", DataDef("cylinder32.10.a_clr", "cylinder32.10.a_clr", "cylinder32.10.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.cell_10_speed", DataDef("cylinder32.10.block_v0", "cylinder32.10.block_v0", "cylinder32.10.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.cell_10_sv", DataDef("cylinder32.10.block_data0", "cylinder32.10.block_data0", "cylinder32.10.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_10_start", DataDef("cylinder32.10.stb", "cylinder32.10.stb", "cylinder32.10.stb"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_10_stop", DataDef("cylinder32.10.s_stop", "cylinder32.10.s_stop", "cylinder32.10.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_11_enable", DataDef("cylinder32.11.srv_on", "cylinder32.11.srv_on", "cylinder32.11.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_11_disable", DataDef("cylinder32.11.srv_on", "cylinder32.11.srv_on", "cylinder32.11.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_11_alarm_clear", DataDef("cylinder32.11.a_clr", "cylinder32.11.a_clr", "cylinder32.11.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.cell_11_speed", DataDef("cylinder32.11.block_v0", "cylinder32.11.block_v0", "cylinder32.11.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.cell_11_sv", DataDef("cylinder32.11.block_data0", "cylinder32.11.block_data0", "cylinder32.11.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_11_start", DataDef("cylinder32.11.stb", "cylinder32.11.stb", "cylinder32.11.stb"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_11_stop", DataDef("cylinder32.11.s_stop", "cylinder32.11.s_stop", "cylinder32.11.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_12_enable", DataDef("cylinder32.12.srv_on", "cylinder32.12.srv_on", "cylinder32.12.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_12_disable", DataDef("cylinder32.12.srv_on", "cylinder32.12.srv_on", "cylinder32.12.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_12_alarm_clear", DataDef("cylinder32.12.a_clr", "cylinder32.12.a_clr", "cylinder32.12.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.cell_12_speed", DataDef("cylinder32.12.block_v0", "cylinder32.12.block_v0", "cylinder32.12.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.cell_12_sv", DataDef("cylinder32.12.block_data0", "cylinder32.12.block_data0", "cylinder32.12.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_12_start", DataDef("cylinder32.12.stb", "cylinder32.12.stb", "cylinder32.12.stb"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_12_stop", DataDef("cylinder32.12.s_stop", "cylinder32.12.s_stop", "cylinder32.12.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_13_enable", DataDef("cylinder32.13.srv_on", "cylinder32.13.srv_on", "cylinder32.13.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_13_disable", DataDef("cylinder32.13.srv_on", "cylinder32.13.srv_on", "cylinder32.13.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_13_alarm_clear", DataDef("cylinder32.13.a_clr", "cylinder32.13.a_clr", "cylinder32.13.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.cell_13_speed", DataDef("cylinder32.13.block_v0", "cylinder32.13.block_v0", "cylinder32.13.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.cell_13_sv", DataDef("cylinder32.13.block_data0", "cylinder32.13.block_data0", "cylinder32.13.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_13_start", DataDef("cylinder32.13.stb", "cylinder32.13.stb", "cylinder32.13.stb"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_13_stop", DataDef("cylinder32.13.s_stop", "cylinder32.13.s_stop", "cylinder32.13.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_14_enable", DataDef("cylinder32.14.srv_on", "cylinder32.14.srv_on", "cylinder32.14.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_14_disable", DataDef("cylinder32.14.srv_on", "cylinder32.14.srv_on", "cylinder32.14.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_14_alarm_clear", DataDef("cylinder32.14.a_clr", "cylinder32.14.a_clr", "cylinder32.14.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.cell_14_speed", DataDef("cylinder32.14.block_v0", "cylinder32.14.block_v0", "cylinder32.14.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.cell_14_sv", DataDef("cylinder32.14.block_data0", "cylinder32.14.block_data0", "cylinder32.14.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_14_start", DataDef("cylinder32.14.stb", "cylinder32.14.stb", "cylinder32.14.stb"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_14_stop", DataDef("cylinder32.14.s_stop", "cylinder32.14.s_stop", "cylinder32.14.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_15_enable", DataDef("cylinder32.15.srv_on", "cylinder32.15.srv_on", "cylinder32.15.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_15_disable", DataDef("cylinder32.15.srv_on", "cylinder32.15.srv_on", "cylinder32.15.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_15_alarm_clear", DataDef("cylinder32.15.a_clr", "cylinder32.15.a_clr", "cylinder32.15.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.cell_15_speed", DataDef("cylinder32.15.block_v0", "cylinder32.15.block_v0", "cylinder32.15.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.cell_15_sv", DataDef("cylinder32.15.block_data0", "cylinder32.15.block_data0", "cylinder32.15.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_15_start", DataDef("cylinder32.15.stb", "cylinder32.15.stb", "cylinder32.15.stb"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_15_stop", DataDef("cylinder32.15.s_stop", "cylinder32.15.s_stop", "cylinder32.15.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_16_enable", DataDef("cylinder32.16.srv_on", "cylinder32.16.srv_on", "cylinder32.16.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_16_disable", DataDef("cylinder32.16.srv_on", "cylinder32.16.srv_on", "cylinder32.16.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_16_alarm_clear", DataDef("cylinder32.16.a_clr", "cylinder32.16.a_clr", "cylinder32.16.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.cell_16_speed", DataDef("cylinder32.16.block_v0", "cylinder32.16.block_v0", "cylinder32.16.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.cell_16_sv", DataDef("cylinder32.16.block_data0", "cylinder32.16.block_data0", "cylinder32.16.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_16_start", DataDef("cylinder32.16.stb", "cylinder32.16.stb", "cylinder32.16.stb"));
    data_model_.SetUiToDataMap("motorcontrol_cylinder32.button_16_stop", DataDef("cylinder32.16.s_stop", "cylinder32.16.s_stop", "cylinder32.16.s_stop"));

    // motorcontrol - device id: reactor16 1-16 channel
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_1_enable", DataDef("reactor16.1.srv_on", "reactor16.1.srv_on", "reactor16.1.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_1_disable", DataDef("reactor16.1.srv_on", "reactor16.1.srv_on", "reactor16.1.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_1_alarm_clear", DataDef("reactor16.1.a_clr", "reactor16.1.a_clr", "reactor16.1.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.cell_1_speed", DataDef("reactor16.1.block_v0", "reactor16.1.block_v0", "reactor16.1.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.cell_1_sv", DataDef("reactor16.1.block_data0", "reactor16.1.block_data0", "reactor16.1.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_1_start", DataDef("reactor16.1.stb", "reactor16.1.stb", "reactor16.1.stb"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_1_stop", DataDef("reactor16.1.s_stop", "reactor16.1.s_stop", "reactor16.1.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_2_enable", DataDef("reactor16.2.srv_on", "reactor16.2.srv_on", "reactor16.2.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_2_disable", DataDef("reactor16.2.srv_on", "reactor16.2.srv_on", "reactor16.2.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_2_alarm_clear", DataDef("reactor16.2.a_clr", "reactor16.2.a_clr", "reactor16.2.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.cell_2_speed", DataDef("reactor16.2.block_v0", "reactor16.2.block_v0", "reactor16.2.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.cell_2_sv", DataDef("reactor16.2.block_data0", "reactor16.2.block_data0", "reactor16.2.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_2_start", DataDef("reactor16.2.stb", "reactor16.2.stb", "reactor16.2.stb"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_2_stop", DataDef("reactor16.2.s_stop", "reactor16.2.s_stop", "reactor16.2.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_3_enable", DataDef("reactor16.3.srv_on", "reactor16.3.srv_on", "reactor16.3.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_3_disable", DataDef("reactor16.3.srv_on", "reactor16.3.srv_on", "reactor16.3.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_3_alarm_clear", DataDef("reactor16.3.a_clr", "reactor16.3.a_clr", "reactor16.3.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.cell_3_speed", DataDef("reactor16.3.block_v0", "reactor16.3.block_v0", "reactor16.3.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.cell_3_sv", DataDef("reactor16.3.block_data0", "reactor16.3.block_data0", "reactor16.3.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_3_start", DataDef("reactor16.3.stb", "reactor16.3.stb", "reactor16.3.stb"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_3_stop", DataDef("reactor16.3.s_stop", "reactor16.3.s_stop", "reactor16.3.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_4_enable", DataDef("reactor16.4.srv_on", "reactor16.4.srv_on", "reactor16.4.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_4_disable", DataDef("reactor16.4.srv_on", "reactor16.4.srv_on", "reactor16.4.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_4_alarm_clear", DataDef("reactor16.4.a_clr", "reactor16.4.a_clr", "reactor16.4.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.cell_4_speed", DataDef("reactor16.4.block_v0", "reactor16.4.block_v0", "reactor16.4.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.cell_4_sv", DataDef("reactor16.4.block_data0", "reactor16.4.block_data0", "reactor16.4.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_4_start", DataDef("reactor16.4.stb", "reactor16.4.stb", "reactor16.4.stb"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_4_stop", DataDef("reactor16.4.s_stop", "reactor16.4.s_stop", "reactor16.4.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_5_enable", DataDef("reactor16.5.srv_on", "reactor16.5.srv_on", "reactor16.5.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_5_disable", DataDef("reactor16.5.srv_on", "reactor16.5.srv_on", "reactor16.5.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_5_alarm_clear", DataDef("reactor16.5.a_clr", "reactor16.5.a_clr", "reactor16.5.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.cell_5_speed", DataDef("reactor16.5.block_v0", "reactor16.5.block_v0", "reactor16.5.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.cell_5_sv", DataDef("reactor16.5.block_data0", "reactor16.5.block_data0", "reactor16.5.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_5_start", DataDef("reactor16.5.stb", "reactor16.5.stb", "reactor16.5.stb"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_5_stop", DataDef("reactor16.5.s_stop", "reactor16.5.s_stop", "reactor16.5.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_6_enable", DataDef("reactor16.6.srv_on", "reactor16.6.srv_on", "reactor16.6.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_6_disable", DataDef("reactor16.6.srv_on", "reactor16.6.srv_on", "reactor16.6.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_6_alarm_clear", DataDef("reactor16.6.a_clr", "reactor16.6.a_clr", "reactor16.6.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.cell_6_speed", DataDef("reactor16.6.block_v0", "reactor16.6.block_v0", "reactor16.6.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.cell_6_sv", DataDef("reactor16.6.block_data0", "reactor16.6.block_data0", "reactor16.6.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_6_start", DataDef("reactor16.6.stb", "reactor16.6.stb", "reactor16.6.stb"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_6_stop", DataDef("reactor16.6.s_stop", "reactor16.6.s_stop", "reactor16.6.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_7_enable", DataDef("reactor16.7.srv_on", "reactor16.7.srv_on", "reactor16.7.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_7_disable", DataDef("reactor16.7.srv_on", "reactor16.7.srv_on", "reactor16.7.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_7_alarm_clear", DataDef("reactor16.7.a_clr", "reactor16.7.a_clr", "reactor16.7.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.cell_7_speed", DataDef("reactor16.7.block_v0", "reactor16.7.block_v0", "reactor16.7.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.cell_7_sv", DataDef("reactor16.7.block_data0", "reactor16.7.block_data0", "reactor16.7.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_7_start", DataDef("reactor16.7.stb", "reactor16.7.stb", "reactor16.7.stb"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_7_stop", DataDef("reactor16.7.s_stop", "reactor16.7.s_stop", "reactor16.7.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_8_enable", DataDef("reactor16.8.srv_on", "reactor16.8.srv_on", "reactor16.8.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_8_disable", DataDef("reactor16.8.srv_on", "reactor16.8.srv_on", "reactor16.8.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_8_alarm_clear", DataDef("reactor16.8.a_clr", "reactor16.8.a_clr", "reactor16.8.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.cell_8_speed", DataDef("reactor16.8.block_v0", "reactor16.8.block_v0", "reactor16.8.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.cell_8_sv", DataDef("reactor16.8.block_data0", "reactor16.8.block_data0", "reactor16.8.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_8_start", DataDef("reactor16.8.stb", "reactor16.8.stb", "reactor16.8.stb"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_8_stop", DataDef("reactor16.8.s_stop", "reactor16.8.s_stop", "reactor16.8.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_9_enable", DataDef("reactor16.9.srv_on", "reactor16.9.srv_on", "reactor16.9.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_9_disable", DataDef("reactor16.9.srv_on", "reactor16.9.srv_on", "reactor16.9.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_9_alarm_clear", DataDef("reactor16.9.a_clr", "reactor16.9.a_clr", "reactor16.9.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.cell_9_speed", DataDef("reactor16.9.block_v0", "reactor16.9.block_v0", "reactor16.9.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.cell_9_sv", DataDef("reactor16.9.block_data0", "reactor16.9.block_data0", "reactor16.9.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_9_start", DataDef("reactor16.9.stb", "reactor16.9.stb", "reactor16.9.stb"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_9_stop", DataDef("reactor16.9.s_stop", "reactor16.9.s_stop", "reactor16.9.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_10_enable", DataDef("reactor16.10.srv_on", "reactor16.10.srv_on", "reactor16.10.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_10_disable", DataDef("reactor16.10.srv_on", "reactor16.10.srv_on", "reactor16.10.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_10_alarm_clear", DataDef("reactor16.10.a_clr", "reactor16.10.a_clr", "reactor16.10.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.cell_10_speed", DataDef("reactor16.10.block_v0", "reactor16.10.block_v0", "reactor16.10.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.cell_10_sv", DataDef("reactor16.10.block_data0", "reactor16.10.block_data0", "reactor16.10.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_10_start", DataDef("reactor16.10.stb", "reactor16.10.stb", "reactor16.10.stb"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_10_stop", DataDef("reactor16.10.s_stop", "reactor16.10.s_stop", "reactor16.10.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_11_enable", DataDef("reactor16.11.srv_on", "reactor16.11.srv_on", "reactor16.11.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_11_disable", DataDef("reactor16.11.srv_on", "reactor16.11.srv_on", "reactor16.11.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_11_alarm_clear", DataDef("reactor16.11.a_clr", "reactor16.11.a_clr", "reactor16.11.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.cell_11_speed", DataDef("reactor16.11.block_v0", "reactor16.11.block_v0", "reactor16.11.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.cell_11_sv", DataDef("reactor16.11.block_data0", "reactor16.11.block_data0", "reactor16.11.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_11_start", DataDef("reactor16.11.stb", "reactor16.11.stb", "reactor16.11.stb"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_11_stop", DataDef("reactor16.11.s_stop", "reactor16.11.s_stop", "reactor16.11.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_12_enable", DataDef("reactor16.12.srv_on", "reactor16.12.srv_on", "reactor16.12.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_12_disable", DataDef("reactor16.12.srv_on", "reactor16.12.srv_on", "reactor16.12.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_12_alarm_clear", DataDef("reactor16.12.a_clr", "reactor16.12.a_clr", "reactor16.12.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.cell_12_speed", DataDef("reactor16.12.block_v0", "reactor16.12.block_v0", "reactor16.12.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.cell_12_sv", DataDef("reactor16.12.block_data0", "reactor16.12.block_data0", "reactor16.12.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_12_start", DataDef("reactor16.12.stb", "reactor16.12.stb", "reactor16.12.stb"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_12_stop", DataDef("reactor16.12.s_stop", "reactor16.12.s_stop", "reactor16.12.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_13_enable", DataDef("reactor16.13.srv_on", "reactor16.13.srv_on", "reactor16.13.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_13_disable", DataDef("reactor16.13.srv_on", "reactor16.13.srv_on", "reactor16.13.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_13_alarm_clear", DataDef("reactor16.13.a_clr", "reactor16.13.a_clr", "reactor16.13.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.cell_13_speed", DataDef("reactor16.13.block_v0", "reactor16.13.block_v0", "reactor16.13.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.cell_13_sv", DataDef("reactor16.13.block_data0", "reactor16.13.block_data0", "reactor16.13.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_13_start", DataDef("reactor16.13.stb", "reactor16.13.stb", "reactor16.13.stb"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_13_stop", DataDef("reactor16.13.s_stop", "reactor16.13.s_stop", "reactor16.13.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_14_enable", DataDef("reactor16.14.srv_on", "reactor16.14.srv_on", "reactor16.14.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_14_disable", DataDef("reactor16.14.srv_on", "reactor16.14.srv_on", "reactor16.14.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_14_alarm_clear", DataDef("reactor16.14.a_clr", "reactor16.14.a_clr", "reactor16.14.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.cell_14_speed", DataDef("reactor16.14.block_v0", "reactor16.14.block_v0", "reactor16.14.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.cell_14_sv", DataDef("reactor16.14.block_data0", "reactor16.14.block_data0", "reactor16.14.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_14_start", DataDef("reactor16.14.stb", "reactor16.14.stb", "reactor16.14.stb"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_14_stop", DataDef("reactor16.14.s_stop", "reactor16.14.s_stop", "reactor16.14.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_15_enable", DataDef("reactor16.15.srv_on", "reactor16.15.srv_on", "reactor16.15.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_15_disable", DataDef("reactor16.15.srv_on", "reactor16.15.srv_on", "reactor16.15.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_15_alarm_clear", DataDef("reactor16.15.a_clr", "reactor16.15.a_clr", "reactor16.15.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.cell_15_speed", DataDef("reactor16.15.block_v0", "reactor16.15.block_v0", "reactor16.15.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.cell_15_sv", DataDef("reactor16.15.block_data0", "reactor16.15.block_data0", "reactor16.15.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_15_start", DataDef("reactor16.15.stb", "reactor16.15.stb", "reactor16.15.stb"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_15_stop", DataDef("reactor16.15.s_stop", "reactor16.15.s_stop", "reactor16.15.s_stop"));

    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_16_enable", DataDef("reactor16.16.srv_on", "reactor16.16.srv_on", "reactor16.16.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_16_disable", DataDef("reactor16.16.srv_on", "reactor16.16.srv_on", "reactor16.16.srv_on"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_16_alarm_clear", DataDef("reactor16.16.a_clr", "reactor16.16.a_clr", "reactor16.16.a_clr"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.cell_16_speed", DataDef("reactor16.16.block_v0", "reactor16.16.block_v0", "reactor16.16.block_v0"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.cell_16_sv", DataDef("reactor16.16.block_data0", "reactor16.16.block_data0", "reactor16.16.block_data0"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_16_start", DataDef("reactor16.16.stb", "reactor16.16.stb", "reactor16.16.stb"));
    data_model_.SetUiToDataMap("motorcontrol_reactor16.button_16_stop", DataDef("reactor16.16.s_stop", "reactor16.16.s_stop", "reactor16.16.s_stop"));
    // liquid distributor
    data_model_.SetUiToDataMap("distributor_sampling.control_code", DataDef("plc.1.dist_command", "plc.1.dist_command", "plc.1.dist_command"));
    data_model_.SetUiToDataMap("distributor_sampling.channel_a_run", DataDef("plc.1.dist_a_run", "plc.1.dist_a_run", "plc.1.dist_a_run"));
    data_model_.SetUiToDataMap("distributor_sampling.channel_b_run", DataDef("plc.1.dist_b_run", "plc.1.dist_b_run", "plc.1.dist_b_run"));
}

void MainWindow::RefreshUi(std::shared_ptr<std::vector<goiot::DataInfo>> data_info_vec)
{
    if (data_info_vec)
    {
        for (const auto& data_info : *data_info_vec)
        {
            bool ok = false;
            std::vector<UiInfo> ui_info_list = data_model_.GetUiInfo(data_info.id, ok);
            if (ok)
            {
                for (auto& ui_info : ui_info_list)
                {
                    Ui::ControlStatus status = data_info.result == 0 ? Ui::ControlStatus::OK : Ui::ControlStatus::FAILURE;
                    QString value;
                    double fvalue;
                    switch (data_info.data_type)
                    {
                    case goiot::DataType::STR:
                        value.fromStdString(data_info.char_value);
                        break;
                    case goiot::DataType::DF:
                        if (std::abs(data_info.ratio - 1.0) < 1e-6) // ratio conversion
                        {
                            fvalue = data_info.float_value;
                        }
                        else
                        {
                            fvalue = data_info.float_value * data_info.ratio;
                        }
                        value = QString::number(fvalue, 'f', ui_info.decimals);
                        break;
                    case goiot::DataType::BT:
                        value = QString::number(data_info.byte_value + ui_info.int_offset); // with UI offset
                        break;
                    case goiot::DataType::DB:
                    case goiot::DataType::DUB:
                    case goiot::DataType::WB:
                    case goiot::DataType::WUB:
                        if (std::abs(data_info.ratio - 1.0) < 1e-6) // ratio conversion with UI offset
                        {
                            value = QString::number(data_info.int_value + ui_info.int_offset);
                        }
                        else
                        {
                            fvalue = data_info.int_value * data_info.ratio + ui_info.int_offset;
                            value = QString::number(fvalue, 'f', ui_info.decimals);
                        }
                        break;
                    default:
                        throw std::invalid_argument("Unsupported data type");
                    }
                    QEvent* event = nullptr;
                    if (ui_info.type == WidgetType::TEXT)
                    {
                        event = new Ui::RefreshTextEvent(ui_info.ui_name, status, ui_info, data_info.id, value);
                    }
                    else if (ui_info.type == WidgetType::ONOFF || ui_info.type == WidgetType::STATE)
                    {
                        event = new Ui::RefreshStateEvent(ui_info.ui_name, status, ui_info, data_info.id, value.toInt());
                    }
                    else if (ui_info.type == WidgetType::PROCESS_VALUE)
                    {
                        event = new Ui::ProcessValueEvent(ui_info.ui_name, status, ui_info, data_info.id);
                    }
                    else if (ui_info.type == WidgetType::NONE)
                    {
                        continue;
                    }
                    else
                    {
                        assert(false);
                    }
                    QApplication::postEvent(ui_info.parent, event); // not using sendEvent() for UI update using the same thread problem.
                }
            }
        }
    }
}

void MainWindow::on_pushButton_clicked()
{
    QWidget* sender = static_cast<QWidget*>(this->sender());
    QApplication::sendEvent(ui_->tabWidget->widget(0),
                            new Ui::RefreshTextEvent("textEdit", Ui::ControlStatus::OK, UiInfo(), "mockid", "xxyy"));
    DialogSetValue set_value_dialog(sender, "34.5", MeasurementUnit::DEGREE);

    // convert the widget position to the screen position.
    QPoint screen_pos = this->mapToGlobal(sender->pos());
    screen_pos.setX(screen_pos.x() + 25);
    screen_pos.setY(screen_pos.y() + 10);
    set_value_dialog.move(screen_pos);
    int result = set_value_dialog.exec();
    if (result == QDialog::Accepted)
    {
        float f = set_value_dialog.NewValue();
        f++;
    }
    else
    {

    }

}

void MainWindow::on_pushButton_2_clicked()
{
    QWidget* sender = static_cast<QWidget*>(this->sender());
    // dialog_setposition
    DialogSetPosition set_position_dialog(sender, 8, 4);
    int result = set_position_dialog.exec();
    if (result == QDialog::Accepted)
    {
        int pos = set_position_dialog.NewValue();
        std::cout << "pos: " << pos << std::endl;
    }
}

void MainWindow::on_pushButton_3_clicked()
{
    QWidget* sender = static_cast<QWidget*>(this->sender());
    // dialog_setposition
    DialogOnOff on_off_dialog(sender, 1);
    int result = on_off_dialog.exec();
    if (result == QDialog::Accepted)
    {
        int pos = on_off_dialog.NewValue();
        std::cout << "pos: " << pos << std::endl;
    }
}

bool MainWindow::ReadData(const QString& parent_ui_name, const QString& ui_name, QString& value, Ui::ControlStatus& status, UiInfo& ui_info)
{
    value = QString();
    status = Ui::ControlStatus::FAILURE;

    bool ok;
    auto data_def = data_model_.GetDataDef(parent_ui_name + "." + ui_name, ok);
    if (!ok)
    {
        return false;
    }

    std::vector<std::string> data_id_vec;
    data_id_vec.emplace_back(data_def.sv_read_id);
    auto data_info_vec = data_manager_.ReadDataCache(data_id_vec);
    assert(data_info_vec.size() == 1);
    auto& data_info = data_info_vec.at(0);
    if (data_info.id.empty())
    {
        return false;
    }

    auto ui_info_list = data_model_.GetUiInfo(data_info.id, ok);
    if (ok)
    {
        ui_info = ui_info_list[0]; // Get the first one only.
        status = data_info.result == 0 ? Ui::ControlStatus::OK : Ui::ControlStatus::FAILURE;
        double fvalue;
        switch (data_info.data_type)
        {
        case goiot::DataType::STR:
            value.fromStdString(data_info.char_value);
            break;
        case goiot::DataType::DF:
            if (std::abs(data_info.ratio - 1.0) < 1e-6) // ratio conversion
            {
                fvalue = data_info.float_value;
            }
            else
            {
                fvalue = data_info.float_value * data_info.ratio;
            }
            value = QString::number(fvalue, 'f', ui_info.decimals);
            break;
        case goiot::DataType::BT:
            value = QString::number(data_info.byte_value + ui_info.int_offset); // with UI offset
            break;
        case goiot::DataType::DB:
        case goiot::DataType::DUB:
        case goiot::DataType::WB:
        case goiot::DataType::WUB:
            if (std::abs(data_info.ratio - 1.0) < 1e-6) // ratio conversion with UI offset
            {
                value = QString::number(data_info.int_value + ui_info.int_offset);
            }
            else
            {
                fvalue = data_info.int_value * data_info.ratio + ui_info.int_offset;
                value = QString::number(fvalue, 'f', ui_info.decimals);
            }
            break;
        default:
            throw std::invalid_argument("Unsupported data type");
        }
        return true;
    }
    else
    {
        return false;
    }
}

bool MainWindow::WriteData(const QString& parent_ui_name, const QString& ui_name, const QString& value)
{
    bool ok;
    auto data_def = data_model_.GetDataDef(parent_ui_name + "." + ui_name, ok);
    if (!ok)
    {
        assert(false);
        return false;
    }

    std::vector<std::string> data_id_vec;
    data_id_vec.emplace_back(data_def.sv_write_id);
    auto data_info_vec = data_manager_.ReadDataCache(data_id_vec);
    assert(data_info_vec.size() == 1);
    auto& data_info = data_info_vec.at(0);
    if (data_info.id.empty())
    {
        assert(false);
        return false;
    }

    auto ui_info_list = data_model_.GetUiInfo(data_info.id, ok);
    // We need UiInfo.int_offset only.
    UiInfo ui_info;
    if (ok)
    {
        ui_info = ui_info_list[0];
    }

    double float_value = 0.0;
    uint8_t byte_value = 0;
    int int_value = 0;
    switch (data_info.data_type)
    {
    case goiot::DataType::STR:
        data_info.char_value = value.toStdString();
        break;
    case goiot::DataType::DF:
        float_value = value.toDouble(&ok);
        if (!ok)
        {
            assert(false);
            throw std::invalid_argument("MainWindow::WriteData converts an invalid value.");
        }
        if (std::abs(data_info.ratio - 1.0) >= 1e-6) // ratio conversion
        {
            float_value /= data_info.ratio;
        }
        data_info.float_value = float_value;
        break;
    case goiot::DataType::BT:
        byte_value = value.toUShort(&ok);
        assert(ok);
        byte_value -= ui_info.int_offset; // with UI offset
        data_info.byte_value = byte_value;
        break;
    case goiot::DataType::DB:
    case goiot::DataType::DUB:
    case goiot::DataType::WB:
    case goiot::DataType::WUB:
        int_value = value.toInt(&ok);
        if (ok)
        {
            int_value -= ui_info.int_offset; // with UI offset
            if (std::abs(data_info.ratio - 1.0) >= 1e-6) // ratio conversion
            {
                int_value = static_cast<int>(int_value / data_info.ratio);
            }
            data_info.int_value = int_value;
        }
        else
        {
            float_value = value.toDouble(&ok);
            if (ok)
            {
                float_value -= ui_info.int_offset; // with UI offset
                if (std::abs(data_info.ratio - 1.0) >= 1e-6) // ratio conversion
                {
                    float_value /= data_info.ratio;
                }
                data_info.int_value = static_cast<int>(float_value);
            }
            else
            {
                assert(false);
                throw std::invalid_argument("MainWindow::WriteData converts an invalid value.");
            }
        }
        break;
    default:
        throw std::invalid_argument("Unsupported data type");
    }
    data_info.data_flow_type = goiot::DataFlowType::ASYNC_WRITE;
    data_info.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock().now().time_since_epoch()).count() / 1000.0;
    data_info.result = 0;
    return data_manager_.WriteDataAsync(data_info_vec) == 0 ? true : false;
}

void MainWindow::on_listView_clicked(const QModelIndex &index)
{
    std::vector<std::string> controls;
    controls.push_back("widget_control");
    controls.push_back("widget_motor");
    controls.push_back("widget_workflow");
    controls.push_back("widget_trend");
    controls.push_back("widget_history");
    controls.push_back("widget_distributor");

    QRegularExpression re("^widget_");

    QList<QWidget*> children = ui_->centralwidget->findChildren<QWidget*>(re, Qt::FindDirectChildrenOnly);
    std::string control_name = controls.at(index.row());
    for (auto& child : children)
    {
        if (child->objectName().toStdString() == control_name)
        {
            child->setEnabled(true);
            child->setVisible(true);
        }
        else
        {
            child->setEnabled(false);
            child->setVisible(false);
        }
    }
}
