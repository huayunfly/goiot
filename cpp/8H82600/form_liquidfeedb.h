#ifndef FORM_LIQUIDFEEDB_H
#define FORM_LIQUIDFEEDB_H

#include "form_common.h"

namespace Ui {
class FormLiquidFeedB;
}

class FormLiquidFeedB : public FormCommon
{
    Q_OBJECT

public:
    explicit FormLiquidFeedB(QWidget *parent = nullptr);
    ~FormLiquidFeedB();

    bool event(QEvent *event) override;

    void InitUiState() override;

private:
    Ui::FormLiquidFeedB *ui;
};

#endif // FORM_LIQUIDFEEDB_H
