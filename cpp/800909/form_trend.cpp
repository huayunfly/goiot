#include "form_trend.h"
#include "ui_form_trend.h"
#include "events.h"

FormTrend::FormTrend(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormTrend)
{
    ui->setupUi(this);
    std::vector<ChartLineDef> line_defs;
    line_defs.push_back(ChartLineDef("e5cc.1.pv", "TC101", "加热1"));
    line_defs.push_back(ChartLineDef("e5cc.2.pv", "TC102", "加热2"));
    line_defs.push_back(ChartLineDef("e5cc.3.pv", "TC103", "加热3"));
    line_defs.push_back(ChartLineDef("e5cc.4.pv", "TC104", "加热4"));
    line_defs.push_back(ChartLineDef("e5cc.5.pv", "TC105", "加热5"));
    line_defs.push_back(ChartLineDef("e5cc.6.pv", "TC106", "加热6"));
    line_defs.push_back(ChartLineDef("e5cc.7.pv", "TC107", "加热7"));
    line_defs.push_back(ChartLineDef("e5cc.8.pv", "TC108", "加热8"));

    chart_reactor_.reset(new TrendChart(nullptr/* can not be *this FormTrend*/, line_defs,
                                        QString("反应温度℃"), 5/*interval*/, std::make_pair<double, double>(0, 400.0), 50/*segment*/, 360000, 7200));
    chart_reactor_->setObjectName(QString::fromUtf8("chart_reactor"));
    chart_reactor_->setMouseTracking(false);
    ui->horizontalLayout->addWidget(chart_reactor_.get());

    line_defs.clear();
    line_defs.push_back(ChartLineDef("e5cc.9.pv", "TC201", "加热11"));
    line_defs.push_back(ChartLineDef("e5cc.10.pv", "TC202", "加热12"));
    line_defs.push_back(ChartLineDef("e5cc.11.pv", "TC203", "加热13"));
    line_defs.push_back(ChartLineDef("e5cc.12.pv", "TC204", "加热14"));
    line_defs.push_back(ChartLineDef("e5cc.13.pv", "TC205", "加热15"));
    line_defs.push_back(ChartLineDef("e5cc.14.pv", "TC206", "加热16"));
    line_defs.push_back(ChartLineDef("e5cc.15.pv", "TC207", "加热17"));
    line_defs.push_back(ChartLineDef("e5cc.16.pv", "TC208", "加热18"));

    chart_reactor_2_.reset(new TrendChart(nullptr/* can not be *this FormTrend*/, line_defs,
                                   QString("反应温度℃"), 5/*interval*/, std::make_pair<double, double>(0, 400.0), 50/*segment*/, 360000, 7200));
    chart_reactor_2_->setObjectName(QString::fromUtf8("chart_reactor2"));
    chart_reactor_2_->setMouseTracking(false);
    ui->horizontalLayout_2->addWidget(chart_reactor_2_.get());
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
