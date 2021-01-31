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

    for (int i = 0; i < 4; i++)
    {
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

    const int LINE_COUNT = 4; // 4 lines group for liquid sampling
    const int LINE_ITEMS = 5;
    for (int row = 0; row < ROW_COUNT; row++)
    {
        for (int line = 0; line < LINE_COUNT; line++)
        {
            auto pos = QString("#") + QString::number((ROW_COUNT - 1 - row) * LINE_COUNT + (line  + 1));
            ui->tableWidget->setItem(row, line * LINE_ITEMS + COL_POS, new QTableWidgetItem(pos));
            ui->tableWidget->item(row, line * LINE_ITEMS + COL_POS)->setFlags(Qt::NoItemFlags);

            ui->tableWidget->setItem(row, line * LINE_ITEMS + COL_CHANNEL, new QTableWidgetItem("a"));
            ui->tableWidget->item(row, line * LINE_ITEMS + COL_CHANNEL)->setTextAlignment(Qt::AlignCenter);

            QCheckBox *checkbox = new QCheckBox();
            ui->tableWidget->setCellWidget(row, line * LINE_ITEMS + COL_FLOW_LIMIT, checkbox);

            ui->tableWidget->setItem(row, line * LINE_ITEMS + COL_TIME, new QTableWidgetItem("1s"));
            ui->tableWidget->item(row, line * LINE_ITEMS + COL_TIME)->setTextAlignment(Qt::AlignCenter);

            ui->tableWidget->setItem(row, line * LINE_ITEMS + COL_PURGE, new QTableWidgetItem("d"));
            ui->tableWidget->item(row, line * LINE_ITEMS + COL_PURGE)->setTextAlignment(Qt::AlignCenter);
        }
    }
}

FormLiquidDistributor::~FormLiquidDistributor()
{
    delete ui;
}
