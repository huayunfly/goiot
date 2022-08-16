#include "form_trend.h"
#include "ui_form_trend.h"
#include "events.h"

FormTrend::FormTrend(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormTrend)
{
    ui->setupUi(this);
    std::vector<ChartLineDef> line_defs;
    line_defs.push_back(ChartLineDef("plc.1.temp101_pv", "TI4141", "#1"));
    line_defs.push_back(ChartLineDef("plc.1.temp102_pv", "TI4142", "#2"));
    line_defs.push_back(ChartLineDef("plc.1.temp103_pv", "TI4143", "#3"));
    line_defs.push_back(ChartLineDef("plc.1.temp104_pv", "TI4144", "#4"));
    line_defs.push_back(ChartLineDef("plc.1.temp105_pv", "TI4145", "#5"));
    line_defs.push_back(ChartLineDef("plc.1.temp106_pv", "TI4146", "#6"));
    line_defs.push_back(ChartLineDef("plc.1.temp107_pv", "TI4147", "#7"));
    line_defs.push_back(ChartLineDef("plc.1.temp108_pv", "TI4148", "#8"));
    line_defs.push_back(ChartLineDef("plc.1.temp109_pv", "TI4149", "#9"));
    line_defs.push_back(ChartLineDef("plc.1.temp110_pv", "TI4150", "#10"));
    line_defs.push_back(ChartLineDef("plc.1.temp111_pv", "TI4151", "#11"));
    line_defs.push_back(ChartLineDef("plc.1.temp112_pv", "TI4152", "#12"));
    line_defs.push_back(ChartLineDef("plc.1.temp113_pv", "TI4153", "#13"));
    line_defs.push_back(ChartLineDef("plc.1.temp114_pv", "TI4154", "#14"));
    line_defs.push_back(ChartLineDef("plc.1.temp115_pv", "TI4155", "#15"));
    line_defs.push_back(ChartLineDef("plc.1.temp116_pv", "TI4156", "#16"));

    chart_reactor_.reset(new TrendChart(nullptr/* can not be *this FormTrend*/, line_defs,
                                        QString("釜温℃"), 5/*interval*/, std::make_pair<double, double>(0, 200.0), 50/*segment*/, 360000, 7200));
    chart_reactor_->setObjectName(QString::fromUtf8("chart_reactor"));
    chart_reactor_->setMouseTracking(false);
    ui->horizontalLayout->addWidget(chart_reactor_.get());

    line_defs.clear();
    line_defs.push_back(ChartLineDef("mfcpfc.11.pv", "PICA4301", "#1"));
    line_defs.push_back(ChartLineDef("mfcpfc.12.pv", "PICA4302", "#2"));
    line_defs.push_back(ChartLineDef("mfcpfc.13.pv", "PICA4303", "#3"));
    line_defs.push_back(ChartLineDef("mfcpfc.14.pv", "PICA4304", "#4"));
    line_defs.push_back(ChartLineDef("mfcpfc.15.pv", "PICA4305", "#5"));
    line_defs.push_back(ChartLineDef("mfcpfc.16.pv", "PICA4306", "#6"));
    line_defs.push_back(ChartLineDef("mfcpfc.17.pv", "PICA4307", "#7"));
    line_defs.push_back(ChartLineDef("mfcpfc.18.pv", "PICA4308", "#8"));
    line_defs.push_back(ChartLineDef("mfcpfc.19.pv", "PICA4309", "#9"));
    line_defs.push_back(ChartLineDef("mfcpfc.20.pv", "PICA4310", "#10"));
    line_defs.push_back(ChartLineDef("mfcpfc.21.pv", "PICA4311", "#11"));
    line_defs.push_back(ChartLineDef("mfcpfc.22.pv", "PICA4312", "#12"));
    line_defs.push_back(ChartLineDef("mfcpfc.23.pv", "PICA4313", "#13"));
    line_defs.push_back(ChartLineDef("mfcpfc.24.pv", "PICA4314", "#14"));
    line_defs.push_back(ChartLineDef("mfcpfc.25.pv", "PICA4315", "#15"));
    line_defs.push_back(ChartLineDef("mfcpfc.26.pv", "PICA4316", "#16"));

    chart_pg_.reset(new TrendChart(nullptr/* can not be *this FormTrend*/, line_defs,
                                   QString("釜压bara"), 5/*interval*/, std::make_pair<double, double>(0, 50.0), 10/*segment*/, 360000, 7200));
    chart_pg_->setObjectName(QString::fromUtf8("chart_pg"));
    chart_pg_->setMouseTracking(false);
    ui->horizontalLayout_2->addWidget(chart_pg_.get());
}

FormTrend::~FormTrend()
{
    delete ui;
}

bool FormTrend::event(QEvent *event)
{
    if (event->type() == Ui::RefreshTextEvent::myType)
    {
        Ui::RefreshTextEvent* e = static_cast<Ui::RefreshTextEvent*>(event);
        // Find target UI control
        auto trend = this->findChild<TrendChart*>(e->Name());
        if (trend != nullptr)
        {
            const std::string& data_info_id = e->GetDataInfoId();
            bool ok = false;
            double value = e->Text().toDouble(&ok);
            if (ok)
            {
                  trend->AddOrUpdateData(data_info_id,
                                               std::make_pair(value, e->GetTimestamp()));
            }
        }
    }
    return QWidget::event(event);
}
