#ifndef FORM_HISTORY_H
#define FORM_HISTORY_H

#include <QWidget>

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
};

#endif // FORM_HISTORY_H
