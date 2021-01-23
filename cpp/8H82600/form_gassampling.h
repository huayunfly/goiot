#ifndef FORM_GASSAMPLING_H
#define FORM_GASSAMPLING_H

#include "form_common.h"

namespace Ui {
class FormGasSampling;
}

class FormGasSampling : public FormCommon
{
    Q_OBJECT

public:
    explicit FormGasSampling(QWidget *parent = nullptr);
    ~FormGasSampling();

    bool event(QEvent *event) override;

    void InitUiState() override;

private:
    Ui::FormGasSampling *ui;
};

#endif // FORM_GASSAMPLING_H
