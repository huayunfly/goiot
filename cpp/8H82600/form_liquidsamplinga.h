#ifndef FORM_LIQUIDSAMPLINGA_H
#define FORM_LIQUIDSAMPLINGA_H

#include "form_common.h"

namespace Ui {
class FormLiquidSamplingA;
}

class FormLiquidSamplingA : public FormCommon
{
    Q_OBJECT

public:
    explicit FormLiquidSamplingA(QWidget *parent = nullptr, bool admin = true);
    ~FormLiquidSamplingA();

    bool event(QEvent *event) override;

    void InitUiState() override;

private:
    Ui::FormLiquidSamplingA *ui;
};

#endif // FORM_LIQUIDSAMPLINGA_H
