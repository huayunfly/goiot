#ifndef FORM_CYLINDERB_H
#define FORM_CYLINDERB_H

#include "form_common.h"

namespace Ui {
class FormCylinderB;
}

class FormCylinderB : public FormCommon
{
    Q_OBJECT

public:
    explicit FormCylinderB(QWidget *parent = nullptr, bool admin = true);
    ~FormCylinderB();

    bool event(QEvent *event) override;

    void InitUiState() override;

private:
    Ui::FormCylinderB *ui;
};

#endif // FORM_CYLINDERB_H
