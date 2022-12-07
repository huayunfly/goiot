#ifndef FORM_LIQUIDSWITCH_H
#define FORM_LIQUIDSWITCH_H

#include "form_common.h"

namespace Ui {
class FormLiquidSwitch;
}

class FormLiquidSwitch : public FormCommon
{
    Q_OBJECT

public:
    explicit FormLiquidSwitch(QWidget *parent = nullptr, bool admin = true);
    ~FormLiquidSwitch();

    bool event(QEvent *event) override;

    void InitUiState() override;

private:
    Ui::FormLiquidSwitch *ui;
};

#endif // FORM_LIQUIDSWITCH_H
