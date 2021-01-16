#include <QPushButton>
#include "form_motorcontrol.h"
#include "ui_form_motorcontrol.h"


FormMotorControl::FormMotorControl(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormMotorControl)
{
    ui->setupUi(this);

    int row_count = 16;
    int col_count = 11;
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
    labels.push_back("移动");
    labels.push_back("停止");
    ui->tableWidget->setColumnCount(col_count);
    ui->tableWidget->setRowCount(row_count);
    ui->tableWidget->setHorizontalHeaderLabels(labels);
    //QTableWidgetItem* head_item = new QTableWidgetItem(QString("Last"),QTableWidgetItem::Type);
    //ui->tableWidget->setHorizontalHeaderItem(0, head_item);

    //ui.qtablewidget->setItem(1, 0, new QTableWidgetItem(str));
    //QString str =ui.qtablewidget->item(0, 0)->data(Qt::DisplayRole).toString();

    // Name
    std::vector<QString> names;
    names.push_back("1#PO缸");
    names.push_back("1#EO缸");
    names.push_back("2#PO缸");
    names.push_back("2#EO缸");
    names.push_back("3#PO缸");
    names.push_back("3#EO缸");
    names.push_back("4#PO缸");
    names.push_back("4#EO缸");
    names.push_back("5#PO缸");
    names.push_back("5#EO缸");
    names.push_back("6#PO缸");
    names.push_back("6#EO缸");
    names.push_back("7#PO缸");
    names.push_back("7#EO缸");
    names.push_back("8#PO缸");
    names.push_back("8#EO缸");
    for (int i = 0; i < row_count; i++)
    {
        ui->tableWidget->setItem(i, 0, new QTableWidgetItem(names.at(i)));
        ui->tableWidget->item(i, 0)->setFlags(Qt::NoItemFlags);
        ui->tableWidget->setItem(i, 1, new QTableWidgetItem("n/a"));
        ui->tableWidget->item(i, 1)->setFlags(Qt::NoItemFlags);
        ui->tableWidget->setItem(i, 4, new QTableWidgetItem("n/a"));
        ui->tableWidget->item(i, 4)->setFlags(Qt::NoItemFlags);
        ui->tableWidget->setItem(i, 6, new QTableWidgetItem("n/a"));
        ui->tableWidget->item(i, 6)->setFlags(Qt::NoItemFlags);
        ui->tableWidget->setItem(i, 7, new QTableWidgetItem("n/a"));
        ui->tableWidget->item(i, 7)->setFlags(Qt::NoItemFlags);
        ui->tableWidget->setItem(i, 8, new QTableWidgetItem("n/a"));
        ui->tableWidget->item(i, 8)->setFlags(Qt::NoItemFlags);
    }

    // Button
    for (int i = 0; i < row_count; i++)
    {
        // Button
        QPushButton *button = new QPushButton();
        button->setText("Enable");
        button->setCursor(Qt::PointingHandCursor);
        connect(button, &QPushButton::clicked, this, &FormMotorControl::OnBtnClicked);
        ui->tableWidget->setCellWidget(i, 2, button);

        button = new QPushButton();
        button->setText("Disable");
        button->setCursor(Qt::PointingHandCursor);
        connect(button, &QPushButton::clicked, this, &FormMotorControl::OnBtnClicked);
        ui->tableWidget->setCellWidget(i, 3, button);

        button = new QPushButton();
        button->setText("Clear");
        button->setCursor(Qt::PointingHandCursor);
        connect(button, &QPushButton::clicked, this, &FormMotorControl::OnBtnClicked);
        ui->tableWidget->setCellWidget(i, 5, button);

        button = new QPushButton();
        button->setText("<->");
        button->setCursor(Qt::PointingHandCursor);
        connect(button, &QPushButton::clicked, this, &FormMotorControl::OnBtnClicked);
        ui->tableWidget->setCellWidget(i, 9, button);

        button = new QPushButton();
        button->setText("Stop");
        button->setCursor(Qt::PointingHandCursor);
        connect(button, &QPushButton::clicked, this, &FormMotorControl::OnBtnClicked);
        ui->tableWidget->setCellWidget(i, 10, button);
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
    QModelIndex idx = ui->tableWidget->indexAt(
                QPoint(senderObj->frameGeometry().x(), senderObj->frameGeometry().y()));
    int row = idx.row();
    int col = idx.column();
}
