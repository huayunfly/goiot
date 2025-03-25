#include <iostream>
#include <functional>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "form_reactor.h"
#include "form_trend.h"
#include "form_history.h"
#include "events.h"
#include "dialog_setvalue.h"
#include "dialog_setposition.h"
#include "dialog_onoff.h"
#include "resourcedef.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui_(new Ui::MainWindow),
      data_manager_(QApplication::applicationDirPath().toStdString() ),
      title_("800909控制系统")
{
    ui_->setupUi(this);

    // Control tab pages
    std::vector<FormCommon*> form_vec;
    form_vec.push_back(new FormReactor);

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

void MainWindow::closeEvent(QCloseEvent *event)
{
    auto result = QMessageBox::information(
                this, title_, "确认退出?",
                QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No);
    if (QMessageBox::StandardButton::Yes == result)
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void MainWindow::InitDataModel()
{
    // data_to_ui

    // reactor TC
    data_model_.SetDataToUiMap("e5cc.1.pv", {UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_TICA101"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0),
                               UiInfo((QWidget*)ui_->widget_trend->children().first(), QString::fromUtf8("chart_reactor"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0)});
    data_model_.SetDataToUiMap("e5cc.2.pv", {UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_TICA102"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0),
                               UiInfo((QWidget*)ui_->widget_trend->children().first(), QString::fromUtf8("chart_reactor"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0)});
    data_model_.SetDataToUiMap("e5cc.3.pv", {UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_TICA103"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0),
                               UiInfo((QWidget*)ui_->widget_trend->children().first(), QString::fromUtf8("chart_reactor"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0)});
    data_model_.SetDataToUiMap("e5cc.4.pv", {UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_TICA104"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0),
                               UiInfo((QWidget*)ui_->widget_trend->children().first(), QString::fromUtf8("chart_reactor"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0)});
    data_model_.SetDataToUiMap("e5cc.5.pv", {UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_TICA105"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0),
                               UiInfo((QWidget*)ui_->widget_trend->children().first(), QString::fromUtf8("chart_reactor"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0)});
    data_model_.SetDataToUiMap("e5cc.6.pv", {UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_TICA106"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0),
                               UiInfo((QWidget*)ui_->widget_trend->children().first(), QString::fromUtf8("chart_reactor"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0)});
    data_model_.SetDataToUiMap("e5cc.7.pv", {UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_TICA107"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0),
                               UiInfo((QWidget*)ui_->widget_trend->children().first(), QString::fromUtf8("chart_reactor"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0)});
    data_model_.SetDataToUiMap("e5cc.8.pv", {UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_TICA108"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0),
                               UiInfo((QWidget*)ui_->widget_trend->children().first(), QString::fromUtf8("chart_reactor"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0)});

    data_model_.SetDataToUiMap("e5cc.9.pv", {UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_TICA201"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0),
                               UiInfo((QWidget*)ui_->widget_trend->children().first(), QString::fromUtf8("chart_reactor2"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0)});
    data_model_.SetDataToUiMap("e5cc.10.pv", {UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_TICA202"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0),
                               UiInfo((QWidget*)ui_->widget_trend->children().first(), QString::fromUtf8("chart_reactor2"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0)});
    data_model_.SetDataToUiMap("e5cc.11.pv", {UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_TICA203"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0),
                               UiInfo((QWidget*)ui_->widget_trend->children().first(), QString::fromUtf8("chart_reactor2"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0)});
    data_model_.SetDataToUiMap("e5cc.12.pv", {UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_TICA204"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0),
                               UiInfo((QWidget*)ui_->widget_trend->children().first(), QString::fromUtf8("chart_reactor2"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0)});
    data_model_.SetDataToUiMap("e5cc.13.pv", {UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_TICA205"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0),
                               UiInfo((QWidget*)ui_->widget_trend->children().first(), QString::fromUtf8("chart_reactor2"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0)});
    data_model_.SetDataToUiMap("e5cc.14.pv", {UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_TICA206"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0),
                               UiInfo((QWidget*)ui_->widget_trend->children().first(), QString::fromUtf8("chart_reactor2"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0)});
    data_model_.SetDataToUiMap("e5cc.15.pv", {UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_TICA207"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0),
                               UiInfo((QWidget*)ui_->widget_trend->children().first(), QString::fromUtf8("chart_reactor2"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0)});
    data_model_.SetDataToUiMap("e5cc.16.pv", {UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_TICA208"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0),
                               UiInfo((QWidget*)ui_->widget_trend->children().first(), QString::fromUtf8("chart_reactor2"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0)});

    data_model_.SetDataToUiMap("e5cc.17.pv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_TICA301"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0));
    data_model_.SetDataToUiMap("e5cc.18.pv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_TICA302"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0));
    data_model_.SetDataToUiMap("e5cc.19.pv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_TICA303"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0));
    data_model_.SetDataToUiMap("e5cc.20.pv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_TICA304"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0));
    data_model_.SetDataToUiMap("e5cc.21.pv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_TICA305"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0));
    data_model_.SetDataToUiMap("e5cc.22.pv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_TICA306"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0));
    data_model_.SetDataToUiMap("e5cc.23.pv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_TICA307"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0));
    data_model_.SetDataToUiMap("e5cc.24.pv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_TICA308"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::DEGREE, 1, 500, 0));

    data_model_.SetDataToUiMap("e5cc.1.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_TICA101"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 500, 0));
    data_model_.SetDataToUiMap("e5cc.2.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_TICA102"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 500, 0));
    data_model_.SetDataToUiMap("e5cc.3.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_TICA103"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 500, 0));
    data_model_.SetDataToUiMap("e5cc.4.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_TICA104"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 500, 0));
    data_model_.SetDataToUiMap("e5cc.5.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_TICA105"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 500, 0));
    data_model_.SetDataToUiMap("e5cc.6.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_TICA106"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 500, 0));
    data_model_.SetDataToUiMap("e5cc.7.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_TICA107"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 500, 0));
    data_model_.SetDataToUiMap("e5cc.8.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_TICA108"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 500, 0));

    data_model_.SetDataToUiMap("e5cc.9.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_TICA201"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 500, 0));
    data_model_.SetDataToUiMap("e5cc.10.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_TICA202"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 500, 0));
    data_model_.SetDataToUiMap("e5cc.11.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_TICA203"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 500, 0));
    data_model_.SetDataToUiMap("e5cc.12.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_TICA204"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 500, 0));
    data_model_.SetDataToUiMap("e5cc.13.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_TICA205"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 500, 0));
    data_model_.SetDataToUiMap("e5cc.14.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_TICA206"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 500, 0));
    data_model_.SetDataToUiMap("e5cc.15.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_TICA207"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 500, 0));
    data_model_.SetDataToUiMap("e5cc.16.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_TICA208"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 500, 0));

    data_model_.SetDataToUiMap("e5cc.17.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_TICA301"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 500, 0));
    data_model_.SetDataToUiMap("e5cc.18.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_TICA302"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 500, 0));
    data_model_.SetDataToUiMap("e5cc.19.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_TICA303"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 500, 0));
    data_model_.SetDataToUiMap("e5cc.20.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_TICA304"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 500, 0));
    data_model_.SetDataToUiMap("e5cc.21.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_TICA305"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 500, 0));
    data_model_.SetDataToUiMap("e5cc.22.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_TICA306"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 500, 0));
    data_model_.SetDataToUiMap("e5cc.23.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_TICA307"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 500, 0));
    data_model_.SetDataToUiMap("e5cc.24.sv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("label_TICA308"), RES_TC, WidgetType::PROCESS_VALUE, MeasurementUnit::DEGREE, 0, 500, 0));

    // mfc
    data_model_.SetDataToUiMap("adam4017_1.1.pv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_FICA101"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::SCCM, 1, 3200, 0));
    data_model_.SetDataToUiMap("adam4017_2.1.pv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit_FICA102"), RES_EMPTY, WidgetType::TEXT, MeasurementUnit::SCCM, 1, 3200, 0));

    // reactor
    data_model_.SetUiToDataMap("reactor.label_TICA101", DataDef("e5cc.1.pv", "e5cc.1.sv", "e5cc.1.sv"));
    data_model_.SetUiToDataMap("reactor.label_TICA102", DataDef("e5cc.2.pv", "e5cc.2.sv", "e5cc.2.sv"));
    data_model_.SetUiToDataMap("reactor.label_TICA103", DataDef("e5cc.3.pv", "e5cc.3.sv", "e5cc.3.sv"));
    data_model_.SetUiToDataMap("reactor.label_TICA104", DataDef("e5cc.4.pv", "e5cc.4.sv", "e5cc.4.sv"));
    data_model_.SetUiToDataMap("reactor.label_TICA105", DataDef("e5cc.5.pv", "e5cc.5.sv", "e5cc.5.sv"));
    data_model_.SetUiToDataMap("reactor.label_TICA106", DataDef("e5cc.6.pv", "e5cc.6.sv", "e5cc.6.sv"));
    data_model_.SetUiToDataMap("reactor.label_TICA107", DataDef("e5cc.7.pv", "e5cc.7.sv", "e5cc.7.sv"));
    data_model_.SetUiToDataMap("reactor.label_TICA108", DataDef("e5cc.8.pv", "e5cc.8.sv", "e5cc.8.sv"));

    data_model_.SetUiToDataMap("reactor.label_TICA201", DataDef("e5cc.9.pv", "e5cc.9.sv", "e5cc.9.sv"));
    data_model_.SetUiToDataMap("reactor.label_TICA202", DataDef("e5cc.10.pv", "e5cc.10.sv", "e5cc.10.sv"));
    data_model_.SetUiToDataMap("reactor.label_TICA203", DataDef("e5cc.11.pv", "e5cc.11.sv", "e5cc.11.sv"));
    data_model_.SetUiToDataMap("reactor.label_TICA204", DataDef("e5cc.12.pv", "e5cc.12.sv", "e5cc.12.sv"));
    data_model_.SetUiToDataMap("reactor.label_TICA205", DataDef("e5cc.13.pv", "e5cc.13.sv", "e5cc.13.sv"));
    data_model_.SetUiToDataMap("reactor.label_TICA206", DataDef("e5cc.14.pv", "e5cc.14.sv", "e5cc.14.sv"));
    data_model_.SetUiToDataMap("reactor.label_TICA207", DataDef("e5cc.15.pv", "e5cc.15.sv", "e5cc.15.sv"));
    data_model_.SetUiToDataMap("reactor.label_TICA208", DataDef("e5cc.16.pv", "e5cc.16.sv", "e5cc.16.sv"));

    data_model_.SetUiToDataMap("reactor.label_TICA301", DataDef("e5cc.17.pv", "e5cc.17.sv", "e5cc.17.sv"));
    data_model_.SetUiToDataMap("reactor.label_TICA302", DataDef("e5cc.18.pv", "e5cc.18.sv", "e5cc.18.sv"));
    data_model_.SetUiToDataMap("reactor.label_TICA303", DataDef("e5cc.19.pv", "e5cc.19.sv", "e5cc.19.sv"));
    data_model_.SetUiToDataMap("reactor.label_TICA304", DataDef("e5cc.20.pv", "e5cc.20.sv", "e5cc.20.sv"));
    data_model_.SetUiToDataMap("reactor.label_TICA305", DataDef("e5cc.21.pv", "e5cc.21.sv", "e5cc.21.sv"));
    data_model_.SetUiToDataMap("reactor.label_TICA306", DataDef("e5cc.22.pv", "e5cc.22.sv", "e5cc.22.sv"));
    data_model_.SetUiToDataMap("reactor.label_TICA307", DataDef("e5cc.23.pv", "e5cc.23.sv", "e5cc.23.sv"));
    data_model_.SetUiToDataMap("reactor.label_TICA308", DataDef("e5cc.24.pv", "e5cc.24.sv", "e5cc.24.sv"));
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
                        fvalue += data_info.offset; // raw value * ratio + offset
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
            fvalue += data_info.offset; // raw value * ratio + offset
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
        float_value -= data_info.offset; // (ui value - offset) / ratio
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
