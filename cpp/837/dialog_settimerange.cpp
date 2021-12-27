#include "dialog_settimerange.h"
#include "ui_dialog_settimerange.h"

DialogSetTimeRange::DialogSetTimeRange(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSetTimeRange)
{
    ui->setupUi(this);

    // combobox time range
    ui->comboBox_range->addItem("过去1天");
    ui->comboBox_range->addItem("过去3天");
    ui->comboBox_range->addItem("过去7天");
    ui->comboBox_range->addItem("过去30天");

    // datatime
    QDateTime now = QDateTime::currentDateTime();
    QDateTime last = now.addSecs(-12 * 3600);
    ui->dateTimeEdit_max->setDateTime(now);
    ui->dateTimeEdit_min->setDateTime(last); // last 12 hours

    // Initializes min, max time
    max_time_ = now.toSecsSinceEpoch();
    min_time_ = last.toSecsSinceEpoch();
}

DialogSetTimeRange::~DialogSetTimeRange()
{
    delete ui;
}

double DialogSetTimeRange::Min()
{
    return min_time_;
}

double DialogSetTimeRange::Max()
{
    return max_time_;
}

void DialogSetTimeRange::on_dateTimeEdit_min_dateTimeChanged(const QDateTime &dateTime)
{
    min_time_ = dateTime.toSecsSinceEpoch();
}

void DialogSetTimeRange::on_dateTimeEdit_max_dateTimeChanged(const QDateTime &dateTime)
{
    max_time_ = dateTime.toSecsSinceEpoch();
}

void DialogSetTimeRange::on_comboBox_range_currentIndexChanged(int index)
{
    if (index < 0)
    {
        return;
    }

    double now = QDateTime::currentSecsSinceEpoch();
    switch (index)
    {
    case 0:
        max_time_ = now;
        min_time_ = now - 24 * 3600;
        break;
    case 1:
        max_time_ = now;
        min_time_ = now - 3 * 24 * 3600;
        break;
    case 2:
        max_time_ = now;
        min_time_ = now - 7 * 24 * 3600;
        break;
    case 3:
        max_time_ = now;
        min_time_ = now - 30 * 24 * 3600;
        break;
    default:
        throw std::invalid_argument("Unsupported time range.");
    }
}
