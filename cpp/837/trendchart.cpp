#include "trendchart.h"
#include "events.h"

TrendChart::TrendChart(QWidget *parent,
                       const std::vector<ChartLineDef>& line_defs, const QString& title,
                       double interval, std::pair<double, double> value_range, double value_segment,
                       double max_time_range, double show_time_range) :
    QChartView(parent),
    title_(title),
    interval_(interval),
    value_range_(value_range),
    value_segment_(value_segment),
    max_time_range_(max_time_range),
    show_time_range_(show_time_range),
    auto_scroll_chart_(true)
{
    // range check
    if (max_time_range_ < show_time_range_)
    {
        max_time_range_ = show_time_range_;
    }
    max_line_point_count_ = max_time_range_ / interval_ + 1;
    if (max_line_point_count_ <= 1)
    {
        max_line_point_count_ = 2; // safe guard
    }
    if (value_range_.first > value_range_.second)
    {
        value_range_.first = 0;
        value_range_.second = 100.0;
    }
    if (value_segment < 1e-3)
    {
        value_segment = 50;
    }

    QChart *chart = new QChart();
    chart->legend()->show();
    chart->legend()->setAlignment(Qt::AlignTop);

    auto axis_value = new QValueAxis();
    axis_value->setRange(value_range_.first, value_range_.second);
    axis_value->setTickCount(
                static_cast<int>((value_range_.second - value_range_.first) / value_segment_) + 1);
    axis_value->setLabelFormat("%3.0f");

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

    this->setChart(chart);
    this->setRenderHint(QPainter::Antialiasing);

    // timer
    connect(&timer_, &QTimer::timeout, this, &TrendChart::UpdateChart);
    timer_.start(interval_ * 1000);
}

void TrendChart::SetRange(double interval, std::pair<double, double> value_range,
                          double value_segment, double max_time_range, double show_time_range)
{
    interval_ = interval;
    value_range_ = value_range;
    value_segment_ = value_segment;
    max_time_range_ = max_time_range;
    show_time_range_ = show_time_range;
}

void TrendChart::AddOrUpdateData(const std::string& name,
                                 const std::pair<double/*value*/, double/*timestamp*/>& pair)
{
    data_table_.AddOrUpdateMapping(name, pair);
}

void TrendChart::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        QCursor cursor;
        cursor.setShape(Qt::ClosedHandCursor);
        QApplication::setOverrideCursor(cursor);
        last_pos_ = event->pos();
        auto_scroll_chart_ = false;
    }
    event->accept();
}

void TrendChart::mouseMoveEvent(QMouseEvent *event)
{
    if (!coordinate_item_)
    {
        coordinate_item_.reset(new QGraphicsSimpleTextItem(this->chart()));
        coordinate_item_->setZValue(5);
        coordinate_item_->setPos(60, 60);
        coordinate_item_->show();
    }

    const QPoint current_pos = event->pos();
    QPointF current_value = this->chart()->mapToValue(QPointF(current_pos)); // map to X-axis in ms, Y-axis in vlue
    QString coordinate_string = QString("X = %1, Y = %2").
            arg(QDateTime::fromMSecsSinceEpoch(current_value.x()).toString("dd日 hh:mm")).
            arg(QString::number(current_value.y(), 'f', 1));
    coordinate_item_->setText(coordinate_string);

    if (event->buttons() & Qt::LeftButton)
    {
        auto_scroll_chart_ = false;
        QPoint offset_pos;
        offset_pos = current_pos - last_pos_;
        last_pos_ = event->pos();
        double offset_second = offset_pos.x() * show_time_range_ / this->size().width();
        this->chart()->scroll(-offset_second, 0); // chart()->scroll(x, y), x/y is ratio
    }
    event->accept();
}

void TrendChart::wheelEvent(QWheelEvent *event)
{
    // A positive value indicates that the wheel was rotated forwards away from the user;
    // Most mouse types (Y-vertical) work in steps of 15 degrees, in which case the delta value is a multiple of 120; i.e., 120 units * 1/8 = 15 degrees.
    auto_scroll_chart_ = false;
    QPoint num_degrees = event->angleDelta() / 8;
    double ratio = 1.0 - num_degrees.y() / 180.0;
    QRectF plot_area = this->chart()->plotArea();
    QPointF center =  plot_area.center();
    plot_area.setWidth(plot_area.width() * ratio);
    plot_area.setHeight(plot_area.height() * ratio);
    QPointF new_center(2 * center - event->pos() - (center - event->pos()) / ratio);
    plot_area.moveCenter(new_center); // keep mouse point nearly unmoved.
    this->chart()->zoomIn(plot_area);
    event->accept();
}

void TrendChart::mouseReleaseEvent(QMouseEvent *event)
{

    QApplication::restoreOverrideCursor();
    if (event->button() == Qt::RightButton)
    {
        this->chart()->zoomReset();
        auto_scroll_chart_ = true;
    }
    event->accept();
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
            line->append(now_ms, 0.0f);  // placehold
        }
        else
        {
            line->append(now_ms, value_pair.first);
        }
    }
}
