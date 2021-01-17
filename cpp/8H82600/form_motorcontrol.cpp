#include <QPushButton>
#include "form_motorcontrol.h"
#include "ui_form_motorcontrol.h"


FormMotorControl::FormMotorControl(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormMotorControl)
{
    ui->setupUi(this);

    int row_count = 16;
    int col_count = 12;
    QStringList labels;
    labels.push_back("名称");
    labels.push_back("状态");
    labels.push_back("使能");
    labels.push_back("禁止");
    labels.push_back("报警");
    labels.push_back("清除");
    labels.push_back("运行");
    labels.push_back("当前位置ml");
    labels.push_back("目标位置ml");
    labels.push_back("速度ml/m");
    labels.push_back("移动");
    labels.push_back("停止");
    ui->tableWidget_cylinder16->setColumnCount(col_count);
    ui->tableWidget_cylinder16->setRowCount(row_count);
    ui->tableWidget_cylinder16->setHorizontalHeaderLabels(labels);
    ui->tableWidget_cylinder16->verticalHeader()->setVisible(false);
    ui->tableWidget_cylinder16->horizontalHeader()->setDefaultSectionSize(80);
    ui->tableWidget_cylinder16->setAlternatingRowColors(true);
    ui->tableWidget_cylinder16->resize(1200, 1100);
    //QTableWidgetItem* head_item = new QTableWidgetItem(QString("Last"),QTableWidgetItem::Type);
    //ui->tableWidget_cylinder16->setHorizontalHeaderItem(0, head_item);

    //ui.qtablewidget->setItem(1, 0, new QTableWidgetItem(str));
    //QString str =ui.qtablewidget->item(0, 0)->data(Qt::DisplayRole).toString();

    // Name
    std::vector<QString> names;
    for (int i = 0; i < 8; i++)
    {
        names.push_back(QString::number(i + 1) + "#PO缸");
        names.push_back(QString::number(i + 1) + "#EO缸");
    }
    for (int i = 0; i < row_count; i++)
    {
        ui->tableWidget_cylinder16->setItem(i, 0, new QTableWidgetItem(names.at(i)));
        ui->tableWidget_cylinder16->item(i, 0)->setFlags(Qt::NoItemFlags);
        ui->tableWidget_cylinder16->setItem(i, 1, new QTableWidgetItem("n/a"));
        ui->tableWidget_cylinder16->item(i, 1)->setFlags(Qt::NoItemFlags);
        ui->tableWidget_cylinder16->setItem(i, 4, new QTableWidgetItem("n/a"));
        ui->tableWidget_cylinder16->item(i, 4)->setFlags(Qt::NoItemFlags);
        ui->tableWidget_cylinder16->setItem(i, 6, new QTableWidgetItem("n/a"));
        ui->tableWidget_cylinder16->item(i, 6)->setFlags(Qt::NoItemFlags);
        ui->tableWidget_cylinder16->setItem(i, 7, new QTableWidgetItem("n/a"));
        ui->tableWidget_cylinder16->item(i, 7)->setFlags(Qt::NoItemFlags);
    }

    // Button
    for (int i = 0; i < row_count; i++)
    {
        // Button
        QPushButton *button = new QPushButton();
        button->setText("Enable");
        button->setCursor(Qt::PointingHandCursor);
        connect(button, &QPushButton::clicked, this, &FormMotorControl::OnBtnClicked);
        ui->tableWidget_cylinder16->setCellWidget(i, 2, button);

        button = new QPushButton();
        button->setText("Disable");
        button->setCursor(Qt::PointingHandCursor);
        connect(button, &QPushButton::clicked, this, &FormMotorControl::OnBtnClicked);
        ui->tableWidget_cylinder16->setCellWidget(i, 3, button);

        button = new QPushButton();
        button->setText("Clear");
        button->setCursor(Qt::PointingHandCursor);
        connect(button, &QPushButton::clicked, this, &FormMotorControl::OnBtnClicked);
        ui->tableWidget_cylinder16->setCellWidget(i, 5, button);

        button = new QPushButton();
        button->setText("<->");
        button->setCursor(Qt::PointingHandCursor);
        connect(button, &QPushButton::clicked, this, &FormMotorControl::OnBtnClicked);
        ui->tableWidget_cylinder16->setCellWidget(i, 10, button);

        button = new QPushButton();
        button->setText("Stop");
        button->setCursor(Qt::PointingHandCursor);
        connect(button, &QPushButton::clicked, this, &FormMotorControl::OnBtnClicked);
        ui->tableWidget_cylinder16->setCellWidget(i, 11, button);
    }
}

FormMotorControl::~FormMotorControl()
{
    delete ui;
}

void FormMotorControl::OnBtnClicked(void)
{
    QPushButton *senderObj = qobject_cast<QPushButton*>(sender());
    if(senderObj == nullptr)
    {
        return;
    }
    QModelIndex idx = ui->tableWidget_cylinder16->indexAt(
                QPoint(senderObj->frameGeometry().x(), senderObj->frameGeometry().y()));
    int row = idx.row();
    int col = idx.column();
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
                default:
                    break;
                }
                if (data_info_id.find(".multi_turn") != std::string::npos)
                {
                    tableWidget->item(data_index - 1, 7)->setText(e->Text() + unit);
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
        if (tableWidget != nullptr)
        {
           if (data_info_id.find(".brk_off") != std::string::npos)
           {
               tableWidget->item(data_index - 1, 1)->setText(QString::number(e->State()));
               tableWidget->item(data_index -1, 1)->setBackgroundColor(QColor(127,255,170));
           }
           else if (data_info_id.find(".alm") != std::string::npos)
           {
               tableWidget->item(data_index - 1, 4)->setText(QString::number(e->State()));
           }
           else if (data_info_id.find(".busy") != std::string::npos)
           {
               tableWidget->item(data_index - 1, 6)->setText(QString::number(e->State()));
           }
        }
        return true;
    }

    return QWidget::event(event);
}

