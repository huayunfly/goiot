// 在界面使用Qt Creator添加一个QGraphicsView组件，然后提升为QChartView。
// 提升的时候，这样写提升为的类：QtCharts::QChartView，头文件写：qchartview.h。

#include "form_trend.h"
#include "ui_form_trend.h"

FormTrend::FormTrend(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormTrend)
{
    ui->setupUi(this);

    QLineSeries *series = new QLineSeries();
    series->append(0, 6);
    series->append(2, 4);
    series->append(3, 8);
    series->append(7, 4);
    series->append(10, 5);
    *series << QPointF(11, 1) << QPointF(13, 3) << QPointF(17, 6) << QPointF(18, 3) << QPointF(20, 2);

    QChart *chart = new QChart();
    chart->legend()->hide();
    chart->addSeries(series);
    chart->createDefaultAxes();
    chart->setTitle("Simple line chart example");

    ui->graphicsView->setChart(chart);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
}

FormTrend::~FormTrend()
{
    delete ui;
}
