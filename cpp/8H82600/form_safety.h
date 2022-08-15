#ifndef FORM_SAFETY_H
#define FORM_SAFETY_H

#include "form_common.h"

namespace Ui {
class FormSafety;
}

class FormSafety : public FormCommon
{
    Q_OBJECT

public:
    explicit FormSafety(QWidget *parent = nullptr);
    ~FormSafety();

    bool event(QEvent *event) override;

private:
    void on_buttonClicked(void);

private:
    Ui::FormSafety *ui;
    // constants
    const int ROW_COUNT = 16;
    const int COL_COUNT = 4;
    const int COL_NAME = 0;
    const int COL_STATUS = 1;
    const int COL_RUN = 2;
    const int COL_STOP = 3;
};

#endif // FORM_SAFETY_H
