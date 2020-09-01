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
    explicit FormCylinderA(QWidget *parent = nullptr);
    ~FormCylinderA();

    bool event(QEvent *event) override;

    void InitUiState() override;

    QString GetDisplayName() override
    {
        return QString::fromUtf8("注射进样1-8");
    }


private:
    Ui::FormCylinderA *ui;
};

#endif // FORM_CYLINDERA_H
