#ifndef HistoryChart_H
#define HistoryChart_H

#include <QWidget>
#include <QtCharts>
#include <QTimer>
#include <unordered_map>
#include <tuple>
#include <string>
#include "threadsafe_lookup_table.h"
#include <QSqlDatabase>

// For QPSQL driver, copy PostgreSQL 64bit or 32bit DLLs to the execute folder:
// libcrypto-1_1-x64.dll, libiconv-2.dll, libintl-8.dll, libpq.dll, libssl-1_1-x64.dll

struct HistoryLineDef
{
    QString driver_id;
    QString pid_code;
    QString show_name;

    HistoryLineDef(const QString& id, const QString& code, const QString& name) :
        driver_id(id), pid_code(code), show_name(name)
    {

    }
};

class HistoryChart : public QChartView
{
    Q_OBJECT
public:
    HistoryChart(QWidget *parent = nullptr, const std::vector<HistoryLineDef>& line_defs = std::vector<HistoryLineDef>(),
               const QString& connection_path = "", const QString& title = "", double interval = 5, std::pair<double/*min*/, double/*max*/> value_range = std::make_pair(0, 100),
               double value_segment = 50,
               double max_time_range = 36000,
               double show_time_range = 7200);

    ~HistoryChart();

    HistoryChart(HistoryChart const& chart) = delete;
    HistoryChart& operator=(HistoryChart const& chart) = delete;

    void SetRange(double interval, std::pair<double/*min*/, double/*max*/> value_range,
                  double value_segment,
                  double max_time_range, double show_time_range);

    void QueryByTimeRange(double min/*second*/, double max/*second*/);

protected:
//    bool event(QEvent* event) override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

private:
    void UpdateChart();

    std::vector<std::vector<std::tuple<double/*value*/, double/*time*/>>>
    QueryData(QString table, std::vector<QString> columns,
            std::pair<double, double> time_range);

private:
    ThreadSafeLookupTable<std::string, std::pair<double/*value*/, double/*timestamp*/> > data_table_;
    std::unordered_map<QString/*driver id*/, std::shared_ptr<QLineSeries>> lines_map_;
    QTimer timer_;
    QString title_;
    double interval_; // in second
    std::pair<double, double> value_range_; // (min, max)
    double value_segment_; // Y-axis segment
    double max_time_range_; // in second
    double show_time_range_; // in second <= max_time_range
    int max_line_point_count_;
    QPoint last_pos_;
    bool auto_scroll_chart_;
    std::unique_ptr<QGraphicsSimpleTextItem> coordinate_item_;

    QSqlDatabase db_;
    QString dbdriver_;
    QString hostname_;
    int port_;
    QString dbname_;
    QString username_;
    QString password_;
};

#endif // HistoryChart_H
