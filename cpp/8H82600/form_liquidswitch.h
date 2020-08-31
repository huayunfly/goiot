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
    explicit FormLiquidSwitch(QWidget *parent = nullptr);
    ~FormLiquidSwitch();

    bool event(QEvent *event) override;

    void InitUiState() override;

    QString GetDisplayName() override
    {
        return QString::fromUtf8("原液切换");
    }

private:
    Ui::FormLiquidSwitch *ui;
};

#endif // FORM_LIQUIDSWITCH_H
