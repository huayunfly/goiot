#include <QCheckBox>
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

    QStringList labels;

    const int LINE_ITEMS = 5;
    const int LINE_GROUPS = 4;
    for (int i = 0; i < 5; i++)
    {
        if (i == 2)
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
                ui->tableWidget->setItem(row, col, new QTableWidgetItem("ch"));
                ui->tableWidget->item(row, col)->setTextAlignment(Qt::AlignCenter);
            }
            else if (line == COL_FLOW_LIMIT)
            {
                // Flow limit?
                QCheckBox *checkbox = new QCheckBox();
                ui->tableWidget->setCellWidget(row, col, checkbox);
            }
            else if (line == COL_TIME)
            {
                // Sampling time in s
                ui->tableWidget->setItem(row, col, new QTableWidgetItem("1s"));
                ui->tableWidget->item(row, col)->setTextAlignment(Qt::AlignCenter);
            }
            else if (line == COL_PURGE)
            {
                // Purge?
                ui->tableWidget->setItem(row, col, new QTableWidgetItem("p"));
                ui->tableWidget->item(row, col)->setTextAlignment(Qt::AlignCenter);
            }
        }
    }
}

FormLiquidDistributor::~FormLiquidDistributor()
{
    delete ui;
}
