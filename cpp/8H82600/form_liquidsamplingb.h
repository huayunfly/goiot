#ifndef FORM_LIQUIDSAMPLINGB_H
#define FORM_LIQUIDSAMPLINGB_H

#include "form_common.h"

namespace Ui {
class FormLiquidSamplingB;
}

class FormLiquidSamplingB : public FormCommon
{
    Q_OBJECT

public:
    explicit FormLiquidSamplingB(QWidget *parent = nullptr, bool admin = true);
    ~FormLiquidSamplingB();

    bool event(QEvent *event) override;

    void InitUiState() override;

private:
    Ui::FormLiquidSamplingB *ui;
};

#endif // FORM_LIQUIDSAMPLINGB_H
