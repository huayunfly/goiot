#include "trendchart.h"
#include "events.h"

TrendChart::TrendChart(QWidget *parent,
                       const std::vector<ChartLineDef>& line_defs, const QString& title,
                       double interval, double value_range, double max_time_range,
                       double show_time_range) :
    QChartView(parent),
    title_(title),
    interval_(interval),
    max_value_range_(value_range),
    max_time_range_(max_time_range),
    show_time_range_(show_time_range),
    auto_scroll_chart_(true)
{
    // time range check
    if (max_time_range_ < show_time_range_)
    {
        max_time_range_ = show_time_range_;
    }
    max_line_point_count_ = max_time_range_ / interval_ + 1;
    if (max_line_point_count_ <= 1)
    {
        max_line_point_count_ = 2; // safe guard
    }

    QChart *chart = new QChart();
    chart->legend()->show();
    chart->legend()->setAlignment(Qt::AlignTop);

    auto axis_value = new QValueAxis();
    axis_value->setRange(0, max_value_range_);
    axis_value->setTickCount(9);

    auto axis_time = new QDateTimeAxis();
    auto current = QDateTime::currentDateTime();
    auto end_time = current.addSecs(show_time_range_);
    axis_time->setRange(current, end_time);
    axis_time->setFormat("HH:mm");
    axis_time->setTickCount(11);

    chart->setTitle(title_);
    chart->addAxis(axis_time, Qt::AlignBottom);
    chart->addAxis(axis_value, Qt::AlignLeft);

    // Initialize the graphic lines
    for (auto& item : line_defs)
    {
        auto line_series = std::make_shared<QLineSeries>();
        line_series->setName(item.pid_code + "_" + item.show_name);
        chart->addSeries(line_series.get());
        line_series->attachAxis(axis_value);
        line_series->attachAxis(axis_time);

        lines_map_.insert(std::make_pair(item.driver_id, line_series));
    }

//    for (const auto& kv : lines_map_)
//    {
//        std::get<2>(kv.second)->setName(
//                    std::get<0>(kv.second) + "_" + std::get<1>(kv.second)); // "TC2105_固定床下热"
//        chart->addSeries(std::get<2>(kv.second).get());
//        std::get<2>(kv.second)->attachAxis(axis_value);
//        std::get<2>(kv.second)->attachAxis(axis_time);
//    }

    this->setChart(chart);
    this->setRenderHint(QPainter::Antialiasing);

    // timer
    connect(&timer_, &QTimer::timeout, this, &TrendChart::UpdateChart);
    timer_.start(interval_ * 1000);
}

void TrendChart::SetRange(double interval, double value_range, double max_time_range, double show_time_range)
{
    interval_ = interval;
    max_value_range_ = value_range;
    max_time_range_ = max_time_range;
    show_time_range_ = show_time_range;
}

void TrendChart::AddOrUpdateData(const std::string& name,
                                 const std::pair<double/*value*/, double/*timestamp*/>& pair)
{
    data_table_.AddOrUpdateMapping(name, pair);
}


//bool TrendChart::event(QEvent* event)
//{
//    if (!event)
//    {
//        return false;
//    }

//    if (event->type() == Ui::RefreshTextEvent::myType)
//    {
//        Ui::RefreshTextEvent* e = static_cast<Ui::RefreshTextEvent*>(event);
//        // Find target UI control

//            const std::string& data_info_id = e->GetDataInfoId();
//            bool ok = false;
//            double value = e->Text().toDouble(&ok);
//            if (ok)
//            {
//                data_table_.AddOrUpdateMapping(data_info_id,
//                                               std::make_pair(value, e->GetTimestamp()));
//            }
//    }
//    return QWidget::event(event);
//}

void TrendChart::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        last_point_ = event->pos();
        auto_scroll_chart_ = false;
    }
    QChartView::mousePressEvent(event);
}

void TrendChart::mouseMoveEvent(QMouseEvent *event)
{
    if (!coordinate_item_)
    {
        coordinate_item_.reset(new QGraphicsSimpleTextItem(this->chart()));
        coordinate_item_->setZValue(5);
        coordinate_item_->setPos(100, 60);
        coordinate_item_->show();
    }

    const QPoint current_pos = event->pos();
    QPointF current_value = this->chart()->mapToValue(QPointF(current_pos));
    QString coordinate_string = QString("X = %1, Y = %2").arg(current_value.x()).arg(current_value.y());
    coordinate_item_->setText(coordinate_string);

    if (event->buttons() & Qt::LeftButton)
    {
        QPoint offset = current_pos - last_point_;
        last_point_ = current_pos;
        this->chart()->scroll(-offset.x(), offset.y());
    }
    QChartView::mouseMoveEvent(event);
}

void TrendChart::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        auto_scroll_chart_ = true;
    }
    QChartView::mouseReleaseEvent(event);
}

void TrendChart::UpdateChart()
{
    if (lines_map_.size() == 0)
    {
        return;
    }

    // Scroll datetime axis
    if (auto_scroll_chart_)
    {
        int show_line_point_count = show_time_range_ / interval_ + 1;
        int points = lines_map_.begin()->second->count();
        auto current = QDateTime::currentDateTime();
        auto begin_time = points < show_line_point_count ?
                    current.addSecs(-points * interval_) :
                    current.addSecs(-(show_line_point_count - 1) * interval_);
        auto end_time = points < show_line_point_count ?
                    current.addSecs((show_line_point_count - points - 1) * interval_) :
                    current;
        this->chart()->axes()[0]->setRange(begin_time, end_time);
    }

    // Append new value and truncate the older one.
    qint64 now_ms = QDateTime::currentMSecsSinceEpoch();
    for (const auto& [driver_id, line] : lines_map_)
    {
        if (line->count() >= max_line_point_count_)
        {
            line->remove(0);
        }
        auto value_pair = data_table_.ValueFor(driver_id.toStdString(), std::make_pair(-1.0, 0.0));
        if (value_pair.first < 0.0/* ignore deadband || abs(now_ms / 1000.0 - value_pair.second) > interval_*/)
        {
            line->append(now_ms + 1000, 100.0f);  // placehold
        }
        else
        {
            line->append(now_ms, value_pair.first);
        }
    }
}
