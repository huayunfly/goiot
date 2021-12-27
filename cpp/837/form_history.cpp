#include "form_history.h"
#include "ui_form_history.h"

FormHistory::FormHistory(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormHistory)
{
    ui->setupUi(this);
    std::vector<HistoryLineDef> line_defs;
    line_defs.push_back(HistoryLineDef("plc.1.temp1_pv", "TC2102", "固定床预热"));
    line_defs.push_back(HistoryLineDef("plc.1.temp2_pv", "TC2103", "固定床上热"));
    line_defs.push_back(HistoryLineDef("plc.1.temp3_pv", "TC2104", "固定床中热"));
    line_defs.push_back(HistoryLineDef("plc.1.temp4_pv", "TC2105", "固定床下热"));

    chart_reactor_.reset(new HistoryChart(nullptr/* can not be *this form*/, line_defs,
                                          "QPSQL:127.0.0.1:5432:837:postgres:hello@123", QString("(历史)反应温度℃"), 5/*interval*/, std::make_pair<double, double>(0, 400.0), 50/*segment*/, 360000, 7200));
    chart_reactor_->setObjectName(QString::fromUtf8("chart_reactor"));
    chart_reactor_->setMouseTracking(false);
//    double min = QDateTime(QDate(2021, 12, 26), QTime(12, 0, 0)).toSecsSinceEpoch();
//    double max = QDateTime(QDate(2021, 12, 26), QTime(14, 0, 0)).toSecsSinceEpoch();
//    chart_reactor_->QueryByTimeRange(min, max);
    ui->horizontalLayout->addWidget(chart_reactor_.get());

    line_defs.clear();
    line_defs.push_back(HistoryLineDef("plc.1.pg_6", "PIA1113", "固定床前压力"));
    line_defs.push_back(HistoryLineDef("plc.1.pg_7", "PIA1513", "釜前压力"));
    line_defs.push_back(HistoryLineDef("plc.1.pg_8", "PIA2402", "固定床后压力"));
    line_defs.push_back(HistoryLineDef("plc.1.pg_9", "PIA2802", "釜后压力"));

    chart_pg_.reset(new HistoryChart(nullptr/* can not be *this form*/, line_defs,
                                          "QPSQL:127.0.0.1:5432:837:postgres:hello@123", QString("(历史)系统压力barG"), 5/*interval*/, std::make_pair<double, double>(0, 60.0), 10/*segment*/, 360000, 7200));
    chart_pg_->setObjectName(QString::fromUtf8("chart_pg"));
    chart_pg_->setMouseTracking(false);
    ui->horizontalLayout_2->addWidget(chart_pg_.get());
}

FormHistory::~FormHistory()
{
    delete ui;
}
