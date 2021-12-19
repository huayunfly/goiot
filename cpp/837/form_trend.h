#ifndef FORM_TREND_H
#define FORM_TREND_H

#include <QWidget>

namespace Ui {
class FormTrend;
}

class FormTrend : public QWidget
{
    Q_OBJECT

public:
    explicit FormTrend(QWidget *parent = nullptr);
    ~FormTrend();

private:
    Ui::FormTrend *ui;
};

#endif // FORM_TREND_H
