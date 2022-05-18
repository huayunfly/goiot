#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <cassert>
#include <algorithm>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QDateTime>
#include "form_liquiddistributor.h"
#include "ui_form_liquiddistributor.h"


FormLiquidDistributor::FormLiquidDistributor(QWidget *parent,
                                             const QString& object_name,
                                             const QString& display_name,
                                             const QString& connection_path,
                                             LiquidDistributorGroup group
                                             ) :
    FormCommon(parent, object_name, display_name),
    ui(new Ui::FormLiquidDistributor),
    group_(group),
    db_ready_(false)
{
    ui->setupUi(this);
    InitUiState();

    // DB connection and table preparation
    PrepareDB(connection_path, object_name);

    // Timer
    connect(&video_timer_0, &QTimer::timeout, this, &FormLiquidDistributor::UpdateImage);

    // Video
    video_cap_0.open(0);
    // Check if camera opened successfully
    if(video_cap_0.isOpened())
    {
        // Default resolutions of the frame are obtained.The default resolutions are system dependent.
        int frame_height = video_cap_0.get(cv::CAP_PROP_FRAME_HEIGHT);
        int frame_width = video_cap_0.get(cv::CAP_PROP_FRAME_WIDTH);
        video_frame_0 = cv::Mat::zeros(frame_height, frame_width, CV_8UC3);
        video_timer_0.start(66);
    }
    // Create image label
    image_label_0 = new QLabel(this);
    image_label_0->setScaledContents(true);
    ui->verticalLayout_video->addWidget(image_label_0, 0, Qt::AlignTop | Qt::AlignLeft);

    // position display
    const int x_gap = 35;
    const int y_gap = 35;
    const int radius = 15;
    std::vector<std::pair<double, double>> positions;
    int number = 1;
    double y_base = 0;
    for (int y = 0; y < 32; y++)
    {
        if (y >= 8 && y < 16)
        {
            y_base = y_gap;
        }
        else if (y >= 16 && y < 24)
        {
            y_base = 2 * y_gap;
        }
        else if (y >= 24)
        {
            y_base = 3 * y_gap;
        }
        for (int x = 0; x < 4; x++)
        {
            auto item =
                    std::make_shared<SamplingUIItem>(radius, number);
            item->setPos(x * x_gap + 2 * x_gap, y_base + y * y_gap + radius);
            sampling_ui_items.push_back(item);
            number++;
        }
    }
    // for sink positions
    for (int y = 0; y < 2; y++)
    {
        if (y == 0)
        {
            y_base = 8 * y_gap;
        }
        else
        {
            y_base = 26 * y_gap;
        }
        for (int x = 0; x < 4; x++)
        {
            auto item = std::make_shared<SamplingUIItem>(
                            radius,
                            129,
                            SamplingUIItem::SamplingUIItemStatus::Undischarge);
            item->setPos(x * x_gap + 2 * x_gap, y_base + radius);
            sampling_ui_items.push_back(item);
        }
    }

    auto scene = new QGraphicsScene(0, 0, 8 * x_gap, 35 * y_gap);
    //scene->addRect(0, 0, 300, 800);
    for (auto& item : sampling_ui_items)
    {
        scene->addItem(item.get());
    }

    auto view = new QGraphicsView(scene);
    view->show();
    ui->verticalLayout->addWidget(view, 0, Qt::AlignTop | Qt::AlignLeft);

    // table
    QStringList labels;

    const int LINE_SEPERATOR = 2;
    for (int group = 0; group < LINE_GROUPS + 1; group++)
    {
        if (group == LINE_SEPERATOR)
        {
            labels.push_back(""); // Seperator column
            continue;
        }
        labels.push_back("位号");
        labels.push_back("通道");
        labels.push_back("限流");
        labels.push_back("时间");
        labels.push_back("清洗");
    }

    ui->tableWidget->setColumnCount(COL_COUNT);
    ui->tableWidget->setRowCount(ROW_COUNT);
    ui->tableWidget->setHorizontalHeaderLabels(labels);
    ui->tableWidget->horizontalHeader()->setVisible(true);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->horizontalHeader()->setDefaultSectionSize(40);
    ui->tableWidget->verticalHeader()->setDefaultSectionSize(25);
    ui->tableWidget->setAlternatingRowColors(true);
    ui->tableWidget->resize(1200, 1100);

    QColor seperator_color = QColor(72, 118, 255);
    for (int row = 0; row < ROW_COUNT; row++)
    {
        for (int col = 0; col < COL_COUNT; col++)
        {
            if (row == ROW_COUNT / 2 || col == COL_COUNT / 2)
            {
                ui->tableWidget->setItem(row, col, new QTableWidgetItem(""));
                ui->tableWidget->item(row, col)->setFlags(Qt::NoItemFlags);
                ui->tableWidget->item(row, col)->setBackground(seperator_color);
                continue;
            }
            int row_positon = row;
            int line = col % LINE_ITEMS;
            int line_group = col / LINE_ITEMS;
            if (row > ROW_COUNT / 2)
            {
                row_positon--;
            }
            if (col > COL_COUNT / 2)
            {
                line = (col - 1) % LINE_ITEMS;
                line_group = (col - 1) / LINE_ITEMS;
            }

            // Initialize channel number, sampling_time, purge_time, solvent type
            QStringList channel_1_to_8({"", "1", "2", "3", "4", "5", "6", "7", "8"});
            QStringList channel_9_to_16({"", "9", "10", "11", "12", "13", "14", "15", "16"});
            QStringList sampling_time;
            for (int s = 1; s < 100; s++)
            {
                sampling_time.append(QString::number(s) + "s");
            }
            QStringList solvent_type({"", "L2", "L3", "L4", "L6", "L7", "L8"});

            if (line == COL_POS)
            {
                // Position
                auto pos = QString("#") + QString::number(row_positon * LINE_GROUPS + line_group + 1);
                ui->tableWidget->setItem(row, col, new QTableWidgetItem(pos));
                ui->tableWidget->item(row, col)->setFlags(Qt::NoItemFlags);
                ui->tableWidget->item(row, col)->setFont(QFont("arial", 9, QFont::Black));
            }
            else if (line == COL_CHANNEL)
            {
                // Channel
                QComboBox *combobox = new QComboBox();
                if (line_group < 2)
                {
                    combobox->addItems(channel_1_to_8);
                }
                else
                {
                    combobox->addItems(channel_9_to_16);
                }
                ui->tableWidget->setCellWidget(row, col, combobox);
            }
            else if (line == COL_FLOW_LIMIT)
            {
                // Flow limit?
                QCheckBox *checkbox = new QCheckBox();
                ui->tableWidget->setCellWidget(row, col, checkbox);
            }
            else if (line == COL_SAMPLING_TIME)
            {
                // Sampling time in s
                QComboBox *combobox = new QComboBox();
                combobox->addItems(sampling_time);
                ui->tableWidget->setCellWidget(row, col, combobox);
            }
            else if (line == COL_SOLVENT_TYPE)
            {
                // Solvent type
                QComboBox *combobox = new QComboBox();
                combobox->addItems(solvent_type);
                ui->tableWidget->setCellWidget(row, col, combobox);
                //ui->tableWidget->setItem(row, col, new QTableWidgetItem("p"));
                //ui->tableWidget->item(row, col)->setTextAlignment(Qt::AlignCenter);
            }
        }
    }
}

FormLiquidDistributor::~FormLiquidDistributor()
{
    delete ui;
    video_cap_0.release();
    if (db_.isOpen())
    {
       db_.close();
    }
}

bool FormLiquidDistributor::event(QEvent *event)
{
    if (event == nullptr)
    {
        return false;
    }

    return FormCommon::event(event);
}

QString FormLiquidDistributor::SaveLiquidSamplingProcedure()
{
    QString record;
    // search
    for (int row = 0; row < ROW_COUNT; row++)
    {
        if (row == ROW_COUNT / 2)
        {
            continue;
        }

        int valid_row = row;
        if (row > ROW_COUNT / 2)
        {
            valid_row--;
        }
        for (int group = 0; group < LINE_GROUPS; group++)
        {
            int start_col = group * LINE_ITEMS;
            if (group > 1)
            {
                start_col += 1; // Jump seperator
            }
            int pos = valid_row * LINE_GROUPS + group + 1;
            QComboBox* combobox = static_cast<QComboBox*>(
                    ui->tableWidget->cellWidget(row, start_col + COL_CHANNEL));
            int channel = 0;
            if (combobox->currentText() == QString())
            {
                continue;
            }
            else
            {
                channel = combobox->currentText().toInt();
            }
            QCheckBox* checkbox = static_cast<QCheckBox*>(
                    ui->tableWidget->cellWidget(row, start_col + COL_FLOW_LIMIT));
            int flow_limit = 0;
            if (checkbox->isChecked())
            {
                flow_limit = 1;
            }
            combobox = static_cast<QComboBox*>(
                    ui->tableWidget->cellWidget(row, start_col + COL_SAMPLING_TIME));
            int sampling_time = combobox->currentText().remove("s", Qt::CaseInsensitive).toUInt();
            combobox = static_cast<QComboBox*>(
                    ui->tableWidget->cellWidget(row, start_col + COL_SOLVENT_TYPE));
            int solvent_type = combobox->currentText().remove("L", Qt::CaseInsensitive).toUInt();
            int purge_time = 2 * sampling_time;
            // Append record
            record.append(QString::number(pos));
            record.append(",");
            record.append(QString::number(channel));
            record.append(",");
            record.append(QString::number(flow_limit));
            record.append(",");
            record.append(QString::number(sampling_time));
            record.append(",");
            record.append(QString::number(solvent_type));
            record.append(",");
            record.append(QString::number(purge_time));
            record.append(";");
        }
    }
    return record;
}

bool FormLiquidDistributor::SaveLiquidSamplingRecipe(const QString &recipe_name)
{
    std::vector<std::vector<QString>> values;

    // search
    for (int row = 0; row < ROW_COUNT; row++)
    {
        if (row == ROW_COUNT / 2)
        {
            continue;
        }

        int valid_row = row;
        if (row > ROW_COUNT / 2)
        {
            valid_row--;
        }
        for (int group = 0; group < LINE_GROUPS / 2; group++)
        {
            // start point: collection |beam| sampling (1a, 2a, 3b, 4b)
            //              collection |beam| sampling (5a, 6a, 7b, 8b)
            int start_col_a = group * LINE_ITEMS;
            int start_col_b = start_col_a + 2 * LINE_ITEMS + 1/*jump UI seperator*/;

            // pos
            int pos_a = valid_row * LINE_GROUPS + group + 1;
            int pos_b = pos_a + 2;

            // channel
            QComboBox* combobox = static_cast<QComboBox*>(
                    ui->tableWidget->cellWidget(row, start_col_a + COL_CHANNEL));
            int channel_a = 0;
            if (!combobox->currentText().isEmpty())
            {
                channel_a = combobox->currentText().toInt();
            }
            combobox = static_cast<QComboBox*>(
                    ui->tableWidget->cellWidget(row, start_col_b + COL_CHANNEL));
            int channel_b = 0;
            if (!combobox->currentText().isEmpty())
            {
                channel_b = combobox->currentText().toInt();
            }
            if (channel_a == 0 && channel_b == 0)
            {
                continue;
            }

            // flowlimit
            QCheckBox* checkbox = static_cast<QCheckBox*>(
                    ui->tableWidget->cellWidget(row, start_col_a + COL_FLOW_LIMIT));
            int flow_limit_a = checkbox->isChecked() ? 1 : 0;
            checkbox = static_cast<QCheckBox*>(
                    ui->tableWidget->cellWidget(row, start_col_b + COL_FLOW_LIMIT));
            int flow_limit_b = checkbox->isChecked() ? 1 : 0;

            // sampling time in second unit
            combobox = static_cast<QComboBox*>(
                    ui->tableWidget->cellWidget(row, start_col_a + COL_SAMPLING_TIME));
            int sampling_time_a = combobox->currentText().remove("s", Qt::CaseInsensitive).toUInt();
            combobox = static_cast<QComboBox*>(
                    ui->tableWidget->cellWidget(row, start_col_b + COL_SAMPLING_TIME));
            int sampling_time_b = combobox->currentText().remove("s", Qt::CaseInsensitive).toUInt();

            // solvent
            combobox = static_cast<QComboBox*>(
                    ui->tableWidget->cellWidget(row, start_col_a + COL_SOLVENT_TYPE));
            int solvent_type_a = 0;
            if (!combobox->currentText().isEmpty())
            {
                solvent_type_a = combobox->currentText().remove("L", Qt::CaseInsensitive).toUInt();
            }
            combobox = static_cast<QComboBox*>(
                    ui->tableWidget->cellWidget(row, start_col_b + COL_SOLVENT_TYPE));
            int solvent_type_b = 0;
            if (!combobox->currentText().isEmpty())
            {
                solvent_type_b = combobox->currentText().remove("L", Qt::CaseInsensitive).toUInt();
            }

            // sampling step
            values.push_back(std::vector<QString>());
            auto tail = values.rbegin();
            QString r_name = recipe_name + "_" + QString::number(QDateTime::currentSecsSinceEpoch());
            int type = TYPE_SAMPLING;
            int x = valid_row + 1;
            int y = group; // 0, 1
            int run_a = channel_a > 0 ? 1 : 0;
            int run_b = channel_b > 0 ? 1 : 0;
            tail->push_back(r_name);
            tail->push_back(QString::number(type));
            tail->push_back(QString::number(channel_a));
            tail->push_back(QString::number(channel_b));
            tail->push_back(QString::number(x));
            tail->push_back(QString::number(y));
            tail->push_back(QString::number(pos_a));
            tail->push_back(QString::number(pos_b));
            tail->push_back(QString::number(flow_limit_a));
            tail->push_back(QString::number(flow_limit_b));
            tail->push_back(QString::number(solvent_type_a));
            tail->push_back(QString::number(solvent_type_b));
            tail->push_back(QString::number(sampling_time_a));
            tail->push_back(QString::number(sampling_time_b));
            tail->push_back(QString::number(run_a));
            tail->push_back(QString::number(run_b));
            int control_code = type + (channel_a << 2) +
                    (channel_b << 7) + (x << 12) + (y << 18) +
                    (flow_limit_a << 19) + (flow_limit_b << 20) +
                    (solvent_type_a << 21) + (solvent_type_b << 25) +
                    (run_a << 29) + (run_b << 30);
            tail->push_back(QString::number(control_code));
            // purge step
            if (solvent_type_a != 0 || solvent_type_b != 0)
            {
                // Append value
                values.push_back(std::vector<QString>());
                tail = values.rbegin();
                int type = TYPE_SAMPLING_PURGE;
                int x = valid_row + 1;
                int y = group; // 0, 1
                int run_a = solvent_type_a > 0 ? 1 : 0;
                int run_b = solvent_type_b > 0 ? 1 : 0;
                tail->push_back(r_name);
                tail->push_back(QString::number(type));
                tail->push_back(QString::number(channel_a));
                tail->push_back(QString::number(channel_b));
                tail->push_back(QString::number(x));
                tail->push_back(QString::number(y));
                tail->push_back(QString::number(pos_a));
                tail->push_back(QString::number(pos_b));
                tail->push_back(QString::number(flow_limit_a));
                tail->push_back(QString::number(flow_limit_b));
                tail->push_back(QString::number(solvent_type_a));
                tail->push_back(QString::number(solvent_type_b));
                tail->push_back(QString::number(sampling_time_a));
                tail->push_back(QString::number(sampling_time_b));
                tail->push_back(QString::number(run_a));
                tail->push_back(QString::number(run_b));
                int control_code = type + (channel_a << 2) +
                        (channel_b << 7) + (x << 12) + (y << 18) +
                        (flow_limit_a << 19) + (flow_limit_b << 20) +
                        (solvent_type_a << 21) + (solvent_type_b << 25) +
                        (run_a << 29) + (run_b << 30);
                tail->push_back(QString::number(control_code));
            }
        }
    }
    if (db_ready_)
    {
        QSqlQuery query(db_);
        QString error_msg;
        bool ok = WriteRecipeToDB(recipe_table_name_, table_columns_, values, query, error_msg);
        if (!ok)
        {
            QMessageBox::critical(0, "保存配方失败", error_msg, QMessageBox::Ignore);
        }
        return ok;
    }
    else
    {
        QMessageBox::critical(0, "保存配方失败", "数据库未连接", QMessageBox::Ignore);
        return false;
    }
}

std::list<std::vector<int>> FormLiquidDistributor::SamplingRecordToList(
        const QString& record)
{
    auto list = std::list<std::vector<int>>();
    QStringList records = record.split(";", Qt::SkipEmptyParts);
    records.sort(); // Sort by the first position.
    for (auto& sub : records)
    {
        QStringList sub_list = sub.split(",", Qt::SkipEmptyParts);
        std::vector<int> vec;
        for (auto& item : sub_list)
        {
            vec.push_back(item.toInt());
        }
        list.push_back(vec);
    }
    return list;
}

void FormLiquidDistributor::LoadLiquidSamplingProcedure(const QString& record)
{
    auto record_list = SamplingRecordToList(record);
    FillTable(record_list);
    FillStatusChart(record_list);
}

void FormLiquidDistributor::ClearUITable()
{
    for (int row = 0; row < ROW_COUNT; row++)
    {
        if (row == ROW_COUNT / 2)
        {
            continue;
        }
        for (int group = 0; group < LINE_GROUPS; group++)
        {
            int start_col = group * LINE_ITEMS;
            if (group >= LINE_GROUPS / 2)
            {
                start_col++; // jump seperator
            }
            QComboBox* combobox = static_cast<QComboBox*>(
                    ui->tableWidget->cellWidget(row, start_col + COL_CHANNEL));
            combobox->setCurrentText(""); // empty channel
            QCheckBox* checkbox = static_cast<QCheckBox*>(
                    ui->tableWidget->cellWidget(row, start_col + COL_FLOW_LIMIT));
            checkbox->setChecked(false); // default value
            combobox = static_cast<QComboBox*>(
                    ui->tableWidget->cellWidget(row, start_col + COL_SAMPLING_TIME));
            combobox->setCurrentText("1s");
            combobox = static_cast<QComboBox*>(
                    ui->tableWidget->cellWidget(row, start_col + COL_SOLVENT_TYPE));
            combobox->setCurrentText(""); // empty solvent type (port)
        }
    }
}

void FormLiquidDistributor::FillUITableChannelInfo(
        int pos, int channel, int flowlimit, int duration, int cleanport)
{
    int row = (pos - 1) / LINE_GROUPS;
    if (row > (MAX_SAMPLING_POS / LINE_GROUPS - 1) / 2)
    {
        row++; // jump seperator
    }
    int start_col = ((pos - 1) % LINE_GROUPS) * LINE_ITEMS;
    if (start_col > LINE_GROUPS * LINE_ITEMS / 2 - 1)
    {
        start_col++; // jump seperator
    }
    QComboBox* combobox = static_cast<QComboBox*>(
            ui->tableWidget->cellWidget(row, start_col + COL_CHANNEL));
    combobox->setCurrentText(QString::number(channel));
    QCheckBox* checkbox = static_cast<QCheckBox*>(
            ui->tableWidget->cellWidget(row, start_col + COL_FLOW_LIMIT));
    if (flowlimit > 0)
    {
        checkbox->setChecked(true);
    }
    else
    {
        checkbox->setChecked(false);
    }
    combobox = static_cast<QComboBox*>(
            ui->tableWidget->cellWidget(row, start_col + COL_SAMPLING_TIME));
    combobox->setCurrentText(QString::number(duration) + "s");
    combobox = static_cast<QComboBox*>(
            ui->tableWidget->cellWidget(row, start_col + COL_SOLVENT_TYPE));
    combobox->setCurrentText(QString("L") + QString::number(cleanport));
}

bool FormLiquidDistributor::LoadLiquidSamplingRecipe(const QString &recipe_name)
{
    if (!db_ready_)
    {
        QMessageBox::critical(0, "加载配方失败", "数据库未连接", QMessageBox::Ignore);
        return false;
    }
    QString error_msg;
    std::vector<std::shared_ptr<std::vector<QString>>> value_list;
    QSqlQuery query(db_);
    bool ok = ReadRecipeFromDB(recipe_table_name_, recipe_name, table_columns_,
                               query, value_list, error_msg);
    if (!ok)
    {
        QMessageBox::critical(0, "加载配方失败", error_msg, QMessageBox::Ignore);
        return false;
    }

    // clear table first.
    if (value_list.size() > 0)
    {
        ClearUITable();
    }
    // fill table
    for (auto& values : value_list)
    {
        bool is_ok;
        int error_count = 0;
        int type = values->at(INDEX_TYPE).toInt(&is_ok);
        if (!is_ok) error_count++;
        int channel_a = values->at(INDEX_CHANNEL_A).toInt(&is_ok);
        if (!is_ok) error_count++;
        int channel_b = values->at(INDEX_CHANNEL_B).toInt(&is_ok);
        if (!is_ok) error_count++;
        int pos_a = values->at(INDEX_POS_A).toInt(&is_ok);
        if (!is_ok) error_count++;
        int pos_b = values->at(INDEX_POS_B).toInt(&is_ok);
        if (!is_ok) error_count++;
        int flowlimit_a = values->at(INDEX_FLOWLIMIT_A).toInt(&is_ok);
        if (!is_ok) error_count++;
        int flowlimit_b = values->at(INDEX_FLOWLIMIT_B).toInt(&is_ok);
        if (!is_ok) error_count++;
        int cleanport_a = values->at(INDEX_CLEANPORT_A).toInt(&is_ok);
        if (!is_ok) error_count++;
        int cleanport_b = values->at(INDEX_CLEANPORT_B).toInt(&is_ok);
        if (!is_ok) error_count++;
        int duration_a = values->at(INDEX_DURATION_A).toInt(&is_ok);
        if (!is_ok) error_count++;
        int duration_b = values->at(INDEX_DURATION_B).toInt(&is_ok);
        if (!is_ok) error_count++;

        if (error_count > 0)
        {
            ClearUITable();
            QMessageBox::critical(0, "加载配方失败", "数据格式错误", QMessageBox::Ignore);
            return false;
        }

        if (type == TYPE_SAMPLING)
        {
            if (channel_a > 0)
            {
                if (pos_a > MAX_SAMPLING_POS)
                {
                    assert(false);
                    continue;
                }
                FillUITableChannelInfo(
                            pos_a, channel_a, flowlimit_a, duration_a, cleanport_a);
            }
            if (channel_b > 0)
            {
                if (pos_b > MAX_SAMPLING_POS)
                {
                    assert(false);
                    continue;
                }
                FillUITableChannelInfo(
                            pos_b, channel_b, flowlimit_b, duration_b, cleanport_b);

            }
        }
    }
    return true;
}


void FormLiquidDistributor::FillTable(const std::list<std::vector<int>>& record_list)
{
    // clear table first.
    if (record_list.size() > 0)
    {
        for (int row = 0; row < ROW_COUNT; row++)
        {
            if (row == ROW_COUNT / 2)
            {
                continue;
            }
            for (int group = 0; group < LINE_GROUPS; group++)
            {
                int start_col = group * LINE_ITEMS;
                if (group >= LINE_GROUPS / 2)
                {
                    start_col++; // jump seperator
                }
                QComboBox* combobox = static_cast<QComboBox*>(
                        ui->tableWidget->cellWidget(row, start_col + COL_CHANNEL));
                combobox->setCurrentText(""); // empty channel
                QCheckBox* checkbox = static_cast<QCheckBox*>(
                        ui->tableWidget->cellWidget(row, start_col + COL_FLOW_LIMIT));
                checkbox->setChecked(false); // default value
                combobox = static_cast<QComboBox*>(
                        ui->tableWidget->cellWidget(row, start_col + COL_SAMPLING_TIME));
                combobox->setCurrentText("1s");
                combobox = static_cast<QComboBox*>(
                        ui->tableWidget->cellWidget(row, start_col + COL_SOLVENT_TYPE));
                combobox->setCurrentText("L2");
            }
        }
    }
    // fill table
    for (auto& vec : record_list)
    {
        int pos = vec.at(COL_POS);
        if (pos > MAX_SAMPLING_POS)
        {
            assert(false);
            continue;
        }
        // Map position to QTable row, reversed order.
        int row = (pos - 1) / LINE_GROUPS;
        if (row > (MAX_SAMPLING_POS / LINE_GROUPS - 1) / 2)
        {
            row++; // jump seperator
        }
        int start_col = ((pos - 1) % LINE_GROUPS) * LINE_ITEMS;
        if (start_col > LINE_GROUPS * LINE_ITEMS / 2 - 1)
        {
            start_col++; // jump seperator
        }
        QComboBox* combobox = static_cast<QComboBox*>(
                ui->tableWidget->cellWidget(row, start_col + COL_CHANNEL));
        combobox->setCurrentText(QString::number(vec.at(COL_CHANNEL)));
        QCheckBox* checkbox = static_cast<QCheckBox*>(
                ui->tableWidget->cellWidget(row, start_col + COL_FLOW_LIMIT));
        if (vec.at(COL_FLOW_LIMIT) > 0)
        {
            checkbox->setChecked(true);
        }
        else
        {
            checkbox->setChecked(false);
        }
        combobox = static_cast<QComboBox*>(
                ui->tableWidget->cellWidget(row, start_col + COL_SAMPLING_TIME));
        combobox->setCurrentText(QString::number(vec.at(COL_SAMPLING_TIME)) + "s");
        combobox = static_cast<QComboBox*>(
                ui->tableWidget->cellWidget(row, start_col + COL_SOLVENT_TYPE));
        combobox->setCurrentText(QString("L") + QString::number(vec.at(COL_SOLVENT_TYPE)));
    }
}

void FormLiquidDistributor::FillStatusChart(const std::list<std::vector<int>>& record_list)
{
    if (record_list.size() > 0)
    {
        for (int i = 0; i < MAX_SAMPLING_POS; i++)
        {
            sampling_ui_items.at(i)->SetStatus(
                        SamplingUIItem::SamplingUIItemStatus::Unsigned);
        }
        for (auto& vec : record_list)
        {
            int pos = vec.at(COL_POS);
            if (pos > MAX_SAMPLING_POS)
            {
                assert(false);
                continue;
            }
            sampling_ui_items.at(pos - 1)->SetStatus(
                        SamplingUIItem::SamplingUIItemStatus::Waiting);
        }
    }
}

void FormLiquidDistributor::InitUiState()
{
    ui->verticalLayout->installEventFilter(this);
}

void FormLiquidDistributor::paintEvent(QPaintEvent *e)
{
    QImage img = QImage(static_cast<uchar*>(video_frame_0.data),
                        video_frame_0.cols, video_frame_0.rows, QImage::Format_BGR888);
    image_label_0->setPixmap(QPixmap::fromImage(img));
    //image_label_0->resize(img.size());
    image_label_0->show();
    FormCommon::paintEvent(e);
}

void FormLiquidDistributor::UpdateImage()
{
    video_cap_0 >> video_frame_0;
    if (video_frame_0.data != nullptr)
    {
        cv::Mat gray, edges;
        std::vector<std::vector<cv::Point>> contours;
        std::vector<cv::Vec4i> hierarchy;
        cv::cvtColor(video_frame_0, gray, cv::COLOR_BGR2GRAY);
        cv::Canny(gray, edges, 60, 360, 3/*apertureSize*/, true/*L2gradient*/);
        cv::findContours(edges,
                         contours,
                         hierarchy,
                         cv::RETR_LIST,
                         cv::CHAIN_APPROX_SIMPLE);
        std::vector<int> lines_idx;
        for (std::size_t i = 0; i < contours.size(); i++)
        {
            // collect mid-length arc
            int arc_len = cv::arcLength(contours.at(i), false);
            if (arc_len > 80 && contours.at(i).size() < 300)
            {
                lines_idx.push_back(i);
            }
        }
        std::vector<int> match_idx;
        for (auto& idx : lines_idx)
        {
//            auto max = std::max_element(contours.at(idx).begin(),
//                                        contours.at(idx).end(),
//                                        [](cv::Point a, cv::Point b) {return a.y < b.y;});
//            int max_idx = std::distance(contours.at(idx).begin(), max);
//            auto min = std::min_element(contours.at(idx).begin(),
//                                        contours.at(idx).end(),
//                                        [](cv::Point a, cv::Point b) {return a.y > b.y;});
//            int min_idx = std::distance(contours.at(idx).begin(), min);
//            if (contours.at(idx).at(max_idx).y < 240 && contours.at(idx).at(min_idx).y > 100)
//            {
//                match_idx.push_back(idx);
//            }
            cv::Vec4f line;
            // find the optimal line
            cv::fitLine(contours.at(idx), line, cv::DIST_L2, 0, 0.01, 0.01);
            if (line[3] < 240 && line[3] > 120)
            {
                match_idx.push_back(idx);
            }
        }
        //cv::cvtColor(video_frame_0, video_frame_0, CV_BGR2RGB);
        for (auto& i : match_idx)
        {
            cv::drawContours(video_frame_0, contours, i, cv::Scalar(255, 0, 0), 3);
        }
        this->update();
    }
}

void FormLiquidDistributor::on_pushButton_clicked()
{
    QString recipe_name = "xb";
    bool ok = SaveLiquidSamplingRecipe(recipe_name);
    if (ok)
    {
        QMessageBox::information(0, "保存成功", recipe_name, QMessageBox::Ok);
    }
}

void FormLiquidDistributor::on_pushButton_2_clicked()
{
    //LoadLiquidSamplingProcedure("23,1,1,10,2;24,9,1,11,3;128,10,1,12,6");
    QString recipe_name = "xb_1652838895";
    bool ok = LoadLiquidSamplingRecipe(recipe_name);
    if (ok)
    {
        QMessageBox::information(0, "加载成功", recipe_name, QMessageBox::Ok);
    }
}

void FormLiquidDistributor::on_tableWidget_cellChanged(int row, int column)
{
    if (row >= ROW_COUNT || column >= COL_COUNT)
    {
        assert(false);
        return;
    }
    if (row == ROW_COUNT / 2)
    {
        return;
    }
    // detect channel changed.
    if (((column < COL_COUNT / 2) && (column % LINE_ITEMS == 1)) ||
            (column > COL_COUNT / 2 && ((column + 1) % LINE_ITEMS == 1)))

    {
        QComboBox* combobox = static_cast<QComboBox*>(
                ui->tableWidget->cellWidget(row, column));
        // calculate channel
        int row_channel = row;
        int col_channel = column;
        if (row > ROW_COUNT / 2)
        {
            row_channel--; // jump seperator
        }
        if (column > COL_COUNT / 2)
        {
            col_channel--;  // jump seperator
        }
        int channel = row_channel * LINE_GROUPS + col_channel / LINE_ITEMS + 1; // base 1
        if (combobox->currentText() == QString())
        {
            sampling_ui_items.at(channel - 1)->SetStatus(
                        SamplingUIItem::SamplingUIItemStatus::Unsigned);
        }
        else
        {
            sampling_ui_items.at(channel - 1)->SetStatus(
                        SamplingUIItem::SamplingUIItemStatus::Waiting);
        }
    }
}

void FormLiquidDistributor::PrepareDB(
        const QString& connection_path, const QString& instance_name)
{
    QStringList list = connection_path.split(":", Qt::SkipEmptyParts);
    if (list.length() == 6)
    {
        dbdriver_ = list[0];
        hostname_ = list[1];
        port_ = list[2].toInt();
        dbname_ = list[3];
        username_ = list[4];
        password_ = list[5];
    }

    // We must assign an unique connection name for addDatabase().
    // For there are more than one (sampling and collection) instance.
    db_ = QSqlDatabase::addDatabase(dbdriver_, instance_name);
    db_.setHostName(hostname_);
    db_.setPort(port_);
    db_.setDatabaseName(dbname_);
    db_.setUserName(username_);
    db_.setPassword(password_);
    bool is_open = db_.open();
    if (!is_open)
    {
        QMessageBox::critical(0, "连接数据库失败",
                              db_.lastError().text(), QMessageBox::Ignore);
    }
    else
    {
        // Create tables
        table_columns_ = {"recipe_name", "type", "channel_a", "channel_b",
                          "x", "y", "pos_a", "pos_b", "flowlimit_a",
                          "flowlimit_b", "cleanport_a", "cleanport_b",
                          "duration_a", "duration_b", "run_a", "run_b", "control_code"};
        QString error_message_1;
        QString error_message_2;
        bool ok1 = CreateDBTable(db_, recipe_table_name_, table_columns_, error_message_1);
        bool ok2 = CreateDBTable(db_, runtime_table_name_, table_columns_, error_message_2);
        if (!ok1 || !ok2)
        {
            QMessageBox::critical(0, "创建数据表失败",
                                  error_message_1 + "\n" + error_message_2, QMessageBox::Ignore);
        }
        else
        {
            db_ready_ = true;
        }
    }
}

bool FormLiquidDistributor::CreateDBTable(const QSqlDatabase& db,
                                          const QString& table_name,
                                          const std::vector<QString> columns,
                                          QString& error_message)
{
    if (!db.isValid() || !db.isOpen())
    {
        error_message = QString("数据库无效或未连接");
        return false;
    }
    QSqlQuery query(db);
    QString query_string;
    query_string.append("CREATE TABLE IF NOT EXISTS ");
    query_string.append(table_name);
    query_string.append(" (id SERIAL primary key, ");
    for (auto& column : columns)
    {
        query_string.append("\"");
        query_string.append(column);
        query_string.append("\" TEXT,");
    }
    query_string.append("time DOUBLE PRECISION, createtime TIMESTAMP WITH TIME ZONE not null default localtimestamp(0));");
    bool ok = query.exec(query_string);
    if (!ok)
    {
        error_message = query.lastError().text();
    }
    else
    {
        error_message = QString();
    }
    return ok;
}

bool FormLiquidDistributor::ReadRecipeFromDB(
        const QString& tablename, const QString& recipe_name,
        const std::vector<QString> columns, QSqlQuery& query,
        std::vector<std::shared_ptr<std::vector<QString>>>& value_list,
        QString& error_message)
{
    value_list.clear();

    QString query_string("select ");
    for (auto& column : columns)
    {
        query_string.append("\"");
        query_string.append(column);
        query_string.append("\", ");
    }
    if (query_string.endsWith(", "))
    {
        query_string.remove(query_string.size() - 2, 1); // remove the tail ","
    }
    query_string.append(" from ");
    query_string.append(tablename);
    query_string.append(" where \"recipe_name\"=\'");
    query_string.append(recipe_name);
    query_string.append("\';");
    bool ok = query.exec(query_string);
    if (!ok)
    {
        error_message = query.lastError().text();
        return false;
    }
    while (query.next())
    {
        auto values = std::make_shared<std::vector<QString>>();
        const QSqlRecord& record = query.record();
        for (auto& column : columns)
        {
            values->push_back(record.value(column).toString());
        }
        value_list.push_back(values);
    }
    return true;
}

bool FormLiquidDistributor::WriteRecipeStepToDB(
        const QString& tablename,const std::vector<QString> columns,
        const std::vector<QString> values, QSqlQuery& query, QString& error_message)
{
    QString query_string("insert into ");
    query_string.append(tablename);
    query_string.append(" (");
    for (auto& column : columns)
    {
        query_string.append("\"");
        query_string.append(column);
        query_string.append("\", ");
    }
    query_string.append("\"time\") VALUES (");
    for (auto& value : values)
    {
        query_string.append("'");
        query_string.append(value);
        query_string.append("', ");
    }
    query_string.append(
                QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch()));
    query_string.append(");");
    bool ok = query.exec(query_string);
    if (!ok)
    {
        error_message = query.lastError().text();
    }
    return ok;
}

// Write to recipe table.
bool FormLiquidDistributor::WriteRecipeToDB(const QString& tablename,
                                const std::vector<QString> columns,
                                const std::vector<std::vector<QString>> value_list,
                                QSqlQuery& query, QString& error_message)
{
    if (value_list.size() == 0 || columns.size() != value_list.at(0).size())
    {
        error_message = QString("配方为空");
        return false;
    }
    QString query_string;
    for (auto& values : value_list)
    {
        query_string.append("insert into ");
        query_string.append(tablename);
        query_string.append(" (");
        for (auto& column : columns)
        {
            query_string.append("\"");
            query_string.append(column);
            query_string.append("\", ");
        }
        query_string.append("\"time\") VALUES (");
        for (auto& value : values)
        {
            query_string.append("'");
            query_string.append(value);
            query_string.append("', ");
        }
        query_string.append(
                    QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch()));
        query_string.append(");");
    }
    bool ok = query.exec(query_string);
    if (!ok)
    {
        error_message = query.lastError().text();
    }
    return ok;
}

