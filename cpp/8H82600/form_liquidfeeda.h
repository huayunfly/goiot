#ifndef FORM_LIQUIDFEEDA_H
#define FORM_LIQUIDFEEDA_H

#include "form_common.h"

namespace Ui {
class FormLiquidFeedA;
}

class FormLiquidFeedA : public FormCommon
{
    Q_OBJECT

public:
    explicit FormLiquidFeedA(QWidget *parent = nullptr, bool admin = true);
    ~FormLiquidFeedA();

    bool event(QEvent *event) override;

    void InitUiState() override;

private:
    Ui::FormLiquidFeedA *ui;
};

#endif // FORM_LIQUIDFEEDA_H
