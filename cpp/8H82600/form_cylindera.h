#ifndef FORM_CYLINDERA_H
#define FORM_CYLINDERA_H

#include "form_common.h"

namespace Ui {
class FormCylinderA;
}

class FormCylinderA : public FormCommon
{
    Q_OBJECT

public:
    explicit FormCylinderA(QWidget *parent = nullptr, bool admin = true);
    ~FormCylinderA();

    bool event(QEvent *event) override;

    void InitUiState() override;

private:
    Ui::FormCylinderA *ui;
};

#endif // FORM_CYLINDERA_H
