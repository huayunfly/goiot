#include "form_history.h"
#include "ui_form_history.h"

FormHistory::FormHistory(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormHistory)
{
    ui->setupUi(this);
    std::vector<HistoryLineDef> line_defs;
    line_defs.push_back(HistoryLineDef("e5cc.1.pv", "TC101", "加热1"));
    line_defs.push_back(HistoryLineDef("e5cc.2.pv", "TC102", "加热2"));
    line_defs.push_back(HistoryLineDef("e5cc.3.pv", "TC103", "加热3"));
    line_defs.push_back(HistoryLineDef("e5cc.4.pv", "TC104", "加热4"));
    line_defs.push_back(HistoryLineDef("e5cc.5.pv", "TC105", "加热5"));
    line_defs.push_back(HistoryLineDef("e5cc.6.pv", "TC106", "加热6"));
    line_defs.push_back(HistoryLineDef("e5cc.7.pv", "TC107", "加热7"));
    line_defs.push_back(HistoryLineDef("e5cc.8.pv", "TC108", "加热8"));

    chart_reactor_.reset(new HistoryChart(nullptr/* can not be *this form*/, line_defs,
                                          "QPSQL:127.0.0.1:5432:800909:postgres:hello@123", QString("(历史)反应温度℃"), 5/*interval*/, std::make_pair<double, double>(0, 400.0), 50/*segment*/, 360000, 7200));
    chart_reactor_->setObjectName(QString::fromUtf8("chart_reactor"));
    chart_reactor_->setMouseTracking(false);
//    double min = QDateTime(QDate(2021, 12, 26), QTime(12, 0, 0)).toSecsSinceEpoch();
//    double max = QDateTime(QDate(2021, 12, 26), QTime(14, 0, 0)).toSecsSinceEpoch();
//    chart_reactor_->QueryByTimeRange(min, max);
    ui->horizontalLayout->addWidget(chart_reactor_.get());

    line_defs.clear();
    line_defs.push_back(HistoryLineDef("e5cc.9.pv", "TC201", "加热21"));
    line_defs.push_back(HistoryLineDef("e5cc.10.pv", "TC202", "加热22"));
    line_defs.push_back(HistoryLineDef("e5cc.11.pv", "TC203", "加热23"));
    line_defs.push_back(HistoryLineDef("e5cc.12.pv", "TC204", "加热24"));
    line_defs.push_back(HistoryLineDef("e5cc.13.pv", "TC205", "加热25"));
    line_defs.push_back(HistoryLineDef("e5cc.14.pv", "TC206", "加热26"));
    line_defs.push_back(HistoryLineDef("e5cc.15.pv", "TC207", "加热27"));
    line_defs.push_back(HistoryLineDef("e5cc.16.pv", "TC208", "加热28"));

    chart_reactor_2_.reset(new HistoryChart(nullptr/* can not be *this form*/, line_defs,
                                          "QPSQL:127.0.0.1:5432:837:postgres:hello@123", QString("(历史)反应温度℃"), 5/*interval*/, std::make_pair<double, double>(0, 400.0), 50/*segment*/, 360000, 7200));
    chart_reactor_2_->setObjectName(QString::fromUtf8("chart_reactor2"));
    chart_reactor_2_->setMouseTracking(false);
    ui->horizontalLayout_2->addWidget(chart_reactor_2_.get());
}

FormHistory::~FormHistory()
{
    delete ui;
}
