#include <thread>
#include <QPushButton>
#include "form_motorcontrol.h"
#include "ui_form_motorcontrol.h"


FormMotorControl::FormMotorControl(QWidget *parent,
                                   const QString& object_name,
                                   const QString& display_name,
                                   MotorGroup group, bool admin) :
    FormCommon(parent, object_name, display_name, admin), ui(new Ui::FormMotorControl),
    group_(group)
{
    ui->setupUi(this);
    InitUiState();

    QStringList labels;
    labels.push_back("名称");
    labels.push_back("状态");
    labels.push_back("使能");
    labels.push_back("禁止");
    labels.push_back("报警");
    labels.push_back("清除");
    labels.push_back("运行");
    labels.push_back("当前位置");
    labels.push_back("目标位置");
    if (group_ == MotorGroup::CYLINDER16 || group_ == MotorGroup::CYLINDER32)
    {
        labels.push_back("速度ml/m(0~40)");
    }
    else if (group_ == MotorGroup::REACTOR)
    {
        labels.push_back("速度rpm(0~600)");
    }
    else
    {
        throw std::invalid_argument("Unsupported Motor group");
    }

    labels.push_back("移动");
    labels.push_back("停止");
    ui->tableWidget->setColumnCount(COL_COUNT);
    ui->tableWidget->setRowCount(ROW_COUNT);
    ui->tableWidget->setHorizontalHeaderLabels(labels);
    ui->tableWidget->horizontalHeader()->setVisible(true);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->horizontalHeader()->setDefaultSectionSize(80);
    ui->tableWidget->setAlternatingRowColors(true);
    ui->tableWidget->resize(1200, 1100);
    //QTableWidgetItem* head_item = new QTableWidgetItem(QString("Last"),QTableWidgetItem::Type);
    //ui->tableWidget->setHorizontalHeaderItem(0, head_item);

    //ui.qtablewidget->setItem(1, 0, new QTableWidgetItem(str));
    //QString str =ui.qtablewidget->item(0, 0)->data(Qt::DisplayRole).toString();

    // Assign names according to the MotorGroup
    std::vector<QString> names;
    if (group_ == MotorGroup::CYLINDER16)
    {
        for (int i = 0; i < 8; i++)
        {
            names.push_back(QString::number(i + 1) + "#PO缸");
            names.push_back(QString::number(i + 1) + "#EO缸");
        }
    }
    else if (group_ == MotorGroup::CYLINDER32)
    {
        for (int i = 0; i < 8; i++)
        {
            names.push_back(QString::number(i + 1 + 8) + "#PO缸");
            names.push_back(QString::number(i + 1 + 8) + "#EO缸");
        }
    }
    else if (group_ == MotorGroup::REACTOR)
    {
        for (int i = 0; i < 16; i++)
        {
            names.push_back(QString::number(i + 1) + "#反应器");
        }
    }
    else
    {
        throw std::invalid_argument("Unsupported Motor group");
    }

    for (int i = 0; i < ROW_COUNT; i++)
    {
        ui->tableWidget->setItem(i, COL_NAME, new QTableWidgetItem(names.at(i)));
        ui->tableWidget->item(i, COL_NAME)->setFlags(Qt::NoItemFlags);
        ui->tableWidget->setItem(i, COL_STATUS, new QTableWidgetItem("n/a"));
        ui->tableWidget->item(i, COL_STATUS)->setFlags(Qt::NoItemFlags);
        ui->tableWidget->setItem(i, COL_ALARM, new QTableWidgetItem("n/a"));
        ui->tableWidget->item(i, COL_ALARM)->setFlags(Qt::NoItemFlags);
        ui->tableWidget->setItem(i, COL_RUN, new QTableWidgetItem("n/a"));
        ui->tableWidget->item(i, COL_RUN)->setFlags(Qt::NoItemFlags);
        ui->tableWidget->setItem(i, COL_PV, new QTableWidgetItem("n/a"));
        ui->tableWidget->item(i, COL_PV)->setFlags(Qt::NoItemFlags);
    }

    // Button with admin previlege.
    for (int i = 0; i < ROW_COUNT; i++)
    {
        // Button
        QPushButton *button = new QPushButton();
        button->setText("Enable");
        button->setCursor(Qt::PointingHandCursor);
        connect(button, &QPushButton::clicked, this, &FormMotorControl::on_buttonClicked);
        ui->tableWidget->setCellWidget(i, COL_ENABLE, button);
        admin_privilege_ ? button->setEnabled(true) : button->setEnabled(false);

        button = new QPushButton();
        button->setText("Disable");
        button->setCursor(Qt::PointingHandCursor);
        connect(button, &QPushButton::clicked, this, &FormMotorControl::on_buttonClicked);
        ui->tableWidget->setCellWidget(i, COL_DISABLE, button);
        admin_privilege_ ? button->setEnabled(true) : button->setEnabled(false);

        button = new QPushButton();
        button->setText("Clear");
        button->setCursor(Qt::PointingHandCursor);
        connect(button, &QPushButton::clicked, this, &FormMotorControl::on_buttonClicked);
        ui->tableWidget->setCellWidget(i, COL_CLEAR_ALARM, button);
        admin_privilege_ ? button->setEnabled(true) : button->setEnabled(false);

        button = new QPushButton();
        button->setText("<->");
        button->setCursor(Qt::PointingHandCursor);
        connect(button, &QPushButton::clicked, this, &FormMotorControl::on_buttonClicked);
        ui->tableWidget->setCellWidget(i, COL_START, button);
        admin_privilege_ ? button->setEnabled(true) : button->setEnabled(false);

        button = new QPushButton();
        button->setText("Stop");
        button->setCursor(Qt::PointingHandCursor);
        connect(button, &QPushButton::clicked, this, &FormMotorControl::on_buttonClicked);
        ui->tableWidget->setCellWidget(i, COL_STOP, button);
        admin_privilege_ ? button->setEnabled(true) : button->setEnabled(false);
    }
    // Editabel cell
    connect(ui->tableWidget, &QTableWidget::cellChanged, this, &FormMotorControl::on_cellChanged);
}

FormMotorControl::~FormMotorControl()
{
    delete ui;
}

void FormMotorControl::on_buttonClicked(void)
{
    QPushButton *senderObj = qobject_cast<QPushButton*>(sender());
    if(senderObj == nullptr)
    {
        return;
    }
    QModelIndex idx = ui->tableWidget->indexAt(
                QPoint(senderObj->frameGeometry().x(), senderObj->frameGeometry().y()));
    // Construct button id: button_1_start ...
    int row = idx.row();
    int col = idx.column();
    QString button_id = "button_" + QString::number(row + 1) + "_";
    bool ok = false;
    if (col == COL_ENABLE)
    {
        button_id += "enable";
        ok = write_data_func_(this->objectName(), button_id, QString::number(1));
        assert(ok);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    else if (col == COL_DISABLE)
    {
        button_id += "disable";
        ok = write_data_func_(this->objectName(), button_id, QString::number(0));
        assert(ok);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    else if (col == COL_CLEAR_ALARM)
    {
        button_id += "alarm_clear";
        ok = write_data_func_(this->objectName(), button_id, QString::number(1));
        assert(ok);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ok = write_data_func_(this->objectName(), button_id, QString::number(0));
    }
    else if (col == COL_START)
    {
        button_id += "start";
        ok = write_data_func_(this->objectName(), button_id, QString::number(1));
        assert(ok);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ok = write_data_func_(this->objectName(), button_id, QString::number(0));
    }
    else if (col == COL_STOP)
    {
        button_id += "stop";
        ok = write_data_func_(this->objectName(), button_id, QString::number(1));
        assert(ok);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ok = write_data_func_(this->objectName(), button_id, QString::number(0));
    }
}

bool FormMotorControl::event(QEvent *event)
{
    if (event == nullptr)
    {
        return false;
    }

    if (event->type() == Ui::RefreshTextEvent::myType)
    {
        Ui::RefreshTextEvent* e = static_cast<Ui::RefreshTextEvent*>(event);
        // Find target UI control
        auto tableWidget = this->findChild<QTableWidget*>(e->Name());
        // Parse data id, such as cylinder16.1.stb
        const std::string& data_info_id = e->GetDataInfoId();
        std::size_t pos = data_info_id.find(".");
        if (pos == std::string::npos)
        {
            return false;
        }
        std::size_t pos_end = data_info_id.find(".", pos + 1);
        if (pos_end == std::string::npos)
        {
            return false;
        }
        int data_index = std::atoi(data_info_id.substr(pos + 1, pos_end - pos - 1).c_str());
        if (data_index < 0)
        {
            return false;
        }

        if (tableWidget != nullptr)
        {
            if (e->Status() == Ui::ControlStatus::OK)
            {
                QString unit;
                switch (e->GetUiInfo().unit)
                {
                case MeasurementUnit::ML:
                    unit = "ml";
                    break;
                case MeasurementUnit::BARA:
                    unit = "bara";
                    break;
                case MeasurementUnit::BARG:
                    unit = "barg";
                    break;
                case MeasurementUnit::SCCM:
                    unit = "sccm";
                    break;
                case MeasurementUnit::DEGREE:
                    unit = u8"°C";
                    break;
                case MeasurementUnit::MM:
                    unit = "mm";
                    break;
                case MeasurementUnit::MLM:
                    unit = "mlm";
                    break;
                case MeasurementUnit::MPA:
                    unit = "mpa";
                    break;
                case MeasurementUnit::RPM:
                    unit = "rpm";
                    break;
                default:
                    break;
                }
                if (data_info_id.find(".multi_turn") != std::string::npos)
                {
                    tableWidget->item(data_index - 1, COL_PV)->setText(e->Text() + unit);
                }
            }
            else
            {
                ; // todo
            }
        }
        return true;
    }
    else if (event->type() == Ui::RefreshStateEvent::myType)
    {
        Ui::RefreshStateEvent* e = static_cast<Ui::RefreshStateEvent*>(event);
        // Find target UI control
        auto tableWidget = this->findChild<QTableWidget*>(e->Name());
        // Parse data id, such as cylinder16.1.stb
        const std::string& data_info_id = e->GetDataInfoId();
        std::size_t pos = data_info_id.find(".");
        if (pos == std::string::npos)
        {
            return false;
        }
        std::size_t pos_end = data_info_id.find(".", pos + 1);
        if (pos_end == std::string::npos)
        {
            return false;
        }
        int data_index = std::atoi(data_info_id.substr(pos + 1, pos_end - pos - 1).c_str());
        if (data_index < 0)
        {
            return false;
        }
        // Get UI info
        auto ui_info = e->GetUiInfo();
        // RGB(FF, FF, FF)white and RGB(F5, F5, F5)grey, choose table alternating row colors.
        QColor inactive_color =
                (data_index % 2) == 1 ? QColor(255, 255, 255) : QColor(245, 245, 245);
        QColor active_green_color = QColor(0, 255, 0);
        QColor alarm_red_color = QColor(255, 69, 0);
        // Update table cell
        if (tableWidget != nullptr)
        {
           if (data_info_id.find(".brk_off") != std::string::npos)
           {
               tableWidget->item(data_index - 1, COL_STATUS)->setText(QString::number(e->State()));
               if (e->State() > 0)
               {
                   tableWidget->item(data_index - 1, COL_STATUS)->setBackground(active_green_color);
               }
               else
               {
                   tableWidget->item(data_index - 1, COL_STATUS)->setBackground(inactive_color);
               }
           }
           else if (data_info_id.find(".alm") != std::string::npos)
           {
               tableWidget->item(data_index - 1, COL_ALARM)->setText(QString::number(e->State()));
               if (e->State() > 0)
               {
                   tableWidget->item(data_index - 1, COL_ALARM)->setBackground(alarm_red_color);
               }
               else
               {
                   tableWidget->item(data_index - 1, COL_ALARM)->setBackground(inactive_color);
               }
           }
           else if (data_info_id.find(".busy") != std::string::npos)
           {
               tableWidget->item(data_index - 1, COL_RUN)->setText(QString::number(e->State()));
               if (e->State() > 0)
               {
                   tableWidget->item(data_index - 1, COL_RUN)->setBackground(active_green_color);
               }
               else
               {
                   tableWidget->item(data_index - 1, COL_RUN)->setBackground(inactive_color);
               }
           }
        }
        return true;
    }

    return QWidget::event(event);
}


void FormMotorControl::on_cellChanged(int row, int column)
{
    // Construct cell id: cell_1_speed, (1) means no.
    QString cell_id = "cell_" + QString::number(row + 1) + "_";
    bool ok = false;
    if (column == COL_SPEED)
    {
        // Check value: low limit
        float speed = ui->tableWidget->item(row, column)->text().toFloat(&ok);
        if (!ok || speed < 0.0f)
        {
            ui->tableWidget->item(row, column)->setText(QString::fromUtf8("无效"));
            return;
        }
        // Check value: high limit
        if ((group_ == MotorGroup::CYLINDER16 || group_ == MotorGroup::CYLINDER32)
                && (speed > MAX_CYLINDER_FLOWRATE))
        {
            ui->tableWidget->item(row, column)->setText(QString::fromUtf8("无效"));
            return;
        }
        if ((group_ == MotorGroup::REACTOR) && (speed > MAX_REACTOR_SPEED))
        {
            ui->tableWidget->item(row, column)->setText(QString::fromUtf8("无效"));
            return;
        }

        // calculate motor speed in rpm
        int flowrate = 0;
        if (group_ == MotorGroup::CYLINDER16 || group_ == MotorGroup::CYLINDER32)
        {
            flowrate = speed / CYLINDER_VOLUME * TOTAL_TRAVEL / PITCH * CYLINDER_REDUCTION_RATIO;
        }
        else if (group_ == MotorGroup::REACTOR)
        {
            flowrate = speed * REACTOR_REDUCTION_RATIO;
        }
        else
        {
            throw std::invalid_argument("Unsupported Motor group");
        }
        // Write data
        cell_id += "speed";
        ok = write_data_func_(this->objectName(), cell_id, QString::number(flowrate));
        assert(ok);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    else if (column == COL_SV)
    {
        // Check value
        float sv = ui->tableWidget->item(row, column)->text().toFloat(&ok);
        if (!ok || sv > MAX_TARGET_IN_ML || sv < 0.0f)
        {
            ui->tableWidget->item(row, column)->setText(QString::fromUtf8("无效"));
            return;
        }

        // Calculate motor position in plus
        int position = sv / CYLINDER_VOLUME * TOTAL_TRAVEL * MOTOR_UNIT_PER_MILLIMETER;
        // Write data
        cell_id += "sv";
        ok = write_data_func_(this->objectName(), cell_id, QString::number(position));
        assert(ok);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}
