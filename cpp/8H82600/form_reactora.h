#ifndef FORM_REACTORA_H
#define FORM_REACTORA_H

#include "form_common.h"

namespace Ui {
class FormReactorA;
}

class FormReactorA : public FormCommon
{
    Q_OBJECT

public:
    explicit FormReactorA(QWidget *parent = nullptr);
    ~FormReactorA();

    bool event(QEvent *event) override;

    void InitUiState() override;

    QString GetDisplayName() override
    {
        return QString::fromUtf8("反应器1-8");
    }

private:
    Ui::FormReactorA *ui;
};

#endif // FORM_REACTORA_H
