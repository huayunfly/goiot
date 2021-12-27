#include "historychart.h"
#include "events.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include "dialog_settimerange.h"

HistoryChart::HistoryChart(QWidget *parent, const std::vector<HistoryLineDef>& line_defs,
                           const QString& connection_path, const QString& title,
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

    // DB connection path
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

    db_ = QSqlDatabase::addDatabase(dbdriver_);
    db_.setHostName(hostname_);
    db_.setPort(port_);
    db_.setDatabaseName(dbname_);
    db_.setUserName(username_);
    db_.setPassword(password_);
    bool is_open = db_.open();
    if (!is_open)
    {
        QMessageBox::critical(0, "无法连接数据库",
                              db_.lastError().text(), QMessageBox::Ignore);
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
}

HistoryChart::~HistoryChart()
{
    if (db_.isOpen())
    {
       db_.close();
    }
}

void HistoryChart::SetRange(double interval, std::pair<double, double> value_range,
                          double value_segment, double max_time_range, double show_time_range)
{
    interval_ = interval;
    value_range_ = value_range;
    value_segment_ = value_segment;
    max_time_range_ = max_time_range;
    show_time_range_ = show_time_range;
}

void HistoryChart::mousePressEvent(QMouseEvent *event)
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

void HistoryChart::mouseMoveEvent(QMouseEvent *event)
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

void HistoryChart::wheelEvent(QWheelEvent *event)
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

void HistoryChart::mouseReleaseEvent(QMouseEvent *event)
{

    QApplication::restoreOverrideCursor();
    if (event->button() == Qt::RightButton)
    {
        this->chart()->zoomReset();
        auto_scroll_chart_ = true;
    }
    event->accept();
}

void HistoryChart::mouseDoubleClickEvent(QMouseEvent *event)
{
    DialogSetTimeRange dlg_set_time = DialogSetTimeRange(this);
    dlg_set_time.move(event->pos());
    int result = dlg_set_time.exec();
    if (result == QDialog::Accepted)
    {
        QueryByTimeRange(dlg_set_time.Min(), dlg_set_time.Max());
    }
}

void HistoryChart::QueryByTimeRange(double min, double max)
{
    if (min >= max)
    {
        return;
    }
    if (lines_map_.size() == 0)
    {
        return;
    }

    this->chart()->axes()[0]->setRange(QDateTime::fromSecsSinceEpoch(min),
                                       QDateTime::fromSecsSinceEpoch(max));

    // Normally lines_map_ is a group of the same driver data, like 'mfc' etc.
    QStringList list = lines_map_.begin()->first.split(".", Qt::SkipEmptyParts);
    if (list.size() < 3)
    {
        return;
    }
    QString table = list[0]; // DB table name is driver id, mfc etc.
    std::vector<QString> columns;
    for (auto& item : lines_map_)
    {
        columns.push_back(item.first); // Table column name is data_id, mfc.1.pv etc.
        item.second->clear(); // Clear QLineSeries
    }

    auto records = QueryData(table, columns, std::make_pair(min, max));
    if (records.size() == 0)
    {
        return;
    }
    for (std::size_t i = 0; i < columns.size(); i++)
    {
        auto line = lines_map_.at(columns.at(i));
        for (const auto& [value, time] : records.at(i))
        {
            line->append(time * 1000/*ms*/, value);
        }
    }
}

std::vector<std::vector<std::tuple<double/*value*/, double/*time*/>>>
HistoryChart::QueryData(QString table, std::vector<QString> columns,
        std::pair<double, double> time_range)
{
    if (!db_.isOpen())
    {
        return std::vector<std::vector<std::tuple<double, double>>>();
    }
    QSqlQuery query(db_);
    QString query_string;
    query_string.append("select ");
    for (auto& column : columns)
    {
        query_string.append("\"");
        query_string.append(column);
        query_string.append("\", ");
    }
    query_string.append("\"time\" ");
    query_string.append("from \"");
    query_string.append(table);
    query_string.append("\" where \"time\" >= ");
    query_string.append(QString::number(time_range.first));
    query_string.append(" and \"time\" <= ");
    query_string.append(QString::number(time_range.second));
    query_string.append(";");

    bool ok = query.exec(query_string);
    if (!ok)
    {
        return std::vector<std::vector<std::tuple<double, double>>>();
    }

    auto records = std::vector<std::vector<std::tuple<double, double>>>(
                columns.size());
    while(query.next())
    {
        QSqlRecord record = query.record();
        double time = record.value("time").toDouble(&ok);
        if (ok)
        {
            for (std::size_t i = 0; i < columns.size(); i++)
            {
                double value = record.value(columns.at(i)).toDouble(&ok);
                if (ok)
                {
                    records.at(i).push_back(std::make_pair(value, time));
                }
                else
                {
                    records.at(i).push_back(std::make_pair(0, time)); // for NULL
                }
            }
        }
    }
    return records;
}
