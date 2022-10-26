#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMenu>
#include <QAction>
#include <cassert>
#include <algorithm>
#include <cmath>
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
    task_running_(false),
    dist_a_run_(false),
    dist_b_run_(false),
    timers_(2),
    image_params_(2),
    pressure_params_(2)
{
    ui->setupUi(this);
    InitUiState();

    // DB connection and table preparation
    PrepareDB(connection_path, object_name);

    if (category == LiquidDistributorCategory::SAMPLING)
    {
        InitVideoCaps();
        LoadImageParams();
    }
    else if (category == LiquidDistributorCategory::COLLECTION)
    {
        LoadPressureParams();
    }

    // For the recipe setting table
    InitRecipeSettingTable();
    // For the recipe runtime view
    InitRecipeRuntimeView();
    // For log window
    InitLogWindow();
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
    task_running_.store(false); // Stop current task
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

    if (event->type() == Ui::RefreshStateEvent::myType)
    {
        Ui::RefreshStateEvent* e = static_cast<Ui::RefreshStateEvent*>(event);
        if (e->Name().compare("label_dist_a_run", Qt::CaseInsensitive) == 0)
        {
            if (e->State() > 0)
            {
                dist_a_run_.store(true);
            }
            else
            {
                dist_a_run_.store(false);
            }
            task_run_cond_.notify_one();
        }
        else if (e->Name().compare("label_dist_b_run", Qt::CaseInsensitive) == 0)
        {
            if (e->State() > 0)
            {
                dist_b_run_.store(true);
            }
            else
            {
                dist_b_run_.store(false);
            }
            task_run_cond_.notify_one();
        }
    }

    return FormCommon::event(event);
}

bool FormLiquidDistributor::SaveRecipe(const QString &recipe_name)
{
    std::vector<std::vector<QString>> values;
    RecipeUITableSetting tbl = GetRecipeUITableSetting();

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
    // search by row, then by column
    QString r_name = recipe_name + "_" + QString::number(QDateTime::currentSecsSinceEpoch());
    for (int group = 0; group < tbl.channel_groups / 2; group++)
    {
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
    // End step.
    if (values.size() > 0)
    {
        values.push_back(std::vector<QString>());
        auto tail = values.rbegin();
        int type = (category_ == LiquidDistributorCategory::SAMPLING) ?
                    TYPE_SAMPLING : TYPE_COLLECTION;
        int channel_a = 1; // placehold
        int channel_b = 9; // placehold
        int x = 0;
        int y = 45; // to center position
        int pos_a = 0; // placehold
        int pos_b = 0; // placehold
        int flow_limit_a = 0; // placehold
        int flow_limit_b = 0; // placehold
        int solvent_type_a = 0; // placehold
        int solvent_type_b = 0; // placehold
        int sampling_time_a = 0; // placehold
        int sampling_time_b = 0; // placehold
        int run_a = 0; // KEY parameter
        int run_b = 0; // KEY parameter
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

bool FormLiquidDistributor::SaveParams(const QString& table_name,
                                            const std::vector<QString> columns,
                                            std::vector<std::vector<QString>> values)
{
    if (db_ready_)
    {
        QSqlQuery query(db_);
        QString error_msg;
        bool ok = UpdateParamsToDB(table_name, columns, QString("name"),
                                        values, query, error_msg);
        if (!ok)
        {
            QMessageBox::critical(0, "保存参数失败", error_msg, QMessageBox::Ignore);
        }
        return ok;
    }
    else
    {
        QMessageBox::critical(0, "保存参数失败", "数据库未连接", QMessageBox::Ignore);
        return false;
    }
}

bool FormLiquidDistributor::LoadImageParams()
{
    if (!db_ready_)
    {
        QMessageBox::critical(0, "加载图像参数失败", "数据库未连接", QMessageBox::Ignore);
        return false;
    }
    QString error_msg;
    std::vector<std::shared_ptr<std::vector<QString>>> value_list;
    QSqlQuery query(db_);
    std::vector<QString> load_names {"V0", "V1"};
    for (std::size_t i = 0; i < load_names.size(); i++)
    {
        bool ok = ReadRecordsFromDB(image_params_table_name_, QString("name"),
                                    load_names.at(i), image_param_columns_,
                                    query, value_list, error_msg);
        if (!ok)
        {
            QMessageBox::critical(0, "加载图像参数失败", error_msg, QMessageBox::Ignore);
            return false;
        }
        int index = 1; // jump 'name'
        for (auto& values : value_list)
        {
            bool is_ok;
            int error_count = 0;
            int roi_x = values->at(index++).toInt(&is_ok);
            if (!is_ok) error_count++;
            int roi_y = values->at(index++).toInt(&is_ok);
            if (!is_ok) error_count++;
            int roi_side = values->at(index++).toInt(&is_ok);
            if (!is_ok) error_count++;
            double lower_threshold = values->at(index++).toDouble(&is_ok);
            if (!is_ok) error_count++;
            double upper_threshold = values->at(index++).toDouble(&is_ok);
            if (!is_ok) error_count++;
            double direction = values->at(index++).toDouble(&is_ok);
            if (!is_ok) error_count++;
            int min_len = values->at(index++).toInt(&is_ok);
            if (!is_ok) error_count++;
            int min_count = values->at(index++).toInt(&is_ok);
            if (!is_ok) error_count++;
            double min_ratio = values->at(index++).toDouble(&is_ok);
            if (!is_ok) error_count++;
            if (error_count > 0)
            {
                QMessageBox::critical(0, "加载图像参数失败", "数据格式错误", QMessageBox::Ignore);
                return false;
            }
            else
            {
                std::lock_guard<std::shared_mutex> lk(shared_mut_);
                image_params_.at(i).roi_x = roi_x;
                image_params_.at(i).roi_y = roi_y;
                image_params_.at(i).roi_side = roi_side;
                image_params_.at(i).canny_lower_threshold = lower_threshold;
                image_params_.at(i).canny_upper_threshold = upper_threshold;
                image_params_.at(i).fit_line_degree = direction;
                image_params_.at(i).min_contour_len = min_len;
                image_params_.at(i).min_line_count = min_count;
                image_params_.at(i).min_ratio = min_ratio;
            }
        }
    }
    return true;
}

bool FormLiquidDistributor::LoadPressureParams()
{
    if (!db_ready_)
    {
        QMessageBox::critical(0, "加载压力参数失败", "数据库未连接", QMessageBox::Ignore);
        return false;
    }
    QString error_msg;
    std::vector<std::shared_ptr<std::vector<QString>>> value_list;
    QSqlQuery query(db_);
    std::vector<QString> load_names {"P0", "P1"};
    for (std::size_t i = 0; i < load_names.size(); i++)
    {
        bool ok = ReadRecordsFromDB(pressure_params_table_name_, QString("name"),
                                    load_names.at(i), pressure_param_columns_,
                                    query, value_list, error_msg);
        if (!ok)
        {
            QMessageBox::critical(0, "加载压力参数失败", error_msg, QMessageBox::Ignore);
            return false;
        }
        int index = 1; // jump 'name'
        for (auto& values : value_list)
        {
            bool is_ok;
            int error_count = 0;
            double lower_pressure = values->at(index++).toDouble(&is_ok);
            if (!is_ok) error_count++;
            double pressure_drop_ratio = values->at(index++).toDouble(&is_ok);
            if (!is_ok) error_count++;
            if (error_count > 0)
            {
                QMessageBox::critical(0, "加载压力参数失败", "数据格式错误", QMessageBox::Ignore);
                return false;
            }
            else
            {
                std::lock_guard<std::shared_mutex> lk(shared_mut_);
                pressure_params_.at(i).lower_pressure = lower_pressure;
                pressure_params_.at(i).pressure_drop_ratio = pressure_drop_ratio;
            }
        }
    }
    return true;
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
    int y_margin = 10;
    double y_base = y_margin;
    for (int y = 0; y < y_count; y++)
    {
        if (y >= y_section && y < 2 * y_section)
        {
            y_base = y_gap + y_margin;
        }
        else if (y >= 2 * y_section && y < 3 * y_section)
        {
            y_base = 2 * y_gap + y_margin;
        }
        else if (y >= 3 * y_section)
        {
            y_base = 3 * y_gap + y_margin;
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
            y_base = y_section * y_gap + y_margin;
        }
        else
        {
            y_base = (3 * y_section + 2) * y_gap + y_margin;
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

    auto scene = new QGraphicsScene(0, 0, x_count * (2 * x_gap), (y_count + 3) * y_gap + y_margin);
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
    ui->tableWidget->horizontalHeader()->setDefaultSectionSize(40);
    ui->tableWidget->verticalHeader()->setDefaultSectionSize(25);
    ui->tableWidget->setAlternatingRowColors(true);
    ui->tableWidget->resize(1200, 1100);
    QColor seperator_color = Qt::darkGray;

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

void FormLiquidDistributor::InitLogWindow()
{
    ui->textEdit_log->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->textEdit_log, &QTextEdit::customContextMenuRequested, this,
            &FormLiquidDistributor::LogWindowShowMenu);
    ui->textEdit_log->document()->setMaximumBlockCount(160);
}

void FormLiquidDistributor::LogWindowShowMenu()
{
    log_menu = std::make_shared<QMenu>("");
    auto action_clear = log_menu->addAction("清除");
    connect(action_clear, &QAction::triggered, this, [&] () { ui->textEdit_log->clear(); });
    log_menu->move(this->cursor().pos());
    log_menu->show();
}

void FormLiquidDistributor::EnableRecipeSettingTable(bool enable)
{
    RecipeUITableSetting tbl = GetRecipeUITableSetting();
    int row_sep = tbl.row_count / 2;
    int col_sep = tbl.col_count / 2;
    for (int row = 0; row < tbl.row_count; row++)
    {
        if (row == row_sep)
        {
            continue;
        }
        for (int grp = 0; grp < tbl.channel_groups; grp++)
        {
            for (int i = 1; i < tbl.line_items; i++)
            {
                int col = grp * tbl.line_items + i;
                if (col >= col_sep)
                {
                    col++;
                }
                ui->tableWidget->cellWidget(row, col)->setEnabled(enable);
            }
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

// Log to text window.
void FormLiquidDistributor::Log2Window(
        const QString& recipe_name, const QString& status)
{
    ui->textEdit_log->append(
                QDateTime::currentDateTime().toString("MM月dd日 hh:mm:ss ")
                + "[" + recipe_name + "]: " + status);
}

void FormLiquidDistributor::LogTaskStep(const RecipeTaskEntity& entity)
{
    QString info = (entity.type == 0 || entity.type == 2) ? "取液->" : "清洗->";
    info.append("位置a[");
    info.append(QString::number(entity.pos_a));
    info.append("] 通道[");
    info.append(QString::number(entity.channel_a));
    info.append("] ");
    info.append(QString::number(entity.run_a));
    info.append(", ");
    info.append("位置b[");
    info.append(QString::number(entity.pos_b));
    info.append("] 通道[");
    info.append(QString::number(entity.channel_b));
    info.append("] ");
    info.append(QString::number(entity.run_b));
    Log2Window(entity.recipe_name, info);
}

bool FormLiquidDistributor::LoadRecipe(const QString& recipe_name)
{
    if (!db_ready_)
    {
        QMessageBox::critical(0, "加载配方失败", "数据库未连接", QMessageBox::Ignore);
        return false;
    }
    QString error_msg;
    std::vector<std::shared_ptr<std::vector<QString>>> value_list;
    QSqlQuery query(db_);
    bool ok = ReadRecordsFromDB(recipe_table_name_, QString("recipe_name"),
                                recipe_name, table_columns_, query,
                                value_list, error_msg);
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
        int run_a = values->at(INDEX_RUN_A).toInt(&is_ok);
        if (!is_ok) error_count++;
        int run_b = values->at(INDEX_RUN_B).toInt(&is_ok);
        if (!is_ok) error_count++;

        if (error_count > 0)
        {
            ClearUITable();
            QMessageBox::critical(0, "加载配方失败", "数据格式错误", QMessageBox::Ignore);
            return false;
        }

        // Skip the move-only step.
        if (run_a == 0 && run_b == 0)
        {
            continue;
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
    loaded_recipe_name_ = recipe_name;
    return true;
}

bool FormLiquidDistributor::DeleteRecipe(const QString& recipe_name)
{
    QString error_msg;
    bool ok = DeleteRecipeFromDB(recipe_table_name_, recipe_name, error_msg);
    if (!ok)
    {
        QMessageBox::critical(0, "删除配方失败", error_msg, QMessageBox::Ignore);
        return false;
    }
    if (0 == loaded_recipe_name_.compare(recipe_name, Qt::CaseInsensitive))
    {
        ClearUITable();
        ClearRecipeRuntimeView();
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
    bool ok = ReadRecordsFromDB(recipe_table_name_, QString("recipe_name"),
                               recipe_name, table_columns_,
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

        // Clear
        ClearRecipeRuntimeView();
        // No edit
        EnableRecipeSettingTable(false);
        // Running
        QString recipe_name = task->size() > 0 ? task->at(0).recipe_name : "None";
        Log2Window(recipe_name, "任务启动");
        for (auto& entity : *task)
        {
            // Adjust purge duration
            bool volatile run_a = entity.run_a;
            bool volatile run_b = entity.run_b;
            int channel_a = entity.channel_a;
            int channel_b = entity.channel_b;
            int duration_a = entity.duration_a;
            int duration_b = entity.duration_b;
            if (entity.type == TYPE_SAMPLING_PURGE ||
                    entity.type == TYPE_COLLECTION_PURGE)
            {
                duration_a *= 2;
                duration_b *= 2;
            }
            std::vector<QString> values = entity.ToValue();
            // Send plc command run
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
            // Abort task execution if run_a == run_b == 0 step,
            // This kind of step always used for the end of the task:
            // the distributor stops at a specifed position.
            if (run_a == 0 && run_b == 0)
            {
                LogTaskStep(entity);
                break;
            }
            // Wait for PLC step run feedback
            std::unique_lock<std::mutex> lk(mut_);
            if (!task_running_.load() || !task_run_cond_.wait_for(lk, std::chrono::seconds(60), [&] {
                        return (run_a == dist_a_run_) && (run_b == dist_b_run_); })
                    )
            {
                qCritical("RunRecipeWorker() failed, %s", "No PLC step run feedback.");
                StopTakingLiquidCmd(StatusCheckGroup::A);
                StopTakingLiquidCmd(StatusCheckGroup::B);
                UpdateRuntimeView(entity, run_a, run_b, SamplingUIItem::SamplingUIItemStatus::Error,
                                   SamplingUIItem::SamplingUIItemStatus::Error);
                break; // no lk.unlock() needed.
//                if (!task_running_) // test code
//                {
//                    break;
//                }
            }
            lk.unlock();
            // Runtime view displaying
            UpdateRuntimeView(entity, run_a, run_b, SamplingUIItem::SamplingUIItemStatus::Sampling,
                               SamplingUIItem::SamplingUIItemStatus::Discharge);
            LogTaskStep(entity);

            // Wait sampling or do liquid level detection, send stop command to plc.
            std::future<qint64> check_a, check_b;
            if (run_a)
            {
                if (entity.type == TYPE_SAMPLING)
                {
                    check_a = std::async(
                                &FormLiquidDistributor::SamplingStatusCheckByImageDetection, this,
                                StatusCheckGroup::A, duration_a);

                }
                else if (entity.type == TYPE_COLLECTION)
                {
                    check_a = std::async(
                                &FormLiquidDistributor::SamplingStatusCheckByPressure, this,
                                StatusCheckGroup::A, channel_a, duration_a);
                }
                else
                {
                    check_a = std::async(
                                &FormLiquidDistributor::SamplingStatusCheckByTime, this,
                                StatusCheckGroup::A, duration_a);
                }
            }
            if (run_b)
            {
                if (entity.type == TYPE_SAMPLING)
                {
                    check_b = std::async(
                                &FormLiquidDistributor::SamplingStatusCheckByImageDetection, this,
                                StatusCheckGroup::B, duration_b);
                }
                else if (entity.type == TYPE_COLLECTION)
                {
                    check_b = std::async(
                                &FormLiquidDistributor::SamplingStatusCheckByPressure, this,
                                StatusCheckGroup::B, channel_b, duration_b);
                }
                else
                {
                    check_b = std::async(
                                &FormLiquidDistributor::SamplingStatusCheckByTime, this,
                                StatusCheckGroup::B, duration_b);
                }
            }

            // Write a step stopped record
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
            // Wait for PLC step stopped feedback
            lk.lock();
            if (!task_running_.load() || !task_run_cond_.wait_for(lk, std::chrono::seconds(20), [&] {
                        return (0 == dist_a_run_) && (0 == dist_b_run_); })
                    )
            {
                qCritical("RunRecipeWorker() failed, %s", "No PLC step stopped feedback.");
                UpdateRuntimeView(entity, run_a, run_b, SamplingUIItem::SamplingUIItemStatus::Error,
                                   SamplingUIItem::SamplingUIItemStatus::Error);
                break;
//                if (!task_running_) // test code
//                {
//                    break;
//                }
            }
            lk.unlock();
            // Runtime view displaying
            UpdateRuntimeView(entity, run_a, run_b, SamplingUIItem::SamplingUIItemStatus::Finished,
                               SamplingUIItem::SamplingUIItemStatus::Undischarge);
            // Sleep seconds between send dist_run_a = 0 and next loop dist_run_a = 1
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }
        // Stop
        bool expected = true;
        task_running_.compare_exchange_strong(expected, false);
        qInfo("recipe task [%s] finished", recipe_name.toLatin1().constData());
        Log2Window(recipe_name, "任务结束");
        EnableRecipeSettingTable(true);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void FormLiquidDistributor::UpdateRuntimeView(const RecipeTaskEntity& entity, bool run_a, bool run_b,
                                               SamplingUIItem::SamplingUIItemStatus sampling_status,
                                               SamplingUIItem::SamplingUIItemStatus purge_status)
{
    if (entity.type == TYPE_SAMPLING || entity.type == TYPE_COLLECTION)
    {
        if (run_a)
        {
            sampling_ui_items.at(entity.pos_a - 1)->SetStatus(sampling_status, entity.channel_a);
        }
        if (run_b)
        {
            sampling_ui_items.at(entity.pos_b - 1)->SetStatus(sampling_status, entity.channel_b);
        }
    }
    else if (entity.type == TYPE_SAMPLING_PURGE)
    {
        if (run_a)
        {
            int purge_index = (entity.pos_a > 64) ? (((entity.pos_a - 1) % 4) + 128 + 4) :
                                           (((entity.pos_a - 1) % 4) + 128);
            sampling_ui_items.at(purge_index)->SetStatus(purge_status, 0);
        }
        if (run_b)
        {
            int purge_index = (entity.pos_b > 64) ? (((entity.pos_b - 1) % 4) + 128 + 4) :
                                           (((entity.pos_b - 1) % 4) + 128);
            sampling_ui_items.at(purge_index)->SetStatus(purge_status, 0);
        }
    }
    else if (entity.type == TYPE_COLLECTION_PURGE)
    {
        if (run_a)
        {
            int purge_index = (entity.pos_a > 8) ? (((entity.pos_a - 1) % 2) + 16 + 2) :
                                           (((entity.pos_a - 1) % 2) + 16);
            sampling_ui_items.at(purge_index)->SetStatus(purge_status, 0);
        }
        if (run_b)
        {
            int purge_index = (entity.pos_b > 8) ? (((entity.pos_b - 1) % 2) + 16 + 2) :
                                           (((entity.pos_b - 1) % 2) + 16);
            sampling_ui_items.at(purge_index)->SetStatus(purge_status, 0);
        }
    }
}

bool FormLiquidDistributor::StopTakingLiquidCmd(StatusCheckGroup group)
{
    bool ok = false;
    if (StatusCheckGroup::A == group)
    {
        ok = write_data_func_(this->objectName(), channel_a_run_name_,
                    QString::number(0)); // no result check here
        assert(ok);
        if (!ok)
        {
            qCritical("write_data_func_ QFull error in StopTakingLiquidCmd()");
        }
    }
    else if (StatusCheckGroup::B == group)
    {
        ok = write_data_func_(this->objectName(), channel_b_run_name_,
                    QString::number(0)); // no result check here
        assert(ok);
        if (!ok)
        {
            qCritical("write_data_func_ QFull error in StopTakingLiquidCmd()");
        }
    }
    return ok;
}

qint64 FormLiquidDistributor::SamplingStatusCheckByTime(
        StatusCheckGroup group, int timeout_sec)
{
    auto timeout = std::chrono::system_clock::now() + std::chrono::seconds(timeout_sec);
    while (task_running_.load() && std::chrono::system_clock::now() < timeout)
    {
        std::this_thread::yield();
    }
    StopTakingLiquidCmd(group);
    return QDateTime::currentDateTime().toMSecsSinceEpoch();
}

qint64 FormLiquidDistributor::SamplingStatusCheckByImageDetection(
        StatusCheckGroup group, int timeout_sec)
{
    std::size_t index = (StatusCheckGroup::B == group) ? 1 : 0;
    auto timeout = std::chrono::system_clock::now() + std::chrono::seconds(timeout_sec);
    while (task_running_.load() && std::chrono::system_clock::now() < timeout)
    {
        if (DetectImage(index))
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    StopTakingLiquidCmd(group);
    return QDateTime::currentDateTime().toMSecsSinceEpoch();
}

qint64 FormLiquidDistributor::SamplingStatusCheckByPressure(
        StatusCheckGroup group, int channel, int timeout_sec)
{
    auto timeout = std::chrono::system_clock::now() + std::chrono::seconds(timeout_sec);
    float pressure_start = 0;
    float pressure_current = 10; // a large enough initial value
    float ratio = 0.5;  // 3barA * 0.5 = 1.5barA
    bool ok = ReadPressure(channel, pressure_start);
    while (task_running_.load() && std::chrono::system_clock::now() < timeout)
    {
        if (!ok || (pressure_current < 1.5) ||
                (pressure_current < pressure_start * ratio))
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        ok = ReadPressure(channel, pressure_current);
    }
    StopTakingLiquidCmd(group);
    return QDateTime::currentDateTime().toMSecsSinceEpoch();
}

bool FormLiquidDistributor::ReadPressure(int channel, float& pressure)
{
    pressure = 0;
    if (channel < 0 || channel > 16)
    {
        return false;
    }
    QString name("pressure_");
    name.append(QString::number(channel));
    QString value;
    Ui::ControlStatus status;
    UiInfo info;
    bool ok = read_data_func_(this->objectName(), name, value, status, info); // no result check here
    if (!ok || status != Ui::ControlStatus::OK)
    {
        return false;
    }
    pressure = value.toFloat(&ok);
    if (!ok)
    {
        return false;
    }
    return true;
}

bool FormLiquidDistributor::DetectImage(int index)
{
    bool ok = false;
    vcaps_.at(index) >> vframes_.at(index);
    if (vframes_.at(index).data != nullptr)
    {
        cv::Mat gray, roi, edges;
        std::vector<std::vector<cv::Point>> contours;
        std::vector<cv::Vec4i> hierarchy;
        cv::cvtColor(vframes_.at(index), gray, cv::COLOR_BGR2GRAY);

        // Prepare params
        std::shared_lock<std::shared_mutex> lk(shared_mut_);
        double canny_lower_threshold = image_params_.at(index).canny_lower_threshold;
        double canny_upper_threshold = image_params_.at(index).canny_upper_threshold;
        int canny_aperture_size = image_params_.at(index).canny_aperture_size;
        int roi_x = image_params_.at(index).roi_x;
        int roi_y = image_params_.at(index).roi_y;
        int roi_side = image_params_.at(index).roi_side;
        double fit_line_degree = image_params_.at(index).fit_line_degree;
        int min_contour_len = image_params_.at(index).min_contour_len;
        int min_line_count = image_params_.at(index).min_line_count;
        double min_ratio = image_params_.at(index).min_ratio;
        lk.unlock();

        // Detect
        if (min_ratio > 1.0)
        {
            min_ratio = 1.0;
        }
        if (roi_y < gray.size[0] && roi_x < gray.size[1] &&
                roi_y + roi_side < gray.size[0] && roi_x + roi_side < gray.size[1])
        {
            roi = gray(cv::Range(roi_y, roi_y + roi_side),
                       cv::Range(roi_x, roi_x + roi_side));
        }
        else
        {
            roi = gray;
        }

        cv::Canny(roi, edges, canny_lower_threshold, canny_upper_threshold,
                  canny_aperture_size, true/*L2gradient*/);
        cv::findContours(edges,
                         contours,
                         hierarchy,
                         cv::RETR_LIST,
                         cv::CHAIN_APPROX_SIMPLE);
        std::vector<int> lines_idx;
        double direction = std::tan(M_PI * (std::abs(fit_line_degree) / 180.0));
        int section_num = 4;
        std::vector<int> lines_in_section(section_num);
        for (std::size_t i = 0; i < contours.size(); i++)
        {
            // Omit short arcs
            int arc_len = cv::arcLength(contours.at(i), true);
            if (arc_len < min_contour_len)
            {
                continue;
            }
            // line structure: (vx, vy, x0, y0)
            cv::Vec4f line;
            cv::fitLine(contours.at(i), line, cv::DIST_L2, 0, 0.01, 0.01);
            if (std::abs(line[1] / line[0]) < direction)
            {
                lines_idx.push_back(i);
                // Map the fitted line to different ROI section by y0
                lines_in_section.at(int(line[3] / (roi_side / section_num))) += 1;
            }
        }
        // Draw liquid level
        if (lines_idx.size() > static_cast<std::size_t>(min_line_count))
        {
            std::vector<double> ratio_in_section(section_num);
            std::transform(lines_in_section.begin(), lines_in_section.end(),
                           ratio_in_section.begin(), [=](int i) {
                                    return double(i) / lines_idx.size(); } );
            for (int i = 0; i < section_num; i++)
            {
                ratio_in_section.at(i) = lines_in_section.at(i) / double(lines_idx.size());
            }
            auto max_iter = std::max_element(ratio_in_section.begin(),
                                                ratio_in_section.end());
            std::size_t max_index = std::distance(ratio_in_section.begin(),
                                                  max_iter);
            // Found and draw the liquid level.
            if (*max_iter > min_ratio)
            {
                int level_y = static_cast<int>(
                            roi_y + max_index * roi_side / section_num + roi_side / section_num / 2);
                cv::line(vframes_.at(index), cv::Point(roi_x, level_y),
                         cv::Point(roi_x + roi_side, level_y), cv::Scalar(0, 255, 0), 2);
                if (max_index == 0)
                {
                    ok = true; // Upmost level
                }
            }
        }
        // Draw contours? time cost.
        for (auto i : lines_idx)
        {
            for (auto& p : contours[i])
            {
                p.x += roi_x;
                p.y += roi_y;
            }
            cv::drawContours(vframes_.at(index), contours, i, cv::Scalar(255, 0, 0), 2);
        }
        // Draw ROI
        std::vector<std::vector<cv::Point>> roi_contours = {{cv::Point(roi_x, roi_y),
                                                             cv::Point(roi_x + roi_side, roi_y),
                                                             cv::Point(roi_x + roi_side, roi_y + roi_side),
                                                             cv::Point(roi_x, roi_y + roi_side)}};
        cv::drawContours(vframes_.at(index), roi_contours, 0, cv::Scalar(0, 255, 255), 2);
        this->update();
    }
    return ok;
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

void FormLiquidDistributor::LoadManagementWindow()
{
    std::vector<QString> recipe_names;
    QString error_msg;
    bool ok = ReadRecipeNamesFromDB(recipe_names, error_msg);
    if (!ok)
    {
        QMessageBox::critical(0, "配方管理", error_msg, QMessageBox::Ignore);
        return;
    }
    // Get image parameters.
    std::vector<std::vector<double>> call_params;
    DialogRecipeMgr::RecipeCategory dlg_category;
    std::shared_lock<std::shared_mutex> lk(shared_mut_);
    if (LiquidDistributorCategory::SAMPLING == category_)
    {
        call_params.push_back(image_params_.at(0).toDoubleValue());
        call_params.push_back(image_params_.at(1).toDoubleValue());
        dlg_category = DialogRecipeMgr::RecipeCategory::SAMPLING;
    }
    else if (LiquidDistributorCategory::COLLECTION == category_)
    {
        call_params.push_back(pressure_params_.at(0).toDoubleValue());
        call_params.push_back(pressure_params_.at(1).toDoubleValue());
        dlg_category = DialogRecipeMgr::RecipeCategory::COLLECTION;
    }
    lk.unlock();
    // Call the dialog.
    DialogRecipeMgr dlg_recipe_mgr =
            DialogRecipeMgr(this, recipe_names, call_params, dlg_category);
    QString title = "配方管理 -> " + loaded_recipe_name_;
    dlg_recipe_mgr.setWindowTitle(title);
    dlg_recipe_mgr.exec();
    // Dialog closed.
    QString recipe_name = dlg_recipe_mgr.GetActingRecipeName();
    DialogRecipeMgr::RecipeAction act = dlg_recipe_mgr.GetRecipeAction();
    if ((DialogRecipeMgr::RecipeAction::NONE != act &&
          DialogRecipeMgr::RecipeAction::STOP != act)  && task_running_.load())
    {
        QMessageBox::critical(0, "任务正在运行",
                              loaded_recipe_name_, QMessageBox::Ignore);
    }
    else
    {
        if (DialogRecipeMgr::RecipeAction::LOAD == act)
        {
            bool ok = LoadRecipe(recipe_name);
            if (ok)
            {
                Log2Window(recipe_name, "加载成功");
                QMessageBox::information(0, "加载成功", recipe_name, QMessageBox::Ok);
            }
        }
        else if (DialogRecipeMgr::RecipeAction::SAVE == act)
        {
            bool ok = SaveRecipe(recipe_name);
            if (ok)
            {
                Log2Window(recipe_name, "保存成功");
                QMessageBox::information(0, "保存成功", recipe_name, QMessageBox::Ok);
            }
        }
        else if (DialogRecipeMgr::RecipeAction::DELETE == act)
        {
            auto ret = QMessageBox::question(0, "删除配方", recipe_name, QMessageBox::Yes | QMessageBox::No);
            if (ret == QMessageBox::Yes)
            {
                bool ok = DeleteRecipe(recipe_name);
                if (ok)
                {
                    Log2Window(recipe_name, "删除成功");
                    QMessageBox::information(0, "删除成功", recipe_name, QMessageBox::Ok);
                }
            }
        }
        else if (DialogRecipeMgr::RecipeAction::RUN == act)
        {
            if (recipe_name.compare(loaded_recipe_name_, Qt::CaseInsensitive) == 0 &&
                    !task_running_.load())
            {
                bool ok = DispatchRecipeTask(recipe_name);
                if (ok)
                {
                    task_running_.store(true);
                    QMessageBox::information(0, "任务启动", recipe_name, QMessageBox::Ok);
                }
            }
            else
            {
                QMessageBox::critical(0, "任务未加载", recipe_name, QMessageBox::Ignore);
            }
        }
        else if (DialogRecipeMgr::RecipeAction::STOP == act)
        {
            auto ret = QMessageBox::question(0, "结束任务", recipe_name, QMessageBox::Yes | QMessageBox::No);
            if (ret == QMessageBox::Yes &&
                    QString::compare(recipe_name, loaded_recipe_name_, Qt::CaseInsensitive) == 0)
            {
                if (task_running_.load())
                {
                    task_running_.store(false);
                    QMessageBox::information(0, "结束任务。。。", recipe_name, QMessageBox::Ok);
                }
                else
                {
                    QMessageBox::critical(0, "任务未运行", recipe_name, QMessageBox::Ignore);
                }
            }
        }
        else if (DialogRecipeMgr::RecipeAction::UPDATE_PARAMS == act)
        {
            std::vector<std::vector<double>> params = dlg_recipe_mgr.GetParams();
            {
                std::lock_guard<std::shared_mutex> lk(shared_mut_);
                if (LiquidDistributorCategory::SAMPLING == category_)
                {
                    for (std::size_t i = 0; i < params.size(); i++)
                    {
                        image_params_.at(i).roi_x = params.at(i).at(0);
                        image_params_.at(i).roi_y = params.at(i).at(1);
                        image_params_.at(i).roi_side = params.at(i).at(2);
                        image_params_.at(i).canny_lower_threshold = params.at(i).at(3);
                        image_params_.at(i).canny_upper_threshold = params.at(i).at(4);
                        image_params_.at(i).fit_line_degree = params.at(i).at(5);
                        image_params_.at(i).min_contour_len = params.at(i).at(6);
                        image_params_.at(i).min_line_count = params.at(i).at(7);
                        image_params_.at(i).min_ratio = params.at(i).at(8);
                    }
                }
                else if (LiquidDistributorCategory::COLLECTION == category_)
                {
                    for (std::size_t i = 0; i < params.size(); i++)
                    {
                        pressure_params_.at(i).lower_pressure = params.at(i).at(0);
                        pressure_params_.at(i).pressure_drop_ratio = params.at(i).at(1);
                    }
                }
            }
            if (LiquidDistributorCategory::SAMPLING == category_)
            {
                std::vector<std::vector<QString>> values({image_params_.at(0).toValue(0),
                                                         image_params_.at(1).toValue(1)});
                SaveParams(image_params_table_name_, image_param_columns_, values);
            }
            else if (LiquidDistributorCategory::COLLECTION == category_)
            {
                std::vector<std::vector<QString>> values({pressure_params_.at(0).toValue(0),
                                                         pressure_params_.at(1).toValue(1)});
                SaveParams(pressure_params_table_name_, pressure_param_columns_, values);
            }
        }
    }
}

void FormLiquidDistributor::UpdateImage(int index)
{
    if (!task_running_.load())
    {
        vcaps_.at(index) >> vframes_.at(index);
        if (vframes_.at(index).data != nullptr)
        {
            cv::Mat gray, roi, edges;
            std::vector<std::vector<cv::Point>> contours;
            std::vector<cv::Vec4i> hierarchy;
            cv::cvtColor(vframes_.at(index), gray, cv::COLOR_BGR2GRAY);

            // Prepare params
            std::shared_lock<std::shared_mutex> lk(shared_mut_);
            double canny_lower_threshold = image_params_.at(index).canny_lower_threshold;
            double canny_upper_threshold = image_params_.at(index).canny_upper_threshold;
            int canny_aperture_size = image_params_.at(index).canny_aperture_size;
            int roi_x = image_params_.at(index).roi_x;
            int roi_y = image_params_.at(index).roi_y;
            int roi_side = image_params_.at(index).roi_side;
            double fit_line_degree = image_params_.at(index).fit_line_degree;
            int min_contour_len = image_params_.at(index).min_contour_len;
            int min_line_count = image_params_.at(index).min_line_count;
            double min_ratio = image_params_.at(index).min_ratio;
            lk.unlock();

            // Detect
            if (min_ratio > 1.0)
            {
                min_ratio = 1.0;
            }
            if (roi_y < gray.size[0] && roi_x < gray.size[1] &&
                    roi_y + roi_side < gray.size[0] && roi_x + roi_side < gray.size[1])
            {
                roi = gray(cv::Range(roi_y, roi_y + roi_side),
                           cv::Range(roi_x, roi_x + roi_side));
            }
            else
            {
                roi = gray;
            }

            cv::Canny(roi, edges, canny_lower_threshold, canny_upper_threshold,
                      canny_aperture_size, true/*L2gradient*/);
            cv::findContours(edges,
                             contours,
                             hierarchy,
                             cv::RETR_LIST,
                             cv::CHAIN_APPROX_SIMPLE);
            std::vector<int> lines_idx;
            double direction = std::tan(M_PI * (std::abs(fit_line_degree) / 180.0));
            int section_num = 4;
            std::vector<int> lines_in_section(section_num);
            for (std::size_t i = 0; i < contours.size(); i++)
            {
                // Omit short arcs
                int arc_len = cv::arcLength(contours.at(i), false);
                if (arc_len < min_contour_len)
                {
                    continue;
                }
                // line structure: (vx, vy, x0, y0)
                cv::Vec4f line;
                cv::fitLine(contours.at(i), line, cv::DIST_L2, 0, 0.01, 0.01);
                if (std::abs(line[1] / line[0]) < direction)
                {
                    lines_idx.push_back(i);
                    // Map the fitted line to different ROI section by y0
                    lines_in_section.at(int(line[3] / (roi_side / section_num))) += 1;
                }
            }
            // Draw liquid level
            if (lines_idx.size() > static_cast<std::size_t>(min_line_count))
            {
                std::vector<double> ratio_in_section(section_num);
                std::transform(lines_in_section.begin(), lines_in_section.end(),
                               ratio_in_section.begin(), [=](int i) {
                    return double(i) / lines_idx.size(); } );
                for (int i = 0; i < section_num; i++)
                {
                    ratio_in_section.at(i) = lines_in_section.at(i) / double(lines_idx.size());
                }
                auto max_iter = std::max_element(ratio_in_section.begin(),
                                                 ratio_in_section.end());
                std::size_t max_index = std::distance(ratio_in_section.begin(),
                                                      max_iter);
                if (*max_iter > min_ratio)
                {
                    int level_y = static_cast<int>(
                                roi_y + max_index * roi_side / section_num + roi_side / section_num / 2);
                    cv::line(vframes_.at(index), cv::Point(roi_x, level_y),
                             cv::Point(roi_x + roi_side, level_y), cv::Scalar(0, 255, 0), 2);
                    //                if (max_index == 0)
                    //                {
                    //                    found_level = true;
                    //                }
                }
            }
            // Draw contours, time cost!!!
            for (auto i : lines_idx)
            {
                for (auto& p : contours[i])
                {
                    p.x += roi_x;
                    p.y += roi_y;
                }
                cv::drawContours(vframes_.at(index), contours, i, cv::Scalar(255, 0, 0), 2);
            }
            // Draw ROI
            std::vector<std::vector<cv::Point>> roi_contours = {{cv::Point(roi_x, roi_y),
                                                                 cv::Point(roi_x + roi_side, roi_y),
                                                                 cv::Point(roi_x + roi_side, roi_y + roi_side),
                                                                 cv::Point(roi_x, roi_y + roi_side)}};
            cv::drawContours(vframes_.at(index), roi_contours, 0, cv::Scalar(0, 255, 255), 2);

            this->update();
        }
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
        image_param_columns_ = {"name", "roi_x", "roi_y", "roi_side", "lower_threshold",
                         "upper_threshold", "direction", "min_len", "min_count", "min_ratio"};
        pressure_param_columns_ = {"name", "lower_pressure", "pressure_drop_ratio"};
        QString error_message_1;
        QString error_message_2;
        QString error_message_3;
        QString error_message_4;
        std::vector<QString> default_primary_keys;
        std::vector<QString> primary_keys {"name"};
        bool ok1 = CreateDBTable(db_, recipe_table_name_, table_columns_, default_primary_keys, error_message_1);
        bool ok2 = CreateDBTable(db_, runtime_table_name_, table_columns_, default_primary_keys, error_message_2);
        bool ok3 = CreateDBTable(db_, image_params_table_name_, image_param_columns_, primary_keys, error_message_3);
        bool ok4 = CreateDBTable(db_, pressure_params_table_name_, pressure_param_columns_, primary_keys, error_message_4);
        if (!ok1 || !ok2 || !ok3 || !ok4)
        {
            QMessageBox::critical(0, "创建数据表失败",
                                  error_message_1 + "\n" + error_message_2 + "\n" + error_message_3 + "\n" + error_message_4, QMessageBox::Ignore);
        }
        else
        {
            db_ready_ = true;
        }
    }
}

bool FormLiquidDistributor::CreateDBTable(const QSqlDatabase& db,
                                          const QString& table_name,
                                          const std::vector<QString>& columns,
                                          const std::vector<QString>& primary_keys,
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
    query_string.append(" (");
    for (auto& column : columns)
    {
        query_string.append("\"");
        query_string.append(column);
        query_string.append("\" TEXT,");
    }
    if (!primary_keys.empty())
    {
        query_string.append("PRIMARY KEY(");
        for (auto& key : primary_keys)
        {
            query_string.append("\"");
            query_string.append(key);
            query_string.append("\"");
            query_string.append(", ");
        }
        query_string.remove(query_string.length() - 2, 2);
        query_string.append("), ");
    }
    else
    {
        query_string.append("id SERIAL PRIMARY KEY, ");
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

bool FormLiquidDistributor::ReadRecordsFromDB(const QString& tablename,
        const QString& where_key, const QString& where_value,
        const std::vector<QString> columns, QSqlQuery& query,
        std::vector<std::shared_ptr<std::vector<QString>>>& value_list,
        QString& error_message)
{
    value_list.clear();

    QString query_string("SELECT ");
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
    query_string.append(" FROM ");
    query_string.append(tablename);
    query_string.append(" WHERE \"");
    query_string.append(where_key);
    query_string.append("\" = '");
    query_string.append(where_value);
    query_string.append("';");
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
    QString msecs = QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());
    QString query_string;
    for (auto& values : value_list)
    {
        query_string.append("INSERT INTO ");
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
        query_string.append(msecs);
        query_string.append(");");
    }
    bool ok = query.exec(query_string);
    if (!ok)
    {
        error_message = query.lastError().text();
    }
    return ok;
}

bool FormLiquidDistributor::UpdateParamsToDB(
        const QString& tablename, const std::vector<QString> columns,
        const QString& primary_key, const std::vector<std::vector<QString> > value_list,
        QSqlQuery &query, QString &error_message)
{
    if (value_list.size() == 0 || columns.size() != value_list.at(0).size())
    {
        error_message = QString("参数为空");
        return false;
    }
    QString query_string;
    QString msecs = QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());
    // Postgresql timestamp with time zone: 2022-10-05 11:29:50+08 plusing +08 zone automatically.
    QString dt = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    for (auto& values : value_list)
    {
        query_string.append("INSERT INTO ");
        query_string.append(tablename);
        query_string.append(" (");
        for (auto& column : columns)
        {
            query_string.append("\"");
            query_string.append(column);
            query_string.append("\", ");
        }
        query_string.append("\"time\", \"createtime\") VALUES (");
        for (auto& value : values)
        {
            query_string.append("'");
            query_string.append(value);
            query_string.append("', ");
        }
        query_string.append(msecs);
        query_string.append(", '");
        query_string.append(dt);
        query_string.append("') ON CONFLICT (\"");
        query_string.append(primary_key);
        query_string.append("\") DO UPDATE SET ");
        for (std::size_t i = 1/*jump 'name'*/; i < columns.size(); i++)
        {
            query_string.append("\"");
            query_string.append(columns.at(i));
            query_string.append("\" = '");
            query_string.append(values.at(i));
            query_string.append("', ");
        }
        query_string.append("\"time\" = ");
        query_string.append(msecs);
        query_string.append(", \"createtime\" = '");
        query_string.append(dt);
        query_string.append("'");
        query_string.append(";");
    }
    bool ok = query.exec(query_string);
    if (!ok)
    {
        error_message = query.lastError().text();
    }
    return ok;
}

bool FormLiquidDistributor::ReadRecipeNamesFromDB(
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
        query_string.append("SELECT DISTINCT \"recipe_name\", \"time\" FROM ");
        query_string.append(recipe_table_name_);
        if (LiquidDistributorCategory::SAMPLING == category_)
        {
            query_string.append(" WHERE \"type\"='0' ");
        }
        else if (LiquidDistributorCategory::COLLECTION == category_)
        {
            query_string.append(" WHERE \"type\"='2' ");
        }
        query_string.append("ORDER BY \"time\" DESC;");
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

bool FormLiquidDistributor::DeleteRecipeFromDB(
        const QString& tablename, const QString& recipe_name, QString& error_message)
{
    error_message = "";

    // query recipes
    if (db_ready_)
    {
        QSqlQuery query(db_);
        QString query_string;
        query_string.append("DELETE FROM ");
        query_string.append(tablename);
        query_string.append(" WHERE \"recipe_name\"='");
        query_string.append(recipe_name);
        query_string.append("';");
        bool ok = query.exec(query_string);
        if (!ok)
        {
            error_message = query.lastError().text();
            return false;
        }
        return true;
    }
    else
    {
        error_message = "连接数据库失败";
        return false;
    }
}

void FormLiquidDistributor::on_pushButton_4_clicked()
{
    QRCodeGenerator encoder;
    auto code = encoder.Encode("https://bing.com");
    this->ui->label_qr->setPixmap(encoder.ToQPixmap(code, "yashen"));
}

void FormLiquidDistributor::on_pushButtonManage_clicked()
{
    LoadManagementWindow();
}
