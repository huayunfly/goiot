#include <iostream>
#include <functional>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "form_gasfeed.h"
#include "form_reactor.h"
#include "form_analysis.h"
#include "form_trend.h"
#include "form_history.h"
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

    // Control tab pages
    std::vector<FormCommon*> form_vec;
    form_vec.push_back(new FormGasFeed);
    form_vec.push_back(new FormReactor);
    form_vec.push_back(new FormAnalysis);

    for (auto& entry : form_vec)
    {
        entry->setBaseSize(QSize(1180, 900));
        entry->RegisterReadDataFunc(std::bind(&MainWindow::ReadData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
        entry->RegisterWriteDataFunc(std::bind(&MainWindow::WriteData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        ui_->tabWidget->addTab(entry, entry->GetDisplayName());
    }

    this->setWindowState(Qt::WindowMaximized);

    // Setup Ocx
    bool ok = ui_->widget_workflow->setControl("{!B6F7A42C-8939-46F0-9BC4-518C1B3036D2}"); // WorkflowComponent.WorkflowComponentCtrl.1 "{B6F7A42C-8939-46F0-9BC4-518C1B3036D2}"
    if (ok)
    {
        ui_->widget_workflow->show();
    }

    // Trend
    FormTrend* form_trend = new FormTrend(ui_->widget_trend);
    form_trend->setObjectName(QString::fromUtf8("widget_trend"));
    form_trend->setGeometry(QRect(61, 0, 1200, 1100));
    form_trend->setAutoFillBackground(false);
    form_trend->setStyleSheet(QString::fromUtf8("background:#FFFFFF"));

    // History
    FormHistory* form_history = new FormHistory(ui_->widget_history);
    form_history->setObjectName(QString::fromUtf8("widget_history"));
    form_history->setGeometry(QRect(61, 0, 1200, 1100));
    form_history->setAutoFillBackground(false);
    form_history->setStyleSheet(QString::fromUtf8("background:#FFFFFF"));

    // Setup listview pages
    QStandardItemModel* model = new QStandardItemModel(this);
    QList<QStandardItem*> list;
    QStandardItem* s1 = new QStandardItem(QIcon(ICON_CONTROL), QString("控制"));
    QStandardItem* s2 = new QStandardItem(QIcon(ICON_WORKFLOW), QString("流程"));
    QStandardItem* s3 = new QStandardItem(QIcon(ICON_TREND), QString("趋势"));
    QStandardItem* s4 = new QStandardItem(QIcon(ICON_HISTORY), QString("历史"));
    model->appendRow(s1);
    model->appendRow(s2);
    model->appendRow(s3);
    model->appendRow(s4);
    ui_->listView->setModel(model);
    QModelIndex index_want = model->index(0, 0);
    on_listView_clicked(index_want);
    //ui_->listView->setCurrentIndex(index_want);

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
    //data_model_.SetDataToUiMap("plc.1.smc14_1", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_HC1110"), RES_VALVE_GAS, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));

    //
    data_model_.SetDataToUiMap("mfc.1.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_FICA1111"), RES_MFC, WidgetType::PROCESS_VALUE, MeasurementUnit::SCCM, 0, 250, 0));
    data_model_.SetDataToUiMap("mfc.2.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_FICA1121"), RES_MFC, WidgetType::PROCESS_VALUE, MeasurementUnit::SCCM, 0, 250, 0));
    data_model_.SetDataToUiMap("mfc.3.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_FICA1131"), RES_MFC, WidgetType::PROCESS_VALUE, MeasurementUnit::SCCM, 0, 250, 0));
    data_model_.SetDataToUiMap("mfc.4.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_FICA1141"), RES_MFC, WidgetType::PROCESS_VALUE, MeasurementUnit::SCCM, 0, 250, 0));
    data_model_.SetDataToUiMap("mfc.5.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_FICA1151"), RES_MFC, WidgetType::PROCESS_VALUE, MeasurementUnit::SCCM, 0, 250, 0));
    data_model_.SetDataToUiMap("mfc.6.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_FICA1511"), RES_MFC, WidgetType::PROCESS_VALUE, MeasurementUnit::SCCM, 0, 250, 0));
    data_model_.SetDataToUiMap("mfc.7.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_FICA1521"), RES_MFC, WidgetType::PROCESS_VALUE, MeasurementUnit::SCCM, 0, 250, 0));
    data_model_.SetDataToUiMap("mfc.8.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_FICA1531"), RES_MFC, WidgetType::PROCESS_VALUE, MeasurementUnit::SCCM, 0, 250, 0));
    data_model_.SetDataToUiMap("mfc.9.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_FICA1541"), RES_MFC, WidgetType::PROCESS_VALUE, MeasurementUnit::SCCM, 0, 250, 0));
    data_model_.SetDataToUiMap("mfc.10.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_FICA1551"), RES_MFC, WidgetType::PROCESS_VALUE, MeasurementUnit::SCCM, 0, 250, 0));

    data_model_.SetDataToUiMap("mfc.1.pv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_FICA1111"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::SCCM, 1, 250, 0));
    data_model_.SetDataToUiMap("mfc.2.pv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_FICA1121"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::SCCM, 1, 250, 0));
    data_model_.SetDataToUiMap("mfc.3.pv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_FICA1131"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::SCCM, 1, 250, 0));
    data_model_.SetDataToUiMap("mfc.4.pv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_FICA1141"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::SCCM, 1, 250, 0));
    data_model_.SetDataToUiMap("mfc.5.pv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_FICA1151"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::SCCM, 1, 250, 0));
    data_model_.SetDataToUiMap("mfc.6.pv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_FICA1511"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::SCCM, 1, 250, 0));
    data_model_.SetDataToUiMap("mfc.7.pv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_FICA1521"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::SCCM, 1, 250, 0));
    data_model_.SetDataToUiMap("mfc.8.pv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_FICA1531"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::SCCM, 1, 250, 0));
    data_model_.SetDataToUiMap("mfc.9.pv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_FICA1541"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::SCCM, 1, 250, 0));
    data_model_.SetDataToUiMap("mfc.10.pv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_FICA1551"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::SCCM, 1, 250, 0));

    data_model_.SetDataToUiMap("plc.1.pg_1", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_PIA1111"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::BARG, 1, 80, 0));
    data_model_.SetDataToUiMap("plc.1.pg_2", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_PIA1121"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::BARG, 1, 80, 0));
    data_model_.SetDataToUiMap("plc.1.pg_3", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_PIA1131"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::BARG, 1, 80, 0));
    data_model_.SetDataToUiMap("plc.1.pg_4", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_PIA1141"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::BARG, 1, 80, 0));
    data_model_.SetDataToUiMap("plc.1.pg_5", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_PIA1151"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::BARG, 1, 80, 0));
    data_model_.SetDataToUiMap("plc.1.pg_6", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_PIA1113"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::BARG, 1, 80, 0));
    data_model_.SetDataToUiMap("plc.1.pg_7", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_PIA1513"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::BARG, 1, 80, 0));

    data_model_.SetDataToUiMap("plc.1.gas_1", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_XISA1001"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::LEL, 1, 20, 0));
    data_model_.SetDataToUiMap("plc.1.gas_2", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_XISA1002"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::PPM, 1, 50, 0));

    // reactor - electric valve
    //data_model_.SetDataToUiMap("plc.1.dq2_7", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_HC1400"), RES_MECHANICAL_PUMP, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    //data_model_.SetDataToUiMap("plc.1.dq2_8", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_HC1402"), RES_VALVE_ELECTRIC, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));

    // reactor TC
    data_model_.SetDataToUiMap("plc.1.temp1_sv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_TICA2102"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 500, 0));
    data_model_.SetDataToUiMap("plc.1.temp2_sv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_TICA2103"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 500, 0));
    data_model_.SetDataToUiMap("plc.1.temp3_sv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_TICA2104"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 500, 0));
    data_model_.SetDataToUiMap("plc.1.temp4_sv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_TICA2105"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 500, 0));
    data_model_.SetDataToUiMap("plc.1.temp5_sv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_TICA2106"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 140, 0));
    data_model_.SetDataToUiMap("plc.1.temp6_sv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_TICA2201"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 140, 0));
    data_model_.SetDataToUiMap("plc.1.temp7_sv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_TICA2202"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 140, 0));
    data_model_.SetDataToUiMap("plc.1.temp8_sv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_TICA2203"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 140, 0));

    data_model_.SetDataToUiMap("plc.1.temp9_sv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_TICA2501"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 140, 0));
    //data_model_.SetDataToUiMap("plc.1.temp10_sv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_TICA2502"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 140, 0));
    data_model_.SetDataToUiMap("plc.1.temp11_sv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_TICA2503"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 140, 0));
    data_model_.SetDataToUiMap("plc.1.temp12_sv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_TICA2504"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 140, 0));
    data_model_.SetDataToUiMap("plc.1.temp13_sv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_TICA2601"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 140, 0));
    data_model_.SetDataToUiMap("plc.1.temp14_sv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_TICA2602"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 140, 0));
    data_model_.SetDataToUiMap("plc.1.temp15_sv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_TICA2603"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 140, 0));
    data_model_.SetDataToUiMap("plc.1.temp16_sv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_TICA2604"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 140, 0));

    data_model_.SetDataToUiMap("plc.1.temp17_sv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_TICA2101"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 140, 0));
    data_model_.SetDataToUiMap("plc.1.temp18_sv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_TICA2401"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 140, 0));
    data_model_.SetDataToUiMap("plc.1.temp19_sv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_TICA2801"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 140, 0));
    data_model_.SetDataToUiMap("plc.1.temp20_sv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_TICA2404"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 140, 0));
    data_model_.SetDataToUiMap("plc.1.temp21_sv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_TICA2804"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 140, 0));

    data_model_.SetDataToUiMap("plc.1.temp1_pv", {UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("textEdit_TICA2102"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0),
                               UiInfo((QWidget*)ui_->widget_trend->children().first(), QString::fromUtf8("chart_reactor"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0)});
    data_model_.SetDataToUiMap("plc.1.temp2_pv", {UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("textEdit_TICA2103"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0),
                               UiInfo((QWidget*)ui_->widget_trend->children().first(), QString::fromUtf8("chart_reactor"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0)});
    data_model_.SetDataToUiMap("plc.1.temp3_pv", {UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("textEdit_TICA2104"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0),
                               UiInfo((QWidget*)ui_->widget_trend->children().first(), QString::fromUtf8("chart_reactor"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0)});
    data_model_.SetDataToUiMap("plc.1.temp4_pv", {UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("textEdit_TICA2105"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0),
                               UiInfo((QWidget*)ui_->widget_trend->children().first(), QString::fromUtf8("chart_reactor"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0)});
    data_model_.SetDataToUiMap("plc.1.temp5_pv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("textEdit_TICA2106"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 140, 0));
    data_model_.SetDataToUiMap("plc.1.temp6_pv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("textEdit_TICA2201"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 140, 0));
    data_model_.SetDataToUiMap("plc.1.temp7_pv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("textEdit_TICA2202"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 140, 0));
    data_model_.SetDataToUiMap("plc.1.temp8_pv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("textEdit_TICA2203"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 140, 0));

    data_model_.SetDataToUiMap("plc.1.temp9_pv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("textEdit_TICA2501"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 140, 0));
    //data_model_.SetDataToUiMap("plc.1.temp10_pv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("textEdit_TICA2502"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 140, 0));
    data_model_.SetDataToUiMap("plc.1.temp11_pv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("textEdit_TICA2503"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 140, 0));
    data_model_.SetDataToUiMap("plc.1.temp12_pv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("textEdit_TICA2504"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 140, 0));
    data_model_.SetDataToUiMap("plc.1.temp13_pv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("textEdit_TICA2601"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 140, 0));
    data_model_.SetDataToUiMap("plc.1.temp14_pv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("textEdit_TICA2602"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 140, 0));
    data_model_.SetDataToUiMap("plc.1.temp15_pv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("textEdit_TICA2603"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 140, 0));
    data_model_.SetDataToUiMap("plc.1.temp16_pv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("textEdit_TICA2604"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 140, 0));

    data_model_.SetDataToUiMap("plc.1.temp17_pv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("textEdit_TICA2101"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 140, 0));
    data_model_.SetDataToUiMap("plc.1.temp18_pv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("textEdit_TICA2401"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 140, 0));
    data_model_.SetDataToUiMap("plc.1.temp19_pv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("textEdit_TICA2801"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 140, 0));
    data_model_.SetDataToUiMap("plc.1.temp20_pv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("textEdit_TICA2404"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 140, 0));
    data_model_.SetDataToUiMap("plc.1.temp21_pv", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("textEdit_TICA2804"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 140, 0));

    // Reactor PG
    data_model_.SetDataToUiMap("plc.1.pg_8", {UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("textEdit_PIA2402"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::BARG, 1, 80, 0),
                                              UiInfo((QWidget*)ui_->widget_trend->children().first(), QString::fromUtf8("chart_pg"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::BARG, 1, 80, 0)});
    data_model_.SetDataToUiMap("plc.1.pg_9", {UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("textEdit_PIA2802"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::BARG, 1, 80, 0),
                                              UiInfo((QWidget*)ui_->widget_trend->children().first(), QString::fromUtf8("chart_pg"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::BARG, 1, 80, 0)});

    // Reactor valves
    data_model_.SetDataToUiMap("plc.1.dq2_7", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_HC2404"), RES_VALVE_GAS_SMALL, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.dq2_8", UiInfo(ui_->tabWidget->widget(1), QString::fromUtf8("label_HC2804"), RES_VALVE_GAS_SMALL, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));

    // Analysis
    data_model_.SetDataToUiMap("plc.1.temp22_sv", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_TICA3110"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 140, 0));
    data_model_.SetDataToUiMap("plc.1.temp23_sv", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_TICA3120"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 140, 0));
    data_model_.SetDataToUiMap("plc.1.temp24_sv", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_TICA3130"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 140, 0));
    data_model_.SetDataToUiMap("plc.1.temp26_sv", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_TICA3140"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 140, 0));

    data_model_.SetDataToUiMap("plc.1.temp22_pv", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("textEdit_TICA3110"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 140, 0));
    data_model_.SetDataToUiMap("plc.1.temp23_pv", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("textEdit_TICA3120"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 140, 0));
    data_model_.SetDataToUiMap("plc.1.temp24_pv", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("textEdit_TICA3130"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 140, 0));
    data_model_.SetDataToUiMap("plc.1.temp26_pv", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("textEdit_TICA3140"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 140, 0));
    // Anslysis GC
    data_model_.SetDataToUiMap("plc.1.di1_5", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_GC1"), RES_GC, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    data_model_.SetDataToUiMap("plc.1.di1_6", UiInfo(ui_->tabWidget->widget(2), QString::fromUtf8("label_GC2"), RES_GC, WidgetType::ONOFF, MeasurementUnit::NONE, 0, 1, 0));
    // ui to data
    // gasfeed
    data_model_.SetUiToDataMap("gasfeed.label_FICA1111", DataDef("mfc.1.pv", "mfc.1.sv", "mfc.1.sv"));
    data_model_.SetUiToDataMap("gasfeed.label_FICA1121", DataDef("mfc.2.pv", "mfc.2.sv", "mfc.2.sv"));
    data_model_.SetUiToDataMap("gasfeed.label_FICA1131", DataDef("mfc.3.pv", "mfc.3.sv", "mfc.3.sv"));
    data_model_.SetUiToDataMap("gasfeed.label_FICA1141", DataDef("mfc.4.pv", "mfc.4.sv", "mfc.4.sv"));
    data_model_.SetUiToDataMap("gasfeed.label_FICA1151", DataDef("mfc.5.pv", "mfc.5.sv", "mfc.5.sv"));
    data_model_.SetUiToDataMap("gasfeed.label_FICA1511", DataDef("mfc.6.pv", "mfc.6.sv", "mfc.6.sv"));
    data_model_.SetUiToDataMap("gasfeed.label_FICA1521", DataDef("mfc.7.pv", "mfc.7.sv", "mfc.7.sv"));
    data_model_.SetUiToDataMap("gasfeed.label_FICA1531", DataDef("mfc.8.pv", "mfc.8.sv", "mfc.8.sv"));
    data_model_.SetUiToDataMap("gasfeed.label_FICA1541", DataDef("mfc.9.pv", "mfc.9.sv", "mfc.9.sv"));
    data_model_.SetUiToDataMap("gasfeed.label_FICA1551", DataDef("mfc.10.pv", "mfc.10.sv", "mfc.10.sv"));

    // reactor
    data_model_.SetUiToDataMap("reactor.label_TICA2102", DataDef("plc.1.temp1_pv", "plc.1.temp1_sv", "plc.1.temp1_sv"));
    data_model_.SetUiToDataMap("reactor.label_TICA2103", DataDef("plc.1.temp2_pv", "plc.1.temp2_sv", "plc.1.temp2_sv"));
    data_model_.SetUiToDataMap("reactor.label_TICA2104", DataDef("plc.1.temp3_pv", "plc.1.temp3_sv", "plc.1.temp3_sv"));
    data_model_.SetUiToDataMap("reactor.label_TICA2105", DataDef("plc.1.temp4_pv", "plc.1.temp4_sv", "plc.1.temp4_sv"));
    data_model_.SetUiToDataMap("reactor.label_TICA2106", DataDef("plc.1.temp5_pv", "plc.1.temp5_sv", "plc.1.temp5_sv"));
    data_model_.SetUiToDataMap("reactor.label_TICA2201", DataDef("plc.1.temp6_pv", "plc.1.temp6_sv", "plc.1.temp6_sv"));
    data_model_.SetUiToDataMap("reactor.label_TICA2202", DataDef("plc.1.temp7_pv", "plc.1.temp7_sv", "plc.1.temp7_sv"));
    data_model_.SetUiToDataMap("reactor.label_TICA2203", DataDef("plc.1.temp8_pv", "plc.1.temp8_sv", "plc.1.temp8_sv"));

    //data_model_.SetUiToDataMap("reactor.label_TICA2501", DataDef("plc.1.temp9_pv", "plc.1.temp9_sv", "plc.1.temp9_sv"));  // 釜管路预热
    //data_model_.SetUiToDataMap("reactor.label_TICA2502", DataDef("plc.1.temp10_pv", "plc.1.temp10_sv", "plc.1.temp10_sv")); // 釜加热预留
    data_model_.SetUiToDataMap("reactor.label_TICA2503", DataDef("plc.1.temp11_pv", "plc.1.temp11_sv", "plc.1.temp11_sv"));
    data_model_.SetUiToDataMap("reactor.label_TICA2504", DataDef("plc.1.temp12_pv", "plc.1.temp12_sv", "plc.1.temp12_sv"));
    data_model_.SetUiToDataMap("reactor.label_TICA2601", DataDef("plc.1.temp13_pv", "plc.1.temp13_sv", "plc.1.temp13_sv"));
    data_model_.SetUiToDataMap("reactor.label_TICA2602", DataDef("plc.1.temp14_pv", "plc.1.temp14_sv", "plc.1.temp14_sv"));
    data_model_.SetUiToDataMap("reactor.label_TICA2603", DataDef("plc.1.temp15_pv", "plc.1.temp15_sv", "plc.1.temp15_sv"));
    data_model_.SetUiToDataMap("reactor.label_TICA2604", DataDef("plc.1.temp16_pv", "plc.1.temp16_sv", "plc.1.temp16_sv"));

    //data_model_.SetUiToDataMap("reactor.label_TICA2101", DataDef("plc.1.temp17_pv", "plc.1.temp17_sv", "plc.1.temp17_sv")); // 固定床入口管预热
    data_model_.SetUiToDataMap("reactor.label_TICA2401", DataDef("plc.1.temp18_pv", "plc.1.temp18_sv", "plc.1.temp18_sv"));
    data_model_.SetUiToDataMap("reactor.label_TICA2801", DataDef("plc.1.temp19_pv", "plc.1.temp19_sv", "plc.1.temp19_sv"));
    data_model_.SetUiToDataMap("reactor.label_TICA2404", DataDef("plc.1.temp20_pv", "plc.1.temp20_sv", "plc.1.temp20_sv"));
    data_model_.SetUiToDataMap("reactor.label_TICA2804", DataDef("plc.1.temp21_pv", "plc.1.temp21_sv", "plc.1.temp21_sv"));
    // valve
    data_model_.SetUiToDataMap("reactor.label_HC2404", DataDef("plc.1.dq2_7", "plc.1.dq2_7", "plc.1.dq2_7"));
    data_model_.SetUiToDataMap("reactor.label_HC2804", DataDef("plc.1.dq2_8", "plc.1.dq2_8", "plc.1.dq2_8"));

    // analysis
    data_model_.SetUiToDataMap("analysis.label_TICA3110", DataDef("plc.1.temp22_pv", "plc.1.temp22_sv", "plc.1.temp22_sv"));
    data_model_.SetUiToDataMap("analysis.label_TICA3120", DataDef("plc.1.temp23_pv", "plc.1.temp23_sv", "plc.1.temp23_sv"));
    data_model_.SetUiToDataMap("analysis.label_TICA3130", DataDef("plc.1.temp24_pv", "plc.1.temp24_sv", "plc.1.temp24_sv"));
    data_model_.SetUiToDataMap("analysis.label_TICA3140", DataDef("plc.1.temp26_pv", "plc.1.temp26_sv", "plc.1.temp26_sv"));
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
                        event = new Ui::RefreshTextEvent(ui_info.ui_name, status, ui_info, data_info.id, data_info.timestamp, value);
                    }
                    else if (ui_info.type == WidgetType::ONOFF || ui_info.type == WidgetType::STATE)
                    {
                        event = new Ui::RefreshStateEvent(ui_info.ui_name, status, ui_info, data_info.id, data_info.timestamp, value.toInt());
                    }
                    else if (ui_info.type == WidgetType::PROCESS_VALUE)
                    {
                        event = new Ui::ProcessValueEvent(ui_info.ui_name, status, ui_info, data_info.id, data_info.timestamp);
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
    controls.push_back("widget_workflow");
    controls.push_back("widget_trend");
    controls.push_back("widget_history");

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
