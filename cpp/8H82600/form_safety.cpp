#include "form_safety.h"
#include "ui_form_safety.h"
#include <QPushButton>
#include <thread>

FormSafety::FormSafety(QWidget *parent) :
    FormCommon(parent, "safety", QString::fromUtf8("安全")),
    ui(new Ui::FormSafety)
{
    ui->setupUi(this);
    InitUiState();

    QStringList labels;
    labels.push_back("名称");
    labels.push_back("实验状态");
    labels.push_back("运行");
    labels.push_back("停止");

    ui->expStatusTableWidget->setColumnCount(COL_COUNT);
    ui->expStatusTableWidget->setRowCount(ROW_COUNT);
    ui->expStatusTableWidget->setHorizontalHeaderLabels(labels);
    ui->expStatusTableWidget->horizontalHeader()->setVisible(true);
    ui->expStatusTableWidget->verticalHeader()->setVisible(false);
    ui->expStatusTableWidget->horizontalHeader()->setDefaultSectionSize(80);
    ui->expStatusTableWidget->setAlternatingRowColors(true);
    ui->expStatusTableWidget->resize(500, 1200);

    std::vector<QString> names;
    for (int i = 0; i < 16; i++)
    {
        names.push_back(QString::number(i + 1) + "#反应器");
    }

    for (int i = 0; i < ROW_COUNT; i++)
    {
        ui->expStatusTableWidget->setItem(i, COL_NAME, new QTableWidgetItem(names.at(i)));
        ui->expStatusTableWidget->item(i, COL_NAME)->setFlags(Qt::NoItemFlags);

        ui->expStatusTableWidget->setItem(i, COL_STATUS, new QTableWidgetItem("n/a"));
        ui->expStatusTableWidget->item(i, COL_STATUS)->setFlags(Qt::NoItemFlags);

        QPushButton *button = new QPushButton();
        button = new QPushButton();
        button->setText("Run");
        button->setCursor(Qt::PointingHandCursor);
        connect(button, &QPushButton::clicked, this, &FormSafety::on_buttonClicked);
        ui->expStatusTableWidget->setCellWidget(i, COL_RUN, button);

        button = new QPushButton();
        button->setText("Stop");
        button->setCursor(Qt::PointingHandCursor);
        connect(button, &QPushButton::clicked, this, &FormSafety::on_buttonClicked);
        ui->expStatusTableWidget->setCellWidget(i, COL_STOP, button);
    }
}

FormSafety::~FormSafety()
{
    delete ui;
}

bool FormSafety::event(QEvent *event)
{
    if (event == nullptr)
    {
        return false;
    }
    if (event->type() == Ui::RefreshStateEvent::myType)
    {
        Ui::RefreshStateEvent* e = static_cast<Ui::RefreshStateEvent*>(event);
        // Find target UI control
        auto tableWidget = this->findChild<QTableWidget*>(e->Name());
        // Parse data id, such as plc.1.reactor_1_run
        const std::string& data_info_id = e->GetDataInfoId();
        std::size_t pos = data_info_id.find("plc.1.reactor_");
        if (pos == std::string::npos)
        {
            return false;
        }
        std::size_t pos_end = data_info_id.find("_", pos + 1);
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
        // Update table cell
        if (tableWidget != nullptr)
        {
           if (data_info_id.find("_run") != std::string::npos)
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
        }
        return true;
    }
    return QWidget::event(event);
}

void FormSafety::on_buttonClicked()
{
    QPushButton *senderObj = qobject_cast<QPushButton*>(sender());
    if(senderObj == nullptr)
    {
        return;
    }
    QModelIndex idx = ui->expStatusTableWidget->indexAt(
                QPoint(senderObj->frameGeometry().x(), senderObj->frameGeometry().y()));
    // Construct button id: button_reactor_1_run ...
    int row = idx.row();
    int col = idx.column();
    QString button_id = "button_reactor_" + QString::number(row + 1) + "_";
    bool ok = false;
    if (col == COL_RUN)
    {
        button_id += "run";
        ok = write_data_func_(this->objectName(), button_id, QString::number(1));
        assert(ok);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    else if (col == COL_STOP)
    {
        button_id += "stop";
        ok = write_data_func_(this->objectName(), button_id, QString::number(0));
        assert(ok);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}
