#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <cassert>
#include "form_liquiddistributor.h"
#include "ui_form_liquiddistributor.h"

FormLiquidDistributor::FormLiquidDistributor(QWidget *parent,
                                             const QString& object_name,
                                             const QString& display_name,
                                             LiquidDistributorGroup group) :
    FormCommon(parent, object_name, display_name),
    ui(new Ui::FormLiquidDistributor),
    group_(group)
{
    ui->setupUi(this);
    InitUiState();

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
            item->setPos(x * x_gap + radius, y_base + y * y_gap + radius);
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
            item->setPos(x * x_gap + radius, y_base + radius);
            sampling_ui_items.push_back(item);
        }
    }

    auto scene = new QGraphicsScene(0, 0, 300, 1200);
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
            if (row < ROW_COUNT / 2)
            {
                row_positon++;
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
            QStringList solvent_type({"L2", "L3", "L4", "L6", "L7", "L8"});

            if (line == COL_POS)
            {
                // Position
                auto pos = QString("#") + QString::number((ROW_COUNT - 1 - row_positon) * LINE_GROUPS + line_group + 1);
                ui->tableWidget->setItem(row, col, new QTableWidgetItem(pos));
                ui->tableWidget->item(row, col)->setFlags(Qt::NoItemFlags);
            }
            else if (line == COL_CHANNEL)
            {
                // Channel
                QComboBox *combobox = new QComboBox();
                if (line_group % 2 == 0)
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
    // Reversed search
    for (int row = ROW_COUNT - 1; row >= 0; row--)
    {
        if (row == ROW_COUNT / 2)
        {
            continue;
        }

        int reversed_row = (ROW_COUNT - 1) - row;
        if (row < ROW_COUNT / 2)
        {
            reversed_row--;
        }
        for (int group = 0; group < LINE_GROUPS; group++)
        {
            int start_col = group * LINE_ITEMS;
            if (group > 1)
            {
                start_col += 1; // Jump seperator
            }
            int pos = reversed_row * LINE_GROUPS + group + 1;
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
        // Reverse
        row = (MAX_SAMPLING_POS / LINE_GROUPS - 1) - row;
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

void FormLiquidDistributor::InitUiState()
{
    ui->verticalLayout->installEventFilter(this);
}

void FormLiquidDistributor::on_pushButton_clicked()
{
    SaveLiquidSamplingProcedure();
}

void FormLiquidDistributor::on_pushButton_2_clicked()
{
    LoadLiquidSamplingProcedure("23,1,1,10,2;24,9,1,11,3;128,10,1,12,6");
}
