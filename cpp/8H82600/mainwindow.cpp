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
#include "events.h"
#include "dialog_setvalue.h"
#include "dialog_setposition.h"
#include "dialog_onoff.h"
#include "resourcedef.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui_(new Ui::MainWindow), data_manager_(QApplication::applicationDirPath().toStdString() )
{
    ui_->setupUi(this);

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

    // Setup Ocx
    //ui_->tabWidget->hide();
    //bool ok = ui_->axWidget->setControl("{B6F7A42C-8939-46F0-9BC4-518C1B3036D2}"); // WorkflowComponent.WorkflowComponentCtrl.1
    //ui_->axWidget->show();


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
    data_model_.SetDataToUiMap("plc.1.mvalve7_pv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_HC1201"), RES_SVALVE_1, WidgetType::STATE, MeasurementUnit::NONE, 0, 4, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_1", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_HC1202"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_2", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_HC1203"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_3", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_HC1204"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve8_pv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_HC1205"), RES_SVALVE_7, WidgetType::STATE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve9_pv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_HC1206"), RES_SVALVE_7, WidgetType::STATE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve10_pv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_HC1207"), RES_SVALVE_7, WidgetType::STATE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve11_pv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_HC1208"), RES_SVALVE_7, WidgetType::STATE, MeasurementUnit::NONE, 0, 8, 1));

    // liquidswtich
    data_model_.SetDataToUiMap("plc.1.mvalve1_pv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_HC2301"), RES_SVALVE_1, WidgetType::STATE, MeasurementUnit::NONE, 0, 4, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_1", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_HC2302"), RES_SVALVE_5, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve2_pv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_HC2303"), RES_SVALVE_2, WidgetType::STATE, MeasurementUnit::NONE, 0, 4, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve3_pv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_HC2304"), RES_SVALVE_8, WidgetType::STATE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve4_pv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_HC2401"), RES_SVALVE_1, WidgetType::STATE, MeasurementUnit::NONE, 0, 4, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve5_pv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_HC2403"), RES_SVALVE_2, WidgetType::STATE, MeasurementUnit::NONE, 0, 4, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve6_pv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_HC2404"), RES_SVALVE_8, WidgetType::STATE, MeasurementUnit::NONE, 0, 8, 1));
    // liquidfeed
    data_model_.SetDataToUiMap("plc.1.smc9_2", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_HC2306"), RES_SVALVE_5, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_3", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_HC2307"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_4", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_HC2308"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_5", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_HC2311"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_6", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_HC2312"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_7", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_HC2313"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_8", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_HC2314"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_9", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_HC2315"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_10", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_HC2316"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_11", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_HC2317"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_12", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_HC2318"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_13", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_HC2319"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_14", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_HC2320"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_15", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_HC2321"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_16", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_HC2322"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_17", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_HC2323"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_18", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_HC2324"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_19", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_HC2325"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    //
    data_model_.SetDataToUiMap("plc.1.smc10_4", UiInfo(ui_->tabWidget->widget(3), QString::fromUtf8("label_HC2407"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_5", UiInfo(ui_->tabWidget->widget(3), QString::fromUtf8("label_HC2408"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_6", UiInfo(ui_->tabWidget->widget(3), QString::fromUtf8("label_HC2411"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_7", UiInfo(ui_->tabWidget->widget(3), QString::fromUtf8("label_HC2412"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_8", UiInfo(ui_->tabWidget->widget(3), QString::fromUtf8("label_HC2413"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_9", UiInfo(ui_->tabWidget->widget(3), QString::fromUtf8("label_HC2414"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_10", UiInfo(ui_->tabWidget->widget(3), QString::fromUtf8("label_HC2415"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_11", UiInfo(ui_->tabWidget->widget(3), QString::fromUtf8("label_HC2416"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_12", UiInfo(ui_->tabWidget->widget(3), QString::fromUtf8("label_HC2417"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_13", UiInfo(ui_->tabWidget->widget(3), QString::fromUtf8("label_HC2418"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_14", UiInfo(ui_->tabWidget->widget(3), QString::fromUtf8("label_HC2419"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_15", UiInfo(ui_->tabWidget->widget(3), QString::fromUtf8("label_HC2420"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_16", UiInfo(ui_->tabWidget->widget(3), QString::fromUtf8("label_HC2421"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_17", UiInfo(ui_->tabWidget->widget(3), QString::fromUtf8("label_HC2422"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_18", UiInfo(ui_->tabWidget->widget(3), QString::fromUtf8("label_HC2423"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_19", UiInfo(ui_->tabWidget->widget(3), QString::fromUtf8("label_HC2424"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_20", UiInfo(ui_->tabWidget->widget(3), QString::fromUtf8("label_HC2425"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    // EO/PO
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
    //
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
    // gassampling
    data_model_.SetDataToUiMap("plc.1.mvalve12_pv", UiInfo(ui_->tabWidget->widget(8), QString::fromUtf8("label_HC5101"), RES_SVALVE_8, WidgetType::STATE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_21", UiInfo(ui_->tabWidget->widget(8), QString::fromUtf8("label_HC5102"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_22", UiInfo(ui_->tabWidget->widget(8), QString::fromUtf8("label_HC5103"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc10_23", UiInfo(ui_->tabWidget->widget(8), QString::fromUtf8("label_HC5104"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve13_pv", UiInfo(ui_->tabWidget->widget(8), QString::fromUtf8("label_HC5201"), RES_SVALVE_8, WidgetType::STATE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_20", UiInfo(ui_->tabWidget->widget(8), QString::fromUtf8("label_HC5202"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_21", UiInfo(ui_->tabWidget->widget(8), QString::fromUtf8("label_HC5203"), RES_SVALVE_4, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc9_22", UiInfo(ui_->tabWidget->widget(8), QString::fromUtf8("label_HC5204"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    // liquidcollection
    data_model_.SetDataToUiMap("plc.1.smc11_19", UiInfo(ui_->tabWidget->widget(9), QString::fromUtf8("label_HC6101"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve14_pv", UiInfo(ui_->tabWidget->widget(9), QString::fromUtf8("label_HC6102"), RES_SVALVE_7, WidgetType::STATE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve15_pv", UiInfo(ui_->tabWidget->widget(9), QString::fromUtf8("label_HC6103"), RES_SVALVE_8, WidgetType::STATE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve16_pv", UiInfo(ui_->tabWidget->widget(9), QString::fromUtf8("label_HC6104"), RES_SVALVE_8, WidgetType::STATE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.smc11_20", UiInfo(ui_->tabWidget->widget(9), QString::fromUtf8("label_HC6105"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.smc12_19", UiInfo(ui_->tabWidget->widget(9), QString::fromUtf8("label_HC6201"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve17_pv", UiInfo(ui_->tabWidget->widget(9), QString::fromUtf8("label_HC6202"), RES_SVALVE_7, WidgetType::STATE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve18_pv", UiInfo(ui_->tabWidget->widget(9), QString::fromUtf8("label_HC6203"), RES_SVALVE_8, WidgetType::STATE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.mvalve19_pv", UiInfo(ui_->tabWidget->widget(9), QString::fromUtf8("label_HC6204"), RES_SVALVE_8, WidgetType::STATE, MeasurementUnit::NONE, 0, 8, 1));
    data_model_.SetDataToUiMap("plc.1.smc12_20", UiInfo(ui_->tabWidget->widget(9), QString::fromUtf8("label_HC6205"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
    // liquidsamping
    data_model_.SetDataToUiMap("plc.1.smc11_1", UiInfo(ui_->tabWidget->widget(10), QString::fromUtf8("label_HC7101"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
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
    //
    data_model_.SetDataToUiMap("plc.1.smc12_1", UiInfo(ui_->tabWidget->widget(11), QString::fromUtf8("label_HC7301"), RES_SVALVE_6, WidgetType::STATE, MeasurementUnit::NONE, 0, 2, 1, 1));
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
    data_model_.SetUiToDataMap("gasfeed.label_HC1201", DataDef("plc.1.mvalve7_pv", "plc.1.mvalve7_sv", "plc.1.mvalve7_sv"));
    data_model_.SetUiToDataMap("gasfeed.label_HC1202", DataDef("plc.1.smc10_1", "plc.1.smc10_1", "plc.1.smc10_1"));
    data_model_.SetUiToDataMap("gasfeed.label_HC1203", DataDef("plc.1.smc10_2", "plc.1.smc10_2", "plc.1.smc10_2"));
    data_model_.SetUiToDataMap("gasfeed.label_HC1204", DataDef("plc.1.smc10_3", "plc.1.smc10_3", "plc.1.smc10_3"));
    data_model_.SetUiToDataMap("gasfeed.label_HC1205", DataDef("plc.1.mvalve8_pv", "plc.1.mvalve8_sv", "plc.1.mvalve8_sv"));
    data_model_.SetUiToDataMap("gasfeed.label_HC1206", DataDef("plc.1.mvalve9_pv", "plc.1.mvalve9_sv", "plc.1.mvalve9_sv"));
    data_model_.SetUiToDataMap("gasfeed.label_HC1207", DataDef("plc.1.mvalve10_pv", "plc.1.mvalve10_sv", "plc.1.mvalve10_sv"));
    data_model_.SetUiToDataMap("gasfeed.label_HC1208", DataDef("plc.1.mvalve11_pv", "plc.1.mvalve11_sv", "plc.1.mvalve11_sv"));
    // liquidswitch
    data_model_.SetUiToDataMap("liquidswitch.label_HC2301", DataDef("plc.1.mvalve1_pv", "plc.1.mvalve1_sv", "plc.1.mvalve1_sv"));
    data_model_.SetUiToDataMap("liquidswitch.label_HC2302", DataDef("plc.1.smc9_1", "plc.1.smc9_1", "plc.1.smc9_1"));
    data_model_.SetUiToDataMap("liquidswitch.label_HC2303", DataDef("plc.1.mvalve2_pv", "plc.1.mvalve2_sv", "plc.2.mvalve2_sv"));
    data_model_.SetUiToDataMap("liquidswitch.label_HC2304", DataDef("plc.1.mvalve3_pv", "plc.1.mvalve3_sv", "plc.3.mvalve3_sv"));
    data_model_.SetUiToDataMap("liquidswitch.label_HC2401", DataDef("plc.1.mvalve4_pv", "plc.1.mvalve4_sv", "plc.1.mvalve4_sv"));
    data_model_.SetUiToDataMap("liquidswitch.label_HC2403", DataDef("plc.1.mvalve5_pv", "plc.1.mvalve5_sv", "plc.2.mvalve5_sv"));
    data_model_.SetUiToDataMap("liquidswitch.label_HC2404", DataDef("plc.1.mvalve6_pv", "plc.1.mvalve6_sv", "plc.3.mvalve6_sv"));
    // liquidfeed
    data_model_.SetUiToDataMap("liquidfeeda.label_HC2306", DataDef("plc.1.smc9_2", "plc.1.smc9_2", "plc.1.smc9_2"));
    data_model_.SetUiToDataMap("liquidfeeda.label_HC2307", DataDef("plc.1.smc9_3", "plc.1.smc9_3", "plc.1.smc9_3"));
    data_model_.SetUiToDataMap("liquidfeeda.label_HC2308", DataDef("plc.1.smc9_4", "plc.1.smc9_4", "plc.1.smc9_4"));
    data_model_.SetUiToDataMap("liquidfeeda.label_HC2311", DataDef("plc.1.smc9_5", "plc.1.smc9_5", "plc.1.smc9_5"));
    data_model_.SetUiToDataMap("liquidfeeda.label_HC2312", DataDef("plc.1.smc9_6", "plc.1.smc9_6", "plc.1.smc9_6"));
    data_model_.SetUiToDataMap("liquidfeeda.label_HC2313", DataDef("plc.1.smc9_7", "plc.1.smc9_7", "plc.1.smc9_7"));
    data_model_.SetUiToDataMap("liquidfeeda.label_HC2314", DataDef("plc.1.smc9_8", "plc.1.smc9_8", "plc.1.smc9_8"));
    data_model_.SetUiToDataMap("liquidfeeda.label_HC2315", DataDef("plc.1.smc9_9", "plc.1.smc9_9", "plc.1.smc9_9"));
    data_model_.SetUiToDataMap("liquidfeeda.label_HC2316", DataDef("plc.1.smc9_10", "plc.1.smc9_10", "plc.1.smc9_10"));
    data_model_.SetUiToDataMap("liquidfeeda.label_HC2317", DataDef("plc.1.smc9_11", "plc.1.smc9_11", "plc.1.smc9_11"));
    data_model_.SetUiToDataMap("liquidfeeda.label_HC2318", DataDef("plc.1.smc9_12", "plc.1.smc9_12", "plc.1.smc9_12"));
    data_model_.SetUiToDataMap("liquidfeeda.label_HC2319", DataDef("plc.1.smc9_13", "plc.1.smc9_13", "plc.1.smc9_13"));
    data_model_.SetUiToDataMap("liquidfeeda.label_HC2320", DataDef("plc.1.smc9_14", "plc.1.smc9_14", "plc.1.smc9_14"));
    data_model_.SetUiToDataMap("liquidfeeda.label_HC2321", DataDef("plc.1.smc9_15", "plc.1.smc9_15", "plc.1.smc9_15"));
    data_model_.SetUiToDataMap("liquidfeeda.label_HC2322", DataDef("plc.1.smc9_16", "plc.1.smc9_16", "plc.1.smc9_16"));
    data_model_.SetUiToDataMap("liquidfeeda.label_HC2323", DataDef("plc.1.smc9_17", "plc.1.smc9_17", "plc.1.smc9_17"));
    data_model_.SetUiToDataMap("liquidfeeda.label_HC2324", DataDef("plc.1.smc9_18", "plc.1.smc9_18", "plc.1.smc9_18"));
    data_model_.SetUiToDataMap("liquidfeeda.label_HC2325", DataDef("plc.1.smc9_19", "plc.1.smc9_19", "plc.1.smc9_19"));
    //
    data_model_.SetUiToDataMap("liquidfeedb.label_HC2407", DataDef("plc.1.smc10_4", "plc.1.smc10_4", "plc.1.smc10_4"));
    data_model_.SetUiToDataMap("liquidfeedb.label_HC2408", DataDef("plc.1.smc10_5", "plc.1.smc10_5", "plc.1.smc10_5"));
    data_model_.SetUiToDataMap("liquidfeedb.label_HC2411", DataDef("plc.1.smc10_6", "plc.1.smc10_6", "plc.1.smc10_6"));
    data_model_.SetUiToDataMap("liquidfeedb.label_HC2412", DataDef("plc.1.smc10_7", "plc.1.smc10_7", "plc.1.smc10_7"));
    data_model_.SetUiToDataMap("liquidfeedb.label_HC2413", DataDef("plc.1.smc10_8", "plc.1.smc10_8", "plc.1.smc10_8"));
    data_model_.SetUiToDataMap("liquidfeedb.label_HC2414", DataDef("plc.1.smc10_9", "plc.1.smc10_9", "plc.1.smc10_9"));
    data_model_.SetUiToDataMap("liquidfeedb.label_HC2415", DataDef("plc.1.smc10_10", "plc.1.smc10_10", "plc.1.smc10_10"));
    data_model_.SetUiToDataMap("liquidfeedb.label_HC2416", DataDef("plc.1.smc10_11", "plc.1.smc10_11", "plc.1.smc10_11"));
    data_model_.SetUiToDataMap("liquidfeedb.label_HC2417", DataDef("plc.1.smc10_12", "plc.1.smc10_12", "plc.1.smc10_12"));
    data_model_.SetUiToDataMap("liquidfeedb.label_HC2418", DataDef("plc.1.smc10_13", "plc.1.smc10_13", "plc.1.smc10_13"));
    data_model_.SetUiToDataMap("liquidfeedb.label_HC2419", DataDef("plc.1.smc10_14", "plc.1.smc10_14", "plc.1.smc10_14"));
    data_model_.SetUiToDataMap("liquidfeedb.label_HC2420", DataDef("plc.1.smc10_15", "plc.1.smc10_15", "plc.1.smc10_15"));
    data_model_.SetUiToDataMap("liquidfeedb.label_HC2421", DataDef("plc.1.smc10_16", "plc.1.smc10_16", "plc.1.smc10_16"));
    data_model_.SetUiToDataMap("liquidfeedb.label_HC2422", DataDef("plc.1.smc10_17", "plc.1.smc10_17", "plc.1.smc10_17"));
    data_model_.SetUiToDataMap("liquidfeedb.label_HC2423", DataDef("plc.1.smc10_18", "plc.1.smc10_18", "plc.1.smc10_18"));
    data_model_.SetUiToDataMap("liquidfeedb.label_HC2424", DataDef("plc.1.smc10_19", "plc.1.smc10_19", "plc.1.smc10_19"));
    data_model_.SetUiToDataMap("liquidfeedb.label_HC2425", DataDef("plc.1.smc10_20", "plc.1.smc10_20", "plc.1.smc10_20"));
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
    // gassampling
    data_model_.SetUiToDataMap("gassampling.label_HC5101", DataDef("plc.1.mvalve12_pv", "plc.1.mvalve12_sv", "plc.1.mvalve12_sv"));
    data_model_.SetUiToDataMap("gassampling.label_HC5102", DataDef("plc.1.smc10_21", "plc.10.smc10_21", "plc.1.smc10_21"));
    data_model_.SetUiToDataMap("gassampling.label_HC5103", DataDef("plc.1.smc10_22", "plc.10.smc10_22", "plc.1.smc10_22"));
    data_model_.SetUiToDataMap("gassampling.label_HC5104", DataDef("plc.1.smc10_23", "plc.10.smc10_23", "plc.1.smc10_23"));
    data_model_.SetUiToDataMap("gassampling.label_HC5201", DataDef("plc.1.mvalve13_pv", "plc.1.mvalve13_sv", "plc.1.mvalve13_sv"));
    data_model_.SetUiToDataMap("gassampling.label_HC5202", DataDef("plc.1.smc9_20", "plc.9.smc9_20", "plc.1.smc9_20"));
    data_model_.SetUiToDataMap("gassampling.label_HC5203", DataDef("plc.1.smc9_21", "plc.9.smc9_21", "plc.1.smc9_21"));
    data_model_.SetUiToDataMap("gassampling.label_HC5204", DataDef("plc.1.smc9_22", "plc.9.smc9_22", "plc.1.smc9_22"));
    // liquidcollection
    data_model_.SetUiToDataMap("liquidcollection.label_HC6101", DataDef("plc.1.smc11_19", "plc.1.smc11_19", "plc.1.smc11_19"));
    data_model_.SetUiToDataMap("liquidcollection.label_HC6102", DataDef("plc.1.mvalve14_pv", "plc.1.mvalve14_sv", "plc.1.mvalve14_sv"));
    data_model_.SetUiToDataMap("liquidcollection.label_HC6103", DataDef("plc.1.mvalve15_pv", "plc.1.mvalve15_sv", "plc.1.mvalve15_sv"));
    data_model_.SetUiToDataMap("liquidcollection.label_HC6104", DataDef("plc.1.mvalve16_pv", "plc.1.mvalve16_sv", "plc.1.mvalve16_sv"));
    data_model_.SetUiToDataMap("liquidcollection.label_HC6105", DataDef("plc.1.smc11_20", "plc.1.smc11_20", "plc.1.smc11_20"));
    data_model_.SetUiToDataMap("liquidcollection.label_HC6201", DataDef("plc.1.smc12_19", "plc.1.smc12_19", "plc.1.smc12_19"));
    data_model_.SetUiToDataMap("liquidcollection.label_HC6202", DataDef("plc.1.mvalve17_pv", "plc.1.mvalve17_sv", "plc.1.mvalve17_sv"));
    data_model_.SetUiToDataMap("liquidcollection.label_HC6203", DataDef("plc.1.mvalve18_pv", "plc.1.mvalve18_sv", "plc.1.mvalve18_sv"));
    data_model_.SetUiToDataMap("liquidcollection.label_HC6204", DataDef("plc.1.mvalve19_pv", "plc.1.mvalve19_sv", "plc.1.mvalve19_sv"));
    data_model_.SetUiToDataMap("liquidcollection.label_HC6205", DataDef("plc.1.smc12_20", "plc.1.smc12_20", "plc.1.smc12_20"));
    // liquidsamplinga
    data_model_.SetUiToDataMap("liquidsamplinga.label_HC7101", DataDef("plc.1.smc11_1", "plc.1.smc11_1", "plc.1.smc11_1"));
    data_model_.SetUiToDataMap("liquidsamplinga.label_HC7102", DataDef("plc.1.mvalve20_pv", "plc.1.mvalve20_sv", "plc.1.mvalve20_sv"));
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
    // liquidsamplingb
    data_model_.SetUiToDataMap("liquidsamplingb.label_HC7301", DataDef("plc.1.smc12_1", "plc.1.smc12_1", "plc.1.smc12_1"));
    data_model_.SetUiToDataMap("liquidsamplingb.label_HC7302", DataDef("plc.1.mvalve21_pv", "plc.1.mvalve21_sv", "plc.1.mvalve21_sv"));
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
}

void MainWindow::RefreshUi(std::shared_ptr<std::vector<goiot::DataInfo>> data_info_vec)
{
    if (data_info_vec)
    {
        for (const auto& data_info : *data_info_vec)
        {
            bool ok = false;
            UiInfo ui_info = data_model_.GetUiInfo(data_info.id, ok);
            if (ok)
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
                    event = new Ui::RefreshTextEvent(ui_info.ui_name, status, ui_info, value);
                }
                else if (ui_info.type == WidgetType::ONOFF || ui_info.type == WidgetType::STATE)
                {
                    event = new Ui::RefreshStateEvent(ui_info.ui_name, status, ui_info, value.toInt());
                }
                else if (ui_info.type == WidgetType::PROCESS_VALUE)
                {
                    continue; // No UI refresh
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

void MainWindow::on_pushButton_clicked()
{
    QWidget* sender = static_cast<QWidget*>(this->sender());
    QApplication::sendEvent(ui_->tabWidget->widget(0),
                new Ui::RefreshTextEvent("textEdit", Ui::ControlStatus::OK, UiInfo(), "xxyy"));
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
    data_id_vec.emplace_back(data_def.pv_id);
    auto data_info_vec = data_manager_.ReadDataCache(data_id_vec);
    assert(data_info_vec.size() == 1);
    auto& data_info = data_info_vec.at(0);
    if (data_info.id.empty())
    {
        return false;
    }

    ui_info = data_model_.GetUiInfo(data_info.id, ok);
    if (ok)
    {
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

    UiInfo ui_info = data_model_.GetUiInfo(data_info.id, ok);
    if (!ok)
    {
        assert(false);
        return false;
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

