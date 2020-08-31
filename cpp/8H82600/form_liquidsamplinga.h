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
    explicit FormLiquidSamplingA(QWidget *parent = nullptr);
    ~FormLiquidSamplingA();

    bool event(QEvent *event) override;

    void InitUiState() override;

    QString GetDisplayName() override
    {
        return QString::fromUtf8("采液1-8");
    }

private:
    Ui::FormLiquidSamplingA *ui;
};

#endif // FORM_LIQUIDSAMPLINGA_H
