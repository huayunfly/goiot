#include "form_history.h"
#include "ui_form_history.h"

FormHistory::FormHistory(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormHistory)
{
    ui->setupUi(this);
    std::vector<HistoryLineDef> line_defs;
    line_defs.push_back(HistoryLineDef("plc.1.temp101_pv", "TI4141", "#1"));
    line_defs.push_back(HistoryLineDef("plc.1.temp102_pv", "TI4142", "#2"));
    line_defs.push_back(HistoryLineDef("plc.1.temp103_pv", "TI4143", "#3"));
    line_defs.push_back(HistoryLineDef("plc.1.temp104_pv", "TI4144", "#4"));
    line_defs.push_back(HistoryLineDef("plc.1.temp105_pv", "TI4145", "#5"));
    line_defs.push_back(HistoryLineDef("plc.1.temp106_pv", "TI4146", "#6"));
    line_defs.push_back(HistoryLineDef("plc.1.temp107_pv", "TI4147", "#7"));
    line_defs.push_back(HistoryLineDef("plc.1.temp108_pv", "TI4148", "#8"));
    line_defs.push_back(HistoryLineDef("plc.1.temp109_pv", "TI4149", "#9"));
    line_defs.push_back(HistoryLineDef("plc.1.temp110_pv", "TI4150", "#10"));
    line_defs.push_back(HistoryLineDef("plc.1.temp111_pv", "TI4151", "#11"));
    line_defs.push_back(HistoryLineDef("plc.1.temp112_pv", "TI4152", "#12"));
    line_defs.push_back(HistoryLineDef("plc.1.temp113_pv", "TI4153", "#13"));
    line_defs.push_back(HistoryLineDef("plc.1.temp114_pv", "TI4154", "#14"));
    line_defs.push_back(HistoryLineDef("plc.1.temp115_pv", "TI4155", "#15"));
    line_defs.push_back(HistoryLineDef("plc.1.temp116_pv", "TI4156", "#16"));

    chart_reactor_.reset(new HistoryChart(nullptr/* can not be *this form*/, line_defs,
                                          "QPSQL:127.0.0.1:5432:8H82600:postgres:hello@123", QString("(历史)釜温℃"), 5/*interval*/, std::make_pair<double, double>(0, 200.0), 50/*segment*/, 360000, 7200));
    chart_reactor_->setObjectName(QString::fromUtf8("chart_reactor"));
    chart_reactor_->setMouseTracking(false);
//    double min = QDateTime(QDate(2021, 12, 26), QTime(12, 0, 0)).toSecsSinceEpoch();
//    double max = QDateTime(QDate(2021, 12, 26), QTime(14, 0, 0)).toSecsSinceEpoch();
//    chart_reactor_->QueryByTimeRange(min, max);
    ui->horizontalLayout->addWidget(chart_reactor_.get());

    line_defs.clear();
    line_defs.push_back(HistoryLineDef("mfcpfc.11.pv", "PICA4301", "#1"));
    line_defs.push_back(HistoryLineDef("mfcpfc.12.pv", "PICA4302", "#2"));
    line_defs.push_back(HistoryLineDef("mfcpfc.13.pv", "PICA4303", "#3"));
    line_defs.push_back(HistoryLineDef("mfcpfc.14.pv", "PICA4304", "#4"));
    line_defs.push_back(HistoryLineDef("mfcpfc.15.pv", "PICA4305", "#5"));
    line_defs.push_back(HistoryLineDef("mfcpfc.16.pv", "PICA4306", "#6"));
    line_defs.push_back(HistoryLineDef("mfcpfc.17.pv", "PICA4307", "#7"));
    line_defs.push_back(HistoryLineDef("mfcpfc.18.pv", "PICA4308", "#8"));
    line_defs.push_back(HistoryLineDef("mfcpfc.19.pv", "PICA4309", "#9"));
    line_defs.push_back(HistoryLineDef("mfcpfc.20.pv", "PICA4310", "#10"));
    line_defs.push_back(HistoryLineDef("mfcpfc.21.pv", "PICA4311", "#11"));
    line_defs.push_back(HistoryLineDef("mfcpfc.22.pv", "PICA4312", "#12"));
    line_defs.push_back(HistoryLineDef("mfcpfc.23.pv", "PICA4313", "#13"));
    line_defs.push_back(HistoryLineDef("mfcpfc.24.pv", "PICA4314", "#14"));
    line_defs.push_back(HistoryLineDef("mfcpfc.25.pv", "PICA4315", "#15"));
    line_defs.push_back(HistoryLineDef("mfcpfc.26.pv", "PICA4316", "#16"));

    chart_pg_.reset(new HistoryChart(nullptr/* can not be *this form*/, line_defs,
                                          "QPSQL:127.0.0.1:5432:8H82600:postgres:hello@123", QString("(历史)釜压bara"), 5/*interval*/, std::make_pair<double, double>(0, 50.0), 10/*segment*/, 360000, 7200));
    chart_pg_->setObjectName(QString::fromUtf8("chart_pg"));
    chart_pg_->setMouseTracking(false);
    ui->horizontalLayout_2->addWidget(chart_pg_.get());
}

FormHistory::~FormHistory()
{
    delete ui;
}
