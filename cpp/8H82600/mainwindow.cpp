#include <iostream>
#include <functional>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "form_gasfeed.h"
#include "events.h"
#include "dialog_setvalue.h"
#include "dialog_setposition.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui_(new Ui::MainWindow), data_manager_(QApplication::applicationDirPath().toStdString() )
{
    ui_->setupUi(this);
    QWidget* tab = new FormGasFeed();
    tab->setObjectName(QString::fromUtf8("tab"));
    ui_->tabWidget->addTab(tab, QString());

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
    data_model_.SetDataToUiMap("mfcpfc.4.pv", UiInfo(ui_->tabWidget->widget(0), QString::fromUtf8("textEdit"), 1, WidgetType::TEXT));
    data_model_.SetUiToDataMap();
}

void MainWindow::RefreshUi(std::shared_ptr<std::vector<goiot::DataInfo>> data_info_vec)
{
    if (data_info_vec)
    {
        for (const auto& data_info : *data_info_vec)
        {
            UiInfo ui_info = data_model_.GetUiInfo(data_info.id);
            if (!ui_info.ui_name.isEmpty())
            {
                Ui::ControlStatus status = data_info.result == 0 ? Ui::ControlStatus::OK : Ui::ControlStatus::FAILURE;
                QString value;
                switch (data_info.data_type)
                {
                case goiot::DataType::STR:
                    value.fromStdString(data_info.char_value);
                    break;
                case goiot::DataType::DF:
                    value = QString::number(data_info.float_value, 'f', ui_info.decimals);
                    break;
                case goiot::DataType::BT:
                    value = QString::number(data_info.byte_value);
                    break;
                case goiot::DataType::DB:
                case goiot::DataType::DUB:
                case goiot::DataType::WB:
                case goiot::DataType::WUB:
                    value = QString::number(data_info.int_value);
                    break;
                default:
                    throw std::invalid_argument("Unsupported data type");
                }
                QEvent* event = nullptr;
                if (ui_info.type == WidgetType::TEXT)
                {
                    event = new Ui::RefreshTextEvent(ui_info.ui_name, status, value);
                }
                else if (ui_info.type == WidgetType::STATE)
                {
                    event = new Ui::RefreshStateEvent(ui_info.ui_name, status, value.toInt());
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
                new Ui::RefreshTextEvent("textEdit", Ui::ControlStatus::OK, "xxyy"));
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
