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


    // ui_to_data
    //data_model_.SetUiToDataMap("gasfeed.svlabel", DataDef("plc.1.writebyte_channel_0", "plc.1.writebyte_channel_0", "plc.1.writebyte_channel_0"));
    //data_model_.SetUiToDataMap("gasfeed.svlabel_2", DataDef("plc.1.out1", "plc.1.out1", "plc.1.out1"));
    //data_model_.SetUiToDataMap("gasfeed.svlabel_3", DataDef("mfcpfc.4.pv", "mfcpfc.4.sv", "mfcpfc.4.sv"));
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
    data_id_vec.emplace_back(data_def.sv_read_id);
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

