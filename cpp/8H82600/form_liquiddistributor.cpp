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
#include "qrcode_generator.h"
#include "dialog_recipe_mgr.h"


FormLiquidDistributor::FormLiquidDistributor(QWidget *parent,
                                             const QString& object_name,
                                             const QString& display_name,
                                             const QString& connection_path,
                                             LiquidDistributorCategory category
                                             ) :
    FormCommon(parent, object_name, display_name),
    ui(new Ui::FormLiquidDistributor),
    category_(category),
    db_ready_(false),
    recipe_task_queue_(1),
    timers_(2)
{
    ui->setupUi(this);
    InitUiState();

    // DB connection and table preparation
    PrepareDB(connection_path, object_name);

    if (category == LiquidDistributorCategory::SAMPLING)
    {
        InitVideoCaps();
    }

    // For the recipe setting table
    InitRecipeSettingTable();
    // For the recipe runtime view
    InitRecipeRuntimeView();
    // thread worker
    threads_.emplace_back(std::thread(&FormLiquidDistributor::RunRecipeWorker, this));
}

FormLiquidDistributor::~FormLiquidDistributor()
{
    delete ui;
    for (auto& cap : vcaps_)
    {
        cap.release();
    }
    if (db_.isOpen())
    {
       db_.close();
    }
    recipe_task_queue_.Close();
    for (auto& entry : threads_)
    {
        entry.join();
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
    RecipeUITableSetting tbl = GetRecipeUITableSetting();
    // search
    for (int row = 0; row < tbl.row_count; row++)
    {
        if (row == tbl.row_count / 2)
        {
            continue;
        }

        int valid_row = row;
        if (row > tbl.row_count / 2)
        {
            valid_row--;
        }
        for (int group = 0; group < tbl.channel_groups; group++)
        {
            int start_col = group * tbl.line_items;
            if (group > 1)
            {
                start_col += 1; // Jump seperator
            }
            int pos = valid_row * tbl.channel_groups + group + 1;
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
    RecipeUITableSetting tbl = GetRecipeUITableSetting();

    // search
    for (int row = 0; row < tbl.row_count; row++)
    {
        if (row == tbl.row_count / 2)
        {
            continue;
        }

        int valid_row = row;
        if (row > tbl.row_count / 2)
        {
            valid_row--;
        }
        for (int group = 0; group < tbl.channel_groups / 2; group++)
        {
            // start point: collection |beam| sampling (1a, 2a, 3b, 4b)
            //              collection |beam| sampling (5a, 6a, 7b, 8b)
            int start_col_a = group * tbl.line_items;
            int start_col_b = start_col_a + 2 * tbl.line_items + 1/*jump UI seperator*/;

            // pos
            int pos_a = valid_row * tbl.channel_groups + group + 1;
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
            int x = valid_row + 1/*start 1*/;
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
                int x = valid_row + 1/*start 1*/;
                x = x > 16 ? 34 : 33; // purge x (1-16)33, (17-32)34
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

bool FormLiquidDistributor::SaveRecipe(const QString &recipe_name)
{
    std::vector<std::vector<QString>> values;
    RecipeUITableSetting tbl = GetRecipeUITableSetting();

    // search
    for (int row = 0; row < tbl.row_count; row++)
    {
        if (row == tbl.row_count / 2)
        {
            continue;
        }

        int valid_row = row;
        if (row > tbl.row_count / 2)
        {
            valid_row--;
        }
        int port_distance;
        if (category_ == LiquidDistributorCategory::SAMPLING)
        {
            port_distance = 2;

        }
        else if (category_ == LiquidDistributorCategory::COLLECTION)
        {
            port_distance = 1;
        }
        else
        {
            throw std::invalid_argument("Invalid category");
        }

        for (int group = 0; group < tbl.channel_groups / 2; group++)
        {
            // start point: (1a 2b) collection |beam| sampling (1a, 2a, 3b, 4b)
            //              (3a 4b) collection |beam| sampling (5a, 6a, 7b, 8b)
            int start_col_a = group * tbl.line_items;
            int start_col_b = start_col_a + port_distance * tbl.line_items + 1/*jump UI seperator*/;

            // pos
            int pos_a = valid_row * tbl.channel_groups + group + 1;
            int pos_b = pos_a + port_distance;

            // channel
            int index = 1;
            QComboBox* combobox = static_cast<QComboBox*>(
                    ui->tableWidget->cellWidget(row, start_col_a + index));
            int channel_a = 0;
            if (!combobox->currentText().isEmpty())
            {
                channel_a = combobox->currentText().toInt();
            }
            combobox = static_cast<QComboBox*>(
                    ui->tableWidget->cellWidget(row, start_col_b + index));
            int channel_b = 0;
            if (!combobox->currentText().isEmpty())
            {
                channel_b = combobox->currentText().toInt();
            }
            if (channel_a == 0 && channel_b == 0)
            {
                continue;
            }
            index++;

            // flowlimit
            int flow_limit_a = 0;
            int flow_limit_b = 0;
            if (category_ == LiquidDistributorCategory::SAMPLING)
            {
                QCheckBox* checkbox = static_cast<QCheckBox*>(
                            ui->tableWidget->cellWidget(row, start_col_a + index));
                flow_limit_a = checkbox->isChecked() ? 1 : 0;
                checkbox = static_cast<QCheckBox*>(
                            ui->tableWidget->cellWidget(row, start_col_b + index));
                flow_limit_b = checkbox->isChecked() ? 1 : 0;
                index++;
            }

            // sampling time in second unit
            combobox = static_cast<QComboBox*>(
                    ui->tableWidget->cellWidget(row, start_col_a + index));
            int sampling_time_a = combobox->currentText().remove("s", Qt::CaseInsensitive).toUInt();
            combobox = static_cast<QComboBox*>(
                    ui->tableWidget->cellWidget(row, start_col_b + index));
            int sampling_time_b = combobox->currentText().remove("s", Qt::CaseInsensitive).toUInt();
            index++;

            // solvent
            combobox = static_cast<QComboBox*>(
                    ui->tableWidget->cellWidget(row, start_col_a + index));
            int solvent_type_a = 0;
            if (!combobox->currentText().isEmpty())
            {
                solvent_type_a = combobox->currentText().remove("L", Qt::CaseInsensitive).toUInt();
            }
            combobox = static_cast<QComboBox*>(
                    ui->tableWidget->cellWidget(row, start_col_b + index));
            int solvent_type_b = 0;
            if (!combobox->currentText().isEmpty())
            {
                solvent_type_b = combobox->currentText().remove("L", Qt::CaseInsensitive).toUInt();
            }
            index++;

            // sampling or collection step
            values.push_back(std::vector<QString>());
            auto tail = values.rbegin();
            QString r_name = recipe_name + "_" + QString::number(QDateTime::currentSecsSinceEpoch());
            int type = (category_ == LiquidDistributorCategory::SAMPLING) ?
                        TYPE_SAMPLING : TYPE_COLLECTION;
            int x = group/*0, 1 or only 0*/;
            int y = (category_ == LiquidDistributorCategory::SAMPLING) ?
                        valid_row + 1/*start 1*/ : valid_row + 1 + 34;
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
            uint control_code = type + (channel_a << 2) +
                    (channel_b << 7) + (x << 12) + (y << 13) +
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
                int type = (category_ == LiquidDistributorCategory::SAMPLING) ?
                            TYPE_SAMPLING_PURGE : TYPE_COLLECTION_PURGE;
                int x = group/*0, 1 or only 0*/;
                int y = valid_row + 1/*start 1*/;
                if (category_ == LiquidDistributorCategory::SAMPLING)
                {
                    y = y > 16 ? 34 : 33; // sampling purge y (1-16)33, (17-32)34
                }
                else if (category_ == LiquidDistributorCategory::COLLECTION)
                {
                    y = y > 4 ? 44 : 43; // collection purge y (1-4)44, (5-8)43
                }
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
                uint control_code = type + (channel_a << 2) +
                        (channel_b << 7) + (x << 12) + (y << 13) +
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

RecipeUITableSetting FormLiquidDistributor::GetRecipeUITableSetting()
{
    if (category_ == LiquidDistributorCategory::SAMPLING)
    {
        return RecipeUITableSetting(33, 21, 4, 5, 128/*work pos*/);
    }
    else if (category_ == LiquidDistributorCategory::COLLECTION)
    {
        return RecipeUITableSetting(9, 9, 2, 4, 16/*work pos*/);
    }
    else
    {
        throw std::invalid_argument("unknown category");
    }
}

void FormLiquidDistributor::InitVideoCaps()
{
    vcaps_.resize(2);
    vframes_.resize(2);
    image_labels_.push_back(new QLabel(this));
    image_labels_.push_back(new QLabel(this));
    std::vector<QVBoxLayout*> boxes {ui->verticalLayout_video,
                                    ui->verticalLayout_video_2};
    // Timer
    for (int i = 0; i < 2; i++)
    {
        connect(&timers_.at(i), &QTimer::timeout, this, [=] () { UpdateImage(i);});
    }

    // Video
    for (int i = 0; i < 2; i++)
    {
        // Check if camera opened successfully
        vcaps_.at(i).open(i);
        if (vcaps_.at(i).isOpened())
        {
            // Default resolutions of the frame are obtained.The default resolutions are system dependent.
            int frame_height = vcaps_.at(i).get(cv::CAP_PROP_FRAME_HEIGHT);
            int frame_width = vcaps_.at(i).get(cv::CAP_PROP_FRAME_WIDTH);
            vframes_.at(i) = cv::Mat::zeros(frame_height, frame_width, CV_8UC3);
            timers_.at(i).start(66);
        }
        // Create image label
        image_labels_.at(i)->setScaledContents(true);
        boxes.at(i)->addWidget(image_labels_.at(i), 0, Qt::AlignTop | Qt::AlignLeft);
    }
}

void FormLiquidDistributor::InitRecipeRuntimeView()
{
    if (category_ == LiquidDistributorCategory::SAMPLING)
    {
        InitRecipeRuntimeView(35, 35, 15, 4, 32, 8);
    }
    else if (category_ == LiquidDistributorCategory::COLLECTION)
    {
        InitRecipeRuntimeView(65, 65, 30, 2, 8, 2);
    }
}

void FormLiquidDistributor::InitRecipeRuntimeView(int x_gap, int y_gap, double radius,
                                                  int x_count, int y_count, int y_section)
{
    std::vector<std::pair<double, double>> positions;
    int number = 1;
    double y_base = 0;
    for (int y = 0; y < y_count; y++)
    {
        if (y >= y_section && y < 2 * y_section)
        {
            y_base = y_gap;
        }
        else if (y >= 2 * y_section && y < 3 * y_section)
        {
            y_base = 2 * y_gap;
        }
        else if (y >= 3 * y_section)
        {
            y_base = 3 * y_gap;
        }
        for (int x = 0; x < x_count; x++)
        {
            auto item =
                    std::make_shared<SamplingUIItem>(radius, number);
            item->setPos(x * x_gap + 2 * x_gap, y_base + y * y_gap + radius);
            sampling_ui_items.push_back(item);
            number++;
        }
    }
    // for 2 sink positions
    for (int y = 0; y < 2; y++)
    {
        if (y == 0)
        {
            y_base = y_section * y_gap;
        }
        else
        {
            y_base = (3 * y_section + 2) * y_gap;
        }
        for (int x = 0; x < x_count; x++)
        {
            auto item = std::make_shared<SamplingUIItem>(
                            radius,
                            129,
                            0,
                            SamplingUIItem::SamplingUIItemStatus::Undischarge);
            item->setPos(x * x_gap + 2 * x_gap, y_base + radius);
            sampling_ui_items.push_back(item);
            number++;
        }
    }

    auto scene = new QGraphicsScene(0, 0, x_count * (2 * x_gap), (y_count + 3) * y_gap);
    //scene->addRect(0, 0, 300, 800);
    for (auto& item : sampling_ui_items)
    {
        scene->addItem(item.get());
    }

    auto view = new QGraphicsView(scene);
    view->show();
    ui->verticalLayout->addWidget(view, 0, Qt::AlignTop | Qt::AlignLeft);
}

void FormLiquidDistributor::ClearRecipeRuntimeView()
{
    RecipeUITableSetting tbl = GetRecipeUITableSetting();
    for (int i = 0; i < tbl.work_positions; i ++)
    {
        sampling_ui_items.at(i)->SetStatus(
                    SamplingUIItem::SamplingUIItemStatus::Unsigned, 0);
    }
}

void FormLiquidDistributor::InitRecipeSettingTable()
{
    if (category_ == LiquidDistributorCategory::SAMPLING)
    {
        QStringList heads {"位号", "通道", "限流", "时间", "清洗"};
        int channel_groups = 4;
        InitRecipeSettingTable(2, channel_groups,
                               channel_groups * heads.count() + 1/*seperator*/,
                               128 / channel_groups + 1/*seperator*/, heads);
    }
    else if (category_ == LiquidDistributorCategory::COLLECTION)
    {
        QStringList heads {"位号", "通道", "时间", "清洗"};
        int channel_groups = 2;
        InitRecipeSettingTable(1, channel_groups,
                               channel_groups * heads.count() + 1/*seperator*/,
                               16 / channel_groups + 1/*seperator*/, heads);
    }
}

void FormLiquidDistributor::InitRecipeSettingTable(int line_seperator,
                                                       int channel_groups,
                                                       int col_count,
                                                       int row_count,
                                                       const QStringList& heads)
{
    int line_items = heads.count();
    QStringList labels;

    for (int group = 0; group < channel_groups + 1; group++)
    {
        if (group == line_seperator)
        {
            labels.push_back(""); // Seperator column
            continue;
        }
        labels.append(heads);
    }

    ui->tableWidget->setColumnCount(col_count);
    ui->tableWidget->setRowCount(row_count);
    ui->tableWidget->setHorizontalHeaderLabels(labels);
    ui->tableWidget->horizontalHeader()->setVisible(true);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->horizontalHeader()->setDefaultSectionSize(50);
    ui->tableWidget->verticalHeader()->setDefaultSectionSize(25);
    ui->tableWidget->setAlternatingRowColors(true);
    ui->tableWidget->resize(1200, 1100);
    QColor seperator_color = QColor(72, 118, 255);

    // Initialize channel number, sampling_time, purge_time, solvent type
    QStringList channel_1_to_8({"", "1", "2", "3", "4", "5", "6", "7", "8"});
    QStringList channel_9_to_16({"", "9", "10", "11", "12", "13", "14", "15", "16"});
    QStringList sampling_time;
    for (int s = 1; s < 100; s++)
    {
        sampling_time.append(QString::number(s) + "s");
    }
    QStringList solvent_type_a({"", "L2", "L3", "L4"});
    QStringList solvent_type_b({"", "L6", "L7", "L8"});

    for (int row = 0; row < row_count; row++)
    {
        if (row == row_count / 2)
        {
            for (int col = 0; col < col_count; col++)
            {
                ui->tableWidget->setItem(row, col, new QTableWidgetItem(""));
                ui->tableWidget->item(row, col)->setFlags(Qt::NoItemFlags);
                ui->tableWidget->item(row, col)->setBackground(seperator_color);
            }
            continue;
        }
        for (int col = 0; col < col_count;)
        {
            if (col == col_count / 2)
            {
                ui->tableWidget->setItem(row, col, new QTableWidgetItem(""));
                ui->tableWidget->item(row, col)->setFlags(Qt::NoItemFlags);
                ui->tableWidget->item(row, col)->setBackground(seperator_color);
                col++;
                continue;
            }
            int row_position = row;
            int line_group = col / line_items;
            if (row > row_count / 2)
            {
                row_position--;
            }
            int offset = 0;

            // Position
            auto pos = QString("#") + QString::number(row_position * channel_groups + line_group + 1);
            ui->tableWidget->setItem(row, col + offset, new QTableWidgetItem(pos));
            ui->tableWidget->item(row, col + offset)->setFlags(Qt::NoItemFlags);
            ui->tableWidget->item(row, col + offset)->setFont(QFont("arial", 9, QFont::Black));
            offset++;

            // Channel
            QComboBox *combobox = new QComboBox();
            connect(combobox, &QComboBox::currentTextChanged, this, &FormLiquidDistributor::SelectChannelChanged);
            if (line_group < channel_groups / 2)
            {
                combobox->addItems(channel_1_to_8);
            }
            else
            {
                combobox->addItems(channel_9_to_16);
            }
            ui->tableWidget->setCellWidget(row, col + offset, combobox);
            offset++;

            // Flow limit
            if (category_ == LiquidDistributorCategory::SAMPLING)
            {
                QCheckBox *checkbox = new QCheckBox();
                ui->tableWidget->setCellWidget(row, col + offset, checkbox);
                offset++;
            }

            // Sampling time in s
            QComboBox *combobox_t = new QComboBox();
            combobox_t->addItems(sampling_time);
            ui->tableWidget->setCellWidget(row, col + offset, combobox_t);
            offset++;

            // Solvent type
            QComboBox *combobox_s = new QComboBox();
            if (line_group < channel_groups / 2)
            {
                combobox_s->addItems(solvent_type_a);
            }
            else
            {
                combobox_s->addItems(solvent_type_b);
            }
            ui->tableWidget->setCellWidget(row, col + offset, combobox_s);

            // next group
            col += line_items;
        }
    }
}

void FormLiquidDistributor::ClearUITable()
{
    RecipeUITableSetting tbl = GetRecipeUITableSetting();
    for (int row = 0; row < tbl.row_count; row++)
    {
        if (row == tbl.row_count / 2)
        {
            continue;
        }
        for (int group = 0; group < tbl.channel_groups; group++)
        {
            int offset = 1;
            int start_col = group * tbl.line_items;
            if (group >= tbl.channel_groups / 2)
            {
                start_col++; // jump seperator
            }
            QComboBox* combobox = static_cast<QComboBox*>(
                    ui->tableWidget->cellWidget(row, start_col + offset));
            combobox->setCurrentText(""); // empty channel
            offset++;
            if (category_ == LiquidDistributorCategory::SAMPLING)
            {
                QCheckBox* checkbox = static_cast<QCheckBox*>(
                        ui->tableWidget->cellWidget(row, start_col + offset));
                checkbox->setChecked(false); // default value
                offset++;
            }
            combobox = static_cast<QComboBox*>(
                    ui->tableWidget->cellWidget(row, start_col + offset));
            combobox->setCurrentText("1s");
            offset++;
            combobox = static_cast<QComboBox*>(
                    ui->tableWidget->cellWidget(row, start_col + offset));
            combobox->setCurrentText(""); // empty solvent type (port)
        }
    }
}

void FormLiquidDistributor::FillUITableChannelInfo(
        int pos, int channel, int flowlimit, int duration, int cleanport)
{
    RecipeUITableSetting tbl = GetRecipeUITableSetting();

    int row = (pos - 1) / tbl.channel_groups;
    if (row > ((tbl.row_count - 1) / 2 - 1))
    {
        row++; // jump seperator
    }
    int start_col = ((pos - 1) % tbl.channel_groups) * tbl.line_items;
    if (start_col > tbl.channel_groups * tbl.line_items / 2 - 1)
    {
        start_col++; // jump seperator
    }
    int index = 1;
    QComboBox* combobox = static_cast<QComboBox*>(
            ui->tableWidget->cellWidget(row, start_col + index));
    combobox->setCurrentText(QString::number(channel));
    index++;
    if (category_ == LiquidDistributorCategory::SAMPLING)
    {
        QCheckBox* checkbox = static_cast<QCheckBox*>(
                ui->tableWidget->cellWidget(row, start_col + index));
        if (flowlimit > 0)
        {
            checkbox->setChecked(true);
        }
        else
        {
            checkbox->setChecked(false);
        }
        index++;
    }

    combobox = static_cast<QComboBox*>(
            ui->tableWidget->cellWidget(row, start_col + index));
    combobox->setCurrentText(QString::number(duration) + "s");
    index++;
    combobox = static_cast<QComboBox*>(
            ui->tableWidget->cellWidget(row, start_col + index));
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

bool FormLiquidDistributor::LoadRecipe(const QString &recipe_name)
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

    // get settings
    RecipeUITableSetting tbl = GetRecipeUITableSetting();
    // clear table first.
    if (value_list.size() > 0)
    {
        ClearUITable();
        ClearRecipeRuntimeView();
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

        if ((type == TYPE_SAMPLING && category_ == LiquidDistributorCategory::SAMPLING) ||
                (type == TYPE_COLLECTION && category_ == LiquidDistributorCategory::COLLECTION)
                )
        {
            if (channel_a > 0)
            {
                if (pos_a > tbl.work_positions)
                {
                    assert(false);
                    continue;
                }
                FillUITableChannelInfo(
                            pos_a, channel_a, flowlimit_a, duration_a, cleanport_a);
                sampling_ui_items.at(pos_a - 1)->SetStatus(
                            SamplingUIItem::SamplingUIItemStatus::Waiting, channel_a);
            }
            if (channel_b > 0)
            {
                if (pos_b > tbl.work_positions)
                {
                    assert(false);
                    continue;
                }
                FillUITableChannelInfo(
                            pos_b, channel_b, flowlimit_b, duration_b, cleanport_b);
                sampling_ui_items.at(pos_b - 1)->SetStatus(
                            SamplingUIItem::SamplingUIItemStatus::Waiting, channel_b);
            }
        }
    }
    return true;
}

bool FormLiquidDistributor::DispatchRecipeTask(const QString& recipe_name)
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

    std::shared_ptr<std::vector<RecipeTaskEntity>> task =
            std::make_shared<std::vector<RecipeTaskEntity>>();
    for (auto& values : value_list)
    {
        RecipeTaskEntity entity;
        bool is_ok;
        int error_count = 0;
        entity.recipe_name = values->at(INDEX_RECIPE_NAME);
        entity.type = values->at(INDEX_TYPE).toInt(&is_ok);
        if (!is_ok) error_count++;
        entity.channel_a = values->at(INDEX_CHANNEL_A).toInt(&is_ok);
        if (!is_ok) error_count++;
        entity.channel_b = values->at(INDEX_CHANNEL_B).toInt(&is_ok);
        if (!is_ok) error_count++;
        entity.x = values->at(INDEX_X).toInt(&is_ok);
        if (!is_ok) error_count++;
        entity.y = values->at(INDEX_Y).toInt(&is_ok);
        if (!is_ok) error_count++;
        entity.pos_a = values->at(INDEX_POS_A).toInt(&is_ok);
        if (!is_ok) error_count++;
        entity.pos_b = values->at(INDEX_POS_B).toInt(&is_ok);
        if (!is_ok) error_count++;
        entity.flowlimit_a = values->at(INDEX_FLOWLIMIT_A).toInt(&is_ok);
        if (!is_ok) error_count++;
        entity.flowlimit_b = values->at(INDEX_FLOWLIMIT_B).toInt(&is_ok);
        if (!is_ok) error_count++;
        entity.cleanport_a = values->at(INDEX_CLEANPORT_A).toInt(&is_ok);
        if (!is_ok) error_count++;
        entity.cleanport_b = values->at(INDEX_CLEANPORT_B).toInt(&is_ok);
        if (!is_ok) error_count++;
        entity.duration_a = values->at(INDEX_DURATION_A).toInt(&is_ok);
        if (!is_ok) error_count++;
        entity.duration_b = values->at(INDEX_DURATION_B).toInt(&is_ok);
        if (!is_ok) error_count++;
        entity.run_a = values->at(INDEX_RUN_A).toInt(&is_ok);
        if (!is_ok) error_count++;
        entity.run_b = values->at(INDEX_RUN_B).toInt(&is_ok);
        if (!is_ok) error_count++;
        entity.control_code = values->at(INDEX_CONTROL_CODE).toUInt(&is_ok);
        if (error_count > 0)
        {
            // Do something to stop recipe runtime on plc.
            QMessageBox::critical(0, "加载配方失败", "数据格式错误", QMessageBox::Ignore);
            return false;
        }
        task->push_back(entity);
    }
    recipe_task_queue_.Put(task);
    return true;
}

void FormLiquidDistributor::RunRecipeWorker()
{
    QString error_msg;
    QSqlQuery query(db_);
    while (true)
    {
        std::shared_ptr<std::vector<RecipeTaskEntity>> task;
        recipe_task_queue_.Get(task);
        if (task == nullptr) // Improve for a robust SENTINEL
        {
            break; // Exit
        }

        for (auto& entity : *task)
        {
            // Adjust purge duration
            int run_a = entity.run_a;
            int run_b = entity.run_b;
            int duration_a = entity.duration_a;
            int duration_b = entity.duration_b;
            if (entity.type == TYPE_SAMPLING_PURGE)
            {
                duration_a *= 2;
                duration_b *= 2;
            }
            std::vector<QString> values = entity.ToValues();
            // Make plc command run
            bool ok = write_data_func_(this->objectName(), control_code_name_,
                                  QString::number(entity.control_code));
            assert(ok);
            if (!ok)
            {
                qCritical("write_data_func_ QFull error in RunRecipeWorker()");
            }
            // Write a runtime record
            if (db_ready_)
            {
                ok = WriteRecipeStepToDB(runtime_table_name_, table_columns_,
                                             values, query, error_msg);
                assert(ok);
                if (!ok)
                {
                    qCritical("WriteRecipeStepToDB failed in RunRecipeWorker(), %s",
                              error_msg.toLatin1().constData());
                }
            }

            // Wait sampling or do liquid level detection, send stop command to plc.
            std::future<qint64> check_a, check_b;
            if (run_a)
            {
                check_a = std::async(
                            &FormLiquidDistributor::SamplingStatusCheckByTime, this,
                            duration_a, StatusCheckGroup::A);
            }
            if (run_b)
            {
                check_b = std::async(
                            &FormLiquidDistributor::SamplingStatusCheckByTime, this,
                            duration_b, StatusCheckGroup::B);
            }

            // Write a runtime record
            if (run_a)
            {
                qint64 stoptime = check_a.get();
                values.at(INDEX_RUN_A) = QString::number(0);

                if (db_ready_)
                {
                    ok = WriteRecipeStepToDB(runtime_table_name_, table_columns_,
                                             values, query, error_msg, stoptime);
                    assert(ok);
                    if (!ok)
                    {
                        qCritical("WriteRecipeStepToDB failed in RunRecipeWorker(), %s",
                                  error_msg.toLatin1().constData());
                    }
                }
            }
            if (run_b)
            {
                qint64 stoptime = check_b.get();
                values.at(INDEX_RUN_B) = QString::number(0);

                if (db_ready_)
                {
                    ok = WriteRecipeStepToDB(runtime_table_name_, table_columns_,
                                             values, query, error_msg, stoptime);
                    assert(ok);
                    if (!ok)
                    {
                        qCritical("WriteRecipeStepToDB failed in RunRecipeWorker(), %s",
                                  error_msg.toLatin1().constData());
                    }
                }
            }
        }
    }
}

qint64 FormLiquidDistributor::SamplingStatusCheckByTime(
        int second, StatusCheckGroup status)
{
    std::this_thread::sleep_for(std::chrono::seconds(second));
    {
         std::lock_guard<std::mutex> lk(mut);
         if (status == StatusCheckGroup::A)
         {
             bool ok = write_data_func_(this->objectName(), channel_a_run_name_,
                         QString::number(0)); // no result check here
             assert(ok);
             if (!ok)
             {
                 qCritical("write_data_func_ QFull error in SamplingStatusCheckByTime()");
             }
         }
         else if (status == StatusCheckGroup::B)
         {
             bool ok = write_data_func_(this->objectName(), channel_b_run_name_,
                         QString::number(0)); // no result check here
             assert(ok);
             if (!ok)
             {
                 qCritical("write_data_func_ QFull error in SamplingStatusCheckByTime()");
             }
         }
    }
    return QDateTime::currentDateTime().toMSecsSinceEpoch();
}

void FormLiquidDistributor::FillTable(const std::list<std::vector<int>>& record_list)
{
    RecipeUITableSetting tbl = GetRecipeUITableSetting();

    // clear table first.
    if (record_list.size() > 0)
    {
        for (int row = 0; row < tbl.row_count; row++)
        {
            if (row == tbl.row_count / 2)
            {
                continue;
            }
            for (int group = 0; group < tbl.channel_groups; group++)
            {
                int start_col = group * tbl.line_items;
                if (group >= tbl.channel_groups / 2)
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
                combobox->setCurrentText("5s");
                combobox = static_cast<QComboBox*>(
                        ui->tableWidget->cellWidget(row, start_col + COL_SOLVENT_TYPE));
                combobox->setCurrentText("");
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
        int row = (pos - 1) / tbl.channel_groups;
        if (row > (MAX_SAMPLING_POS / tbl.channel_groups - 1) / 2)
        {
            row++; // jump seperator
        }
        int start_col = ((pos - 1) % tbl.channel_groups) * tbl.line_items;
        if (start_col > tbl.channel_groups * tbl.line_items / 2 - 1)
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
                        SamplingUIItem::SamplingUIItemStatus::Unsigned, 0);
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
                        SamplingUIItem::SamplingUIItemStatus::Waiting, 0);
        }
    }
}

void FormLiquidDistributor::InitUiState()
{
    ui->verticalLayout->installEventFilter(this);
}

void FormLiquidDistributor::paintEvent(QPaintEvent *e)
{
    if (category_ == LiquidDistributorCategory::SAMPLING)
    {
        for (int i = 0; i < 2; i++)
        {
            QImage img = QImage(static_cast<uchar*>(vframes_.at(i).data),
                                vframes_.at(i).cols, vframes_.at(i).rows, QImage::Format_BGR888);
            image_labels_.at(i)->setPixmap(QPixmap::fromImage(img));
            //image_label_0->resize(img.size());
            image_labels_.at(i)->show();
        }
    }
    FormCommon::paintEvent(e);
}

void FormLiquidDistributor::mouseDoubleClickEvent(QMouseEvent *event)
{
    std::vector<QString> recipe_names;
    QString error_msg;
    bool ok = ReadRecipeNames(recipe_names, error_msg);
    if (!ok)
    {
        QMessageBox::critical(0, "配方管理", error_msg, QMessageBox::Ignore);
        return;
    }
    DialogRecipeMgr dlg_recipe_mgr = DialogRecipeMgr(this, recipe_names);
    dlg_recipe_mgr.setWindowTitle("配方管理");
    dlg_recipe_mgr.move(event->globalPos() - QPoint(50, 50));
    int result = dlg_recipe_mgr.exec();
    if (result == QDialog::Accepted)
    {
        //
    }
    event->accept();
}

void FormLiquidDistributor::UpdateImage(int index)
{
    vcaps_.at(index) >> vframes_.at(index);
    if (vframes_.at(index).data != nullptr)
    {
        cv::Mat gray, edges;
        std::vector<std::vector<cv::Point>> contours;
        std::vector<cv::Vec4i> hierarchy;
        cv::cvtColor(vframes_.at(index), gray, cv::COLOR_BGR2GRAY);
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
            cv::drawContours(vframes_.at(index), contours, i, cv::Scalar(255, 0, 0), 3);
        }
        this->update();
    }
}

void FormLiquidDistributor::on_pushButton_clicked()
{
    QString recipe_name = "xb";
    bool ok = SaveRecipe(recipe_name);
    if (ok)
    {
        QMessageBox::information(0, "保存成功", recipe_name, QMessageBox::Ok);
    }
}

void FormLiquidDistributor::on_pushButton_2_clicked()
{
    //LoadLiquidSamplingProcedure("23,1,1,10,2;24,9,1,11,3;128,10,1,12,6");
    QString recipe_name = "xb_1655197617";
    bool ok = LoadRecipe(recipe_name);
    if (ok)
    {
        QMessageBox::information(0, "加载成功", recipe_name, QMessageBox::Ok);
    }
}

void FormLiquidDistributor::SelectChannelChanged(const QString& text)
{
    int row = ui->tableWidget->currentRow();
    int col = ui->tableWidget->currentColumn();
    auto tbl = GetRecipeUITableSetting();
    if (row >= tbl.row_count || col >= tbl.col_count)
    {
        assert(false);
        return;
    }
    if (row == tbl.row_count / 2)
    {
        return;
    }
    // detect channel changed.
    if (((col < tbl.col_count / 2) && (col % tbl.line_items == 1)) ||
            (col > tbl.col_count / 2 && ((col - 1) % tbl.line_items == 1)))

    {
//        QComboBox* combobox =
//                static_cast<QComboBox*>(ui->tableWidget->cellWidget(row, col));
        if (row > tbl.row_count / 2)
        {
            row--; // jump seperator
        }
        if (col > tbl.col_count / 2)
        {
            col--;  // jump seperator
        }
        int pos = row * tbl.channel_groups + col / tbl.line_items + 1; // base 1
        if (text == QString())
        {
            sampling_ui_items.at(pos - 1)->SetStatus(
                        SamplingUIItem::SamplingUIItemStatus::Unsigned, 0);
        }
        else
        {
            sampling_ui_items.at(pos - 1)->SetStatus(
                        SamplingUIItem::SamplingUIItemStatus::Waiting, text.toInt());
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
    if (value_list.empty())
    {
        error_message = "空数据";
        return false;
    }
    return true;
}

bool FormLiquidDistributor::WriteRecipeStepToDB(const QString& tablename, const std::vector<QString> columns,
        const std::vector<QString> values, QSqlQuery& query, QString& error_message, qint64 msecs)
{
    if (columns.empty() || values.empty() || columns.size() != values.size())
    {
        error_message = QString("输入数据格式错误");
        return false;
    }
    QString time = msecs > 0 ? QString::number(msecs) : QString::number(
                                   QDateTime::currentDateTime().toMSecsSinceEpoch());
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
    query_string.append(time);
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

bool FormLiquidDistributor::ReadRecipeNames(
        std::vector<QString>& recipe_names, QString &error_message)
{
    recipe_names.clear();
    error_message = "";

    // query recipes
    if (db_ready_)
    {
        QSqlQuery query(db_);
        QString error_message;
        QString query_string;
        query_string.append("select distinct \"recipe_name\", \"time\" from ");
        query_string.append(recipe_table_name_);
        if (LiquidDistributorCategory::SAMPLING == category_)
        {
            query_string.append(" where \"type\"='0' ");
        }
        else if (LiquidDistributorCategory::COLLECTION == category_)
        {
            query_string.append(" where \"type\"='2' ");
        }
        query_string.append("order by \"time\" desc;");
        bool ok = query.exec(query_string);
        if (!ok)
        {
            error_message = query.lastError().text();
            return false;
        }
        while (query.next())
        {
            const QSqlRecord& record = query.record();
            recipe_names.push_back(record.value("recipe_name").toString());
        }
        return true;
    }
    else
    {
        error_message = "连接数据库失败";
        return false;
    }
}


void FormLiquidDistributor::on_pushButton_3_clicked()
{
    QString recipe_name = "xb_1652838895";
    bool ok = DispatchRecipeTask(recipe_name);
    if (ok)
    {
        QMessageBox::information(0, "任务启动", recipe_name, QMessageBox::Ok);
    }
}

void FormLiquidDistributor::on_pushButton_4_clicked()
{
    QRCodeGenerator encoder;
    auto code = encoder.Encode("https://bing.com");
    this->ui->label_qr->setPixmap(encoder.ToQPixmap(code, "yashen"));
}
