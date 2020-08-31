#ifndef FORM_LIQUIDCOLLECTION_H
#define FORM_LIQUIDCOLLECTION_H

#include "form_common.h"

namespace Ui {
class FormLiquidCollection;
}

class FormLiquidCollection : public FormCommon
{
    Q_OBJECT

public:
    explicit FormLiquidCollection(QWidget *parent = nullptr);
    ~FormLiquidCollection();

    bool event(QEvent *event) override;

    void InitUiState() override;

    QString GetDisplayName() override
    {
        return QString::fromUtf8("液体收集");
    }

private:
    Ui::FormLiquidCollection *ui;
};

#endif // FORM_LIQUIDCOLLECTION_H
