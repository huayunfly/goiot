#include "form_safety.h"
#include "ui_form_safety.h"
#include <QPushButton>
#include <QCheckBox>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <thread>

FormSafety::FormSafety(QWidget *parent, bool admin) :
    FormCommon(parent, "safety", QString::fromUtf8("安全"), admin),
    ui(new Ui::FormSafety)
{
    ui->setupUi(this);
    InitUiState();
    InitExpStatusTable();
    InitAlarmView();
    InitAlarmEnableTable();
    connect(&timer_, &QTimer::timeout, this, &FormSafety::CheckTotalAlarmBox);
    timer_.start(1800 * 1000); // call EnableTotalAlarm by 30min interval.
    QTimer::singleShot(10000, this, &FormSafety::EnableTotalAlarm); // Force enable alarm once.
}

void FormSafety::InitExpStatusTable(void)
{
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
    ui->expStatusTableWidget->resize(320, 800);

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
        // Disable, no cursor display.
        button->setEnabled(false);
        connect(button, &QPushButton::clicked, this, &FormSafety::on_buttonClicked);
        ui->expStatusTableWidget->setCellWidget(i, COL_RUN, button);

        button = new QPushButton();
        button->setText("Stop");
        button->setCursor(Qt::PointingHandCursor);
        connect(button, &QPushButton::clicked, this, &FormSafety::on_buttonClicked);
        ui->expStatusTableWidget->setCellWidget(i, COL_STOP, button);
    }
}

void FormSafety::InitAlarmView()
{
    // TC group A
    alarm_items_.push_back(std::make_pair<QString, std::vector<AlarmItemInfo>>(
                               "釜加热", {AlarmItemInfo("TC4101", "釜1底加热"),
                                       AlarmItemInfo("TC4102", "釜2底加热"),
                                       AlarmItemInfo("TC4103", "釜3底加热"),
                                       AlarmItemInfo("TC4104", "釜4底加热"),
                                       AlarmItemInfo("TC4105", "釜5底加热"),
                                       AlarmItemInfo("TC4106", "釜6底加热"),
                                       AlarmItemInfo("TC4107", "釜7底加热"),
                                       AlarmItemInfo("TC4108", "釜8底加热"),
                                       AlarmItemInfo("TC4109", "釜9底加热"),
                                       AlarmItemInfo("TC4110", "釜10底加热"),
                                       AlarmItemInfo("TC4111", "釜11底加热"),
                                       AlarmItemInfo("TC4112", "釜12底加热"),
                                       AlarmItemInfo("TC4113", "釜13底加热"),
                                       AlarmItemInfo("TC4114", "釜14底加热"),
                                       AlarmItemInfo("TC4115", "釜15底加热"),
                                       AlarmItemInfo("TC4116", "釜16底加热"),
                                       AlarmItemInfo("TC4121", "釜1顶加热"),
                                       AlarmItemInfo("TC4122", "釜2顶加热"),
                                       AlarmItemInfo("TC4123", "釜3顶加热"),
                                       AlarmItemInfo("TC4124", "釜4顶加热"),
                                       AlarmItemInfo("TC4125", "釜5顶加热"),
                                       AlarmItemInfo("TC4126", "釜6顶加热"),
                                       AlarmItemInfo("TC4127", "釜7顶加热"),
                                       AlarmItemInfo("TC4128", "釜8顶加热"),
                                       AlarmItemInfo("TC4129", "釜9顶加热"),
                                       AlarmItemInfo("TC4130", "釜10顶加热"),
                                       AlarmItemInfo("TC4131", "釜11顶加热"),
                                       AlarmItemInfo("TC4132", "釜12顶加热"),
                                       AlarmItemInfo("TC4133", "釜13顶加热"),
                                       AlarmItemInfo("TC4134", "釜14顶加热"),
                                       AlarmItemInfo("TC4135", "釜15顶加热"),
                                       AlarmItemInfo("TC4136", "釜16顶加热")}));
    // TC group B
    alarm_items_.push_back(std::make_pair<QString, std::vector<AlarmItemInfo>>(
                               "釜内测温", {AlarmItemInfo("TI4141", "釜1内测温"),
                           AlarmItemInfo("TI4142", "釜2内测温"),
                           AlarmItemInfo("TI4143", "釜3内测温"),
                           AlarmItemInfo("TI4144", "釜4内测温"),
                           AlarmItemInfo("TI4145", "釜5内测温"),
                           AlarmItemInfo("TI4146", "釜6内测温"),
                           AlarmItemInfo("TI4147", "釜7内测温"),
                           AlarmItemInfo("TI4148", "釜8内测温"),
                           AlarmItemInfo("TI4149", "釜9内测温"),
                           AlarmItemInfo("TI4150", "釜10内测温"),
                           AlarmItemInfo("TI4151", "釜11内测温"),
                           AlarmItemInfo("TI4152", "釜12内测温"),
                           AlarmItemInfo("TI4153", "釜13内测温"),
                           AlarmItemInfo("TI4154", "釜14内测温"),
                           AlarmItemInfo("TI4155", "釜15内测温"),
                           AlarmItemInfo("TI4156", "釜16内测温"),}));
    // TC group C
    alarm_items_.push_back(std::make_pair<QString, std::vector<AlarmItemInfo>>(
                               "管路伴热A", {AlarmItemInfo("TC4601", "釜1出管加热"),
                            AlarmItemInfo("TC4602", "釜2出管加热"),
                            AlarmItemInfo("TC4603", "釜3出管加热"),
                            AlarmItemInfo("TC4604", "釜4出管加热"),
                            AlarmItemInfo("TC4605", "釜5出管加热"),
                            AlarmItemInfo("TC4606", "釜6出管加热"),
                            AlarmItemInfo("TC4607", "釜7出管加热"),
                            AlarmItemInfo("TC4608", "釜8出管加热"),
                            AlarmItemInfo("TC4609", "釜9出管加热"),
                            AlarmItemInfo("TC4610", "釜10出管加热"),
                            AlarmItemInfo("TC4611", "釜11出管加热"),
                            AlarmItemInfo("TC4612", "釜12出管加热"),
                            AlarmItemInfo("TC4613", "釜13出管加热"),
                            AlarmItemInfo("TC4614", "釜14出管加热"),
                            AlarmItemInfo("TC4615", "釜15出管加热"),
                            AlarmItemInfo("TC4616", "釜16出管加热"),
                            AlarmItemInfo("TC5501", "气体采样阀箱A加热"),
                            AlarmItemInfo("TC5502", "气体采样阀箱B加热"),
                            AlarmItemInfo("TC6501", "液体收集阀箱A加热"),
                            AlarmItemInfo("TC6502", "液体收集阀箱B加热"),
                            AlarmItemInfo("TC7501", "液体采样箱A1加热"),
                            AlarmItemInfo("TC7502", "液体采样箱A2加热"),
                            AlarmItemInfo("TC7503", "液体采样箱B1加热"),
                            AlarmItemInfo("TC7504", "液体采样箱B2加热"),
                            AlarmItemInfo("TC5105", "GC1管路伴热"),
                            AlarmItemInfo("TC5205", "GC2管路伴热"),
                            AlarmItemInfo("TC6105", "液体收集选通管伴热"),
                            AlarmItemInfo("TC7218", "液体采样选通管伴热"),
                            AlarmItemInfo("TC6301", "移液仪收集管伴热"),
                            AlarmItemInfo("TC7301", "移液仪采样管伴热"),
                            AlarmItemInfo("TC6401", "移液仪座A加热"),
                            AlarmItemInfo("TC6402", "移液仪座B加热")}));
    // TC group D
    alarm_items_.push_back(std::make_pair<QString, std::vector<AlarmItemInfo>>(
                               "管路伴热B", {AlarmItemInfo("TC4701", "箱顶出管1伴热"),
                           AlarmItemInfo("TC4702", "箱顶出管2伴热"),
                           AlarmItemInfo("TC4703", "箱顶出管3伴热"),
                           AlarmItemInfo("TC4704", "箱顶出管4伴热"),
                           AlarmItemInfo("TC4705", "箱顶出管5伴热"),
                           AlarmItemInfo("TC4706", "箱顶出管6伴热"),
                           AlarmItemInfo("TC4707", "箱顶出管7伴热"),
                           AlarmItemInfo("TC4708", "箱顶出管8伴热"),
                           AlarmItemInfo("TC4709", "箱顶出管9伴热"),
                           AlarmItemInfo("TC4710", "箱顶出管10伴热"),
                           AlarmItemInfo("TC4711", "箱顶出管11伴热"),
                           AlarmItemInfo("TC4712", "箱顶出管12伴热"),
                           AlarmItemInfo("TC4713", "箱顶出管13伴热"),
                           AlarmItemInfo("TC4714", "箱顶出管14伴热"),
                           AlarmItemInfo("TC4715", "箱顶出管15伴热"),
                           AlarmItemInfo("TC4716", "箱顶出管16伴热"),
                           AlarmItemInfo("TC2305", "A泵头加热"),
                           AlarmItemInfo("TC2405", "B泵头加热"),
                           AlarmItemInfo("TC2306", "A泵出管伴热"),
                           AlarmItemInfo("TC2406", "B泵出管伴热"),
                           AlarmItemInfo("TC3501", "进液阀箱A1加热"),
                           AlarmItemInfo("TC3502", "进液阀箱A2加热"),
                           AlarmItemInfo("TC3503", "进液阀箱B1加热"),
                           AlarmItemInfo("TC3504", "进液阀箱B2加热"),
                           AlarmItemInfo("TC2407", "泵切换管至进液箱B"),
                           AlarmItemInfo("TC2315", "进液箱A1-A2连接管伴热"),
                           AlarmItemInfo("TC2415", "进液箱B1-B2连接管伴热"),
                           AlarmItemInfo("TC6104", "液体收集箱A1-A2/B1-B2互连管伴热"),
                           AlarmItemInfo("TC7211", "液体采样箱A1-A2/B1-B2互连管伴热")}));
    // PG
    alarm_items_.push_back(std::make_pair<QString, std::vector<AlarmItemInfo>>(
                               "气源压力", {AlarmItemInfo("PI1110", "H2气源压力"),
                           AlarmItemInfo("PI1120", "NH3气源压力"),
                           AlarmItemInfo("PI1130", "CH3Cl气源压力"),
                           AlarmItemInfo("PI1140", "N2气源压力"),
                           AlarmItemInfo("PI1010", "仪表气气源压力"),
                           AlarmItemInfo("PI1020", "吹扫N2气源压力"),
                           AlarmItemInfo("PI3110", "PO气源压力"),
                           AlarmItemInfo("PI3120", "EO气源压力")}));
    // GAS
    alarm_items_.push_back(std::make_pair<QString, std::vector<AlarmItemInfo>>(
                               "气体报警", {AlarmItemInfo("XI3901", "箱底EO1报警器"),
                           AlarmItemInfo("XI3902", "箱底EO2报警器"),
                           AlarmItemInfo("XI3903", "箱顶H2报警器"),
                           AlarmItemInfo("XI3904", "通风橱CO报警器")}));

    // Runtime signals
    alarm_items_.push_back(std::make_pair<QString, std::vector<AlarmItemInfo>>(
                               "信号报警", {AlarmItemInfo("EMER", "急停按钮"),
                           AlarmItemInfo("PHASE", "相序保护"),
                           AlarmItemInfo("SWTICH", "断路器辅助"),
                           AlarmItemInfo("POWER", "市电")}));

    // Group info, matching alarm_items_
    alarm_group_.emplace(std::make_pair<std::string, AlarmGroupInfo>(
                            "plc.1.alm_temp_high_a", AlarmGroupInfo(0, SafetyUIItem::SafetyUIItemStatus::HLimit, 32, 0)));
    alarm_group_.emplace(std::make_pair<std::string, AlarmGroupInfo>(
                            "plc.1.alm_temp_hhigh_a", AlarmGroupInfo(0, SafetyUIItem::SafetyUIItemStatus::HHLimit, 32, 0)));
    alarm_group_.emplace(std::make_pair<std::string, AlarmGroupInfo>(
                            "plc.1.alm_temp_break_a", AlarmGroupInfo(0, SafetyUIItem::SafetyUIItemStatus::TBreak, 32, 0)));
    alarm_group_.emplace(std::make_pair<std::string, AlarmGroupInfo>(
                            "plc.1.alm_temp_high_b", AlarmGroupInfo(1, SafetyUIItem::SafetyUIItemStatus::HLimit, 16, 32)));
    alarm_group_.emplace(std::make_pair<std::string, AlarmGroupInfo>(
                            "plc.1.alm_temp_hhigh_b", AlarmGroupInfo(1, SafetyUIItem::SafetyUIItemStatus::HHLimit, 16, 32)));
    alarm_group_.emplace(std::make_pair<std::string, AlarmGroupInfo>(
                            "plc.1.alm_temp_break_b", AlarmGroupInfo(1, SafetyUIItem::SafetyUIItemStatus::TBreak, 16, 32)));
    alarm_group_.emplace(std::make_pair<std::string, AlarmGroupInfo>(
                            "plc.1.alm_temp_high_c", AlarmGroupInfo(2, SafetyUIItem::SafetyUIItemStatus::HLimit, 32, 48)));
    alarm_group_.emplace(std::make_pair<std::string, AlarmGroupInfo>(
                            "plc.1.alm_temp_hhigh_c", AlarmGroupInfo(2, SafetyUIItem::SafetyUIItemStatus::HHLimit, 32, 48)));
    alarm_group_.emplace(std::make_pair<std::string, AlarmGroupInfo>(
                            "plc.1.alm_temp_break_c", AlarmGroupInfo(2, SafetyUIItem::SafetyUIItemStatus::TBreak, 32, 48)));
    alarm_group_.emplace(std::make_pair<std::string, AlarmGroupInfo>(
                            "plc.1.alm_temp_high_d", AlarmGroupInfo(3, SafetyUIItem::SafetyUIItemStatus::HLimit, 29, 80)));
    alarm_group_.emplace(std::make_pair<std::string, AlarmGroupInfo>(
                            "plc.1.alm_temp_hhigh_d", AlarmGroupInfo(3, SafetyUIItem::SafetyUIItemStatus::HHLimit, 29, 80)));
    alarm_group_.emplace(std::make_pair<std::string, AlarmGroupInfo>(
                            "plc.1.alm_temp_break_d", AlarmGroupInfo(3, SafetyUIItem::SafetyUIItemStatus::TBreak, 29, 80)));
    alarm_group_.emplace(std::make_pair<std::string, AlarmGroupInfo>(
                            "plc.1.alm_pg_high", AlarmGroupInfo(4, SafetyUIItem::SafetyUIItemStatus::HLimit, 8, 109)));
    alarm_group_.emplace(std::make_pair<std::string, AlarmGroupInfo>(
                            "plc.1.alm_pg_hhigh", AlarmGroupInfo(4, SafetyUIItem::SafetyUIItemStatus::HHLimit, 8, 109)));
    alarm_group_.emplace(std::make_pair<std::string, AlarmGroupInfo>(
                            "plc.1.alm_gas_high", AlarmGroupInfo(5, SafetyUIItem::SafetyUIItemStatus::HLimit, 4, 117)));
    alarm_group_.emplace(std::make_pair<std::string, AlarmGroupInfo>(
                            "plc.1.alm_gas_hhigh", AlarmGroupInfo(5, SafetyUIItem::SafetyUIItemStatus::HHLimit, 4, 117)));
    alarm_group_.emplace(std::make_pair<std::string, AlarmGroupInfo>(
                            "plc.1.di1_4", AlarmGroupInfo(6, SafetyUIItem::SafetyUIItemStatus::Trigger, 1, 121)));
    alarm_group_.emplace(std::make_pair<std::string, AlarmGroupInfo>(
                            "plc.1.di1_7", AlarmGroupInfo(6, SafetyUIItem::SafetyUIItemStatus::Failure, 1, 122)));
    alarm_group_.emplace(std::make_pair<std::string, AlarmGroupInfo>(
                            "plc.1.di1_8", AlarmGroupInfo(6, SafetyUIItem::SafetyUIItemStatus::Failure, 1, 123)));
    alarm_group_.emplace(std::make_pair<std::string, AlarmGroupInfo>(
                            "plc.1.di1_9", AlarmGroupInfo(6, SafetyUIItem::SafetyUIItemStatus::Failure, 1, 124)));

    // Draw the alarm view
    int cols = 16;
    int col_gap = 45;
    int row_gap = 40;
    int radius = 15;
    int group_row_start = row_gap;
    for (std::size_t group = 0; group < alarm_items_.size(); group++)
    {
        int rows = (alarm_items_.at(group).second.size() - 1) / cols + 1;
        int left = (alarm_items_.at(group).second.size() - 1) % cols + 1;
        for (int row = 0; row < rows; row++)
        {
            int item_in_col = (row + 1) == rows ? left : cols;
            for (int col = 0; col < item_in_col; col++)
            {
                const QString& id = alarm_items_.at(group).second.at(row * cols + col).id;
                const QString& note = alarm_items_.at(group).second.at(row * cols + col).note;
                auto item =
                        std::make_shared<SafetyUIItem>(radius, id, note);
                item->setPos((col + 1) * col_gap, group_row_start + row * row_gap);
                alarm_ui_items_.push_back(item);
            }
        }
        group_row_start += (rows + 1) * row_gap; // add a row gap for every group
    }
    auto scene = new QGraphicsScene(0, 0, (cols + 2) * col_gap, group_row_start);
    for (auto& item : alarm_ui_items_)
    {
        scene->addItem(item.get());
    }

    auto view = new QGraphicsView(scene);
    view->show();
    ui->verticalLayoutAlarmView->addWidget(view, 0, Qt::AlignTop | Qt::AlignLeft);
}

void FormSafety::InitAlarmEnableTable(void)
{
    QStringList labels;
    int col_group_num = 4;
    int row_num = (alarm_items_.size() + col_group_num - 1) / col_group_num;
    for (int i = 0; i < col_group_num; i++)
    {
        labels.push_back("报警组");
        labels.push_back("激活");
    }

    ui->alarmEnableTableWidget->setColumnCount(col_group_num * 2);
    ui->alarmEnableTableWidget->setRowCount(row_num);
    ui->alarmEnableTableWidget->setHorizontalHeaderLabels(labels);
    ui->alarmEnableTableWidget->horizontalHeader()->setVisible(true);
    ui->alarmEnableTableWidget->verticalHeader()->setVisible(false);
    ui->alarmEnableTableWidget->horizontalHeader()->setDefaultSectionSize(80);
    ui->alarmEnableTableWidget->setAlternatingRowColors(true);
    ui->alarmEnableTableWidget->resize(812, 130);

    std::size_t alarm_group_index = 0;
    for (int i = 0; i < row_num; i++)
    {
        for (int j = 0; j < col_group_num; j++)
        {
            if (alarm_group_index >= alarm_items_.size() - 1) // Exclude the Runtime signals group.
            {
                break;
            }
            // Set alarm group lable.
            ui->alarmEnableTableWidget->setItem(
                        i, 2 * j, new QTableWidgetItem(alarm_items_.at(alarm_group_index).first));
            ui->alarmEnableTableWidget->item(i, 2 * j)->setFlags(Qt::NoItemFlags);
            ui->alarmEnableTableWidget->setColumnWidth(2 * j, 100);
            // Set checkbox
            QCheckBox *box = new QCheckBox();
            box->setChecked(true);  
            connect(box, &QCheckBox::stateChanged, this, &FormSafety::on_boxStateChanged);
            // Admin
            if (admin_privilege_)
            {
                box->setEnabled(true);
            }
            else
            {
                box->setEnabled(false);
            }
            ui->alarmEnableTableWidget->setCellWidget(i, 2 * j + 1, box);
            ui->alarmEnableTableWidget->setColumnWidth(2 * j + 1, 60);
            alarm_group_index++;
        }
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
        const std::string& data_info_id = e->GetDataInfoId();
        // Parse data id, such as plc.1.reactor_1_run, plc.1.alm_temp_high_a
        if (0 == data_info_id.find("plc.1.reactor_"))
        {
            std::size_t pos = data_info_id.find("_");
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
        else if (0 == data_info_id.find("plc.1.alm_"))
        {
            auto iter = alarm_group_.find(data_info_id);
            if (iter != alarm_group_.end())
            {
                int num = (*iter).second.total_num;
                int offset = (*iter).second.item_offset;
                SafetyUIItem::SafetyUIItemStatus status = (*iter).second.status;
                std::vector<int> alarm_index;
                std::vector<int> normal_index;
                uint32_t test_bit = 1;
                uint32_t alm_bits = e->State(); // 64bit to 32bit force shift 0xFFFF FFFF 8000 0000 to 0xFFFF FFFF
                //alm_bits = alm_bits >> 32;
                // qWarning("ALM coming [%s] [%u] e.state[%d]", data_info_id.c_str(), alm_bits, e->State());
                // Adjust byte_order by 16bits or 32bits,
                // for PLC defined array of bit[16] or [32], but get data by s7-driver
                // WUB or DUB, which made big -> litter endian conversion.
                // Convert it back here:
                auto& byte_order = (num > 16) ? byte_order_big32_ : byte_order_big16_;
                for (int i = 0; i < num; i++)
                {
                    // Find alarm bit by byte_order conversion.
                    if ((test_bit << byte_order.at(i)) & alm_bits)
                    {
                        alarm_index.push_back(i);
                    }
                    else
                    {
                        normal_index.push_back(i);
                    }
                }
                for (std::size_t i = 0; i < alarm_index.size(); i++)
                {
                    int pos = offset + alarm_index.at(i);
                    if (alarm_ui_items_.at(pos)->Status() != status)
                    {
                         alarm_ui_items_.at(pos)->SetStatus(status);
                         qWarning("警报 [%s], [%s]", alarm_ui_items_.at(pos)->Note().toStdString().c_str(),
                               alarm_ui_items_.at(pos)->StatusNote().toStdString().c_str());
                    }
                }
                for (std::size_t i = 0; i < normal_index.size(); i++)
                {
                    int pos = offset + normal_index.at(i);
                    if (alarm_ui_items_.at(pos)->Status() == status)
                    {
                        alarm_ui_items_.at(pos)->SetStatus(
                                    SafetyUIItem::SafetyUIItemStatus::Normal);
                        qInfo("警报 [%s], [%s], 恢复正常", alarm_ui_items_.at(pos)->Note().toStdString().c_str(),
                              alarm_ui_items_.at(pos)->StatusNote().toStdString().c_str());
                    }
                }
            }
            return true;
        }
        else if (0 == data_info_id.find("plc.1.di1_")) // DI1 1-16 signals
        {
            auto iter = alarm_group_.find(data_info_id);
            if (iter != alarm_group_.end())
            {
                int pos = (*iter).second.item_offset;
                SafetyUIItem::SafetyUIItemStatus status = (*iter).second.status;
                if (e->State() > 0)
                {
                    if (alarm_ui_items_.at(pos)->Status() != status)
                    {
                         alarm_ui_items_.at(pos)->SetStatus(status);
                         qWarning("警报 [%s], [%s]", alarm_ui_items_.at(pos)->Note().toStdString().c_str(),
                               alarm_ui_items_.at(pos)->StatusNote().toStdString().c_str());
                    }
                }
                else
                {
                    if (alarm_ui_items_.at(pos)->Status() == status)
                    {
                        alarm_ui_items_.at(pos)->SetStatus(
                                    SafetyUIItem::SafetyUIItemStatus::Normal);
                        qInfo("警报 [%s], [%s], 恢复正常", alarm_ui_items_.at(pos)->Note().toStdString().c_str(),
                              alarm_ui_items_.at(pos)->StatusNote().toStdString().c_str());
                    }
                }
            }
            return true;
        }
    }
    return QWidget::event(event);
}

void FormSafety::CheckTotalAlarmBox()
{
    int group_num = alarm_items_.size() - 1; // exclude "信号报警" group
    int group_idx = 0;
    for (int row = 0; row < ui->alarmEnableTableWidget->rowCount(); row++)
    {
        for (int col = 1; col < ui->alarmEnableTableWidget->columnCount(); )
        {
            if (group_idx > group_num - 1)
            {
                return;
            }
            QCheckBox* checkbox = static_cast<QCheckBox*>(
                    ui->alarmEnableTableWidget->cellWidget(row, col));
            checkbox->setChecked(false);
            col += 2;
            group_idx++;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }
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

void FormSafety::EnableTotalAlarm(void)
{
    int idx_num = alarm_items_.size() - 1; // Exclude Runtime signals group
    for (int checkbox_idx = 0; checkbox_idx < idx_num; checkbox_idx++)
    {
        // Write data
        QString checkbox_id = "checkbox_alm_enable_" + QString::number(checkbox_idx + 1); // 1 based
        bool ok = write_data_func_(this->objectName(), checkbox_id, QString::number(0xffffffff));
        assert(ok);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void FormSafety::on_boxStateChanged(int state)
{
    QCheckBox *senderObj = qobject_cast<QCheckBox*>(sender());
    if(senderObj == nullptr)
    {
        return;
    }
    QModelIndex idx = ui->alarmEnableTableWidget->indexAt(
                QPoint(senderObj->frameGeometry().x(), senderObj->frameGeometry().y()));
    // Construct button id: checkbox_alm_enable_1 ...
    int row = idx.row();
    int col = idx.column();
    int checkbox_idx = row * (ui->alarmEnableTableWidget->columnCount() / 2) +
            col / 2/*note and checkbox pair*/;
    // Mapping alarm view items
    int alm_view_item_start = 0;
    for (int i = 0; i < checkbox_idx; i++)
    {
        alm_view_item_start += alarm_items_.at(i).second.size();
    }
    int alm_view_item_end = alm_view_item_start +
            alarm_items_.at(checkbox_idx).second.size();
    // Write data
    QString checkbox_id = "checkbox_alm_enable_" + QString::number(checkbox_idx + 1); // 1 based
    bool ok = false;
    if (0 == state)
    {
        ok = write_data_func_(this->objectName(), checkbox_id, QString::number(0));
        assert(ok);
    }
    else
    {
        ok = write_data_func_(this->objectName(), checkbox_id, QString::number(0xffffffff));
        assert(ok);
    }
    for (int i = alm_view_item_start; i < alm_view_item_end; i++)
    {
        alarm_ui_items_.at(i)->SetStatus(
                    (0 == state) ? SafetyUIItem::SafetyUIItemStatus::Inactive :
                                   SafetyUIItem::SafetyUIItemStatus::Normal);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}


