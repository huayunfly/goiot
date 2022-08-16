#ifndef FORM_TREND_H
#define FORM_TREND_H

#include <QWidget>
#include "trendchart.h"

namespace Ui {
class FormTrend;
}

class FormTrend : public QWidget
{
    Q_OBJECT

public:
    explicit FormTrend(QWidget *parent = nullptr);
    ~FormTrend();

protected:
    bool event(QEvent *event) override;

private:
    Ui::FormTrend *ui;
    std::unique_ptr<TrendChart> chart_reactor_;
    std::unique_ptr<TrendChart> chart_pg_;
};

#endif // FORM_TREND_H
