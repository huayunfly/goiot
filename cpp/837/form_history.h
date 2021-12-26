#ifndef FORM_HISTORY_H
#define FORM_HISTORY_H

#include <QWidget>
#include "historychart.h"

namespace Ui {
class FormHistory;
}

class FormHistory : public QWidget
{
    Q_OBJECT

public:
    explicit FormHistory(QWidget *parent = nullptr);
    ~FormHistory();

private:
    Ui::FormHistory *ui;
    std::unique_ptr<HistoryChart> chart_reactor_;
};

#endif // FORM_HISTORY_H
