#include "form_trend.h"
#include "ui_form_trend.h"

FormTrend::FormTrend(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormTrend)
{
    ui->setupUi(this);
    std::vector<ChartLineDef> line_defs;
    line_defs.push_back(ChartLineDef("plc.1.temp1_pv", "TC2102", "固定床预热"));
    line_defs.push_back(ChartLineDef("plc.1.temp2_pv", "TC2103", "固定床上热"));
    line_defs.push_back(ChartLineDef("plc.1.temp3_pv", "TC2104", "固定床中热"));
    line_defs.push_back(ChartLineDef("plc.1.temp4_pv", "TC2105", "固定床下热"));

    chart_reactor_ = new TrendChart(nullptr/* can not be *this FormTrend*/, line_defs,
                                    QString("反应温度"), 5, std::make_pair<double, double>(0, 400.0), 36000, 7200);
    chart_reactor_->setObjectName(QString::fromUtf8("chart_reactor"));
    chart_reactor_->setMouseTracking(false);
    ui->horizontalLayout->addWidget(chart_reactor_);
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
