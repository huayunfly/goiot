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
    explicit FormLiquidSamplingB(QWidget *parent = nullptr);
    ~FormLiquidSamplingB();

    bool event(QEvent *event) override;

    void InitUiState() override;

    QString GetDisplayName() override
    {
        return QString::fromUtf8("采液9-16");
    }

private:
    Ui::FormLiquidSamplingB *ui;
};

#endif // FORM_LIQUIDSAMPLINGB_H
