#ifndef FORM_REACTORB_H
#define FORM_REACTORB_H

#include "form_common.h"

namespace Ui {
class FormReactorB;
}

class FormReactorB : public FormCommon
{
    Q_OBJECT

public:
    explicit FormReactorB(QWidget *parent = nullptr, bool admin = true);
    ~FormReactorB();

    bool event(QEvent *event) override;

    void InitUiState() override;

    QString GetDisplayName() override
    {
        return QString::fromUtf8("反应器9-16");
    }

private:
    Ui::FormReactorB *ui;
};

#endif // FORM_REACTORB_H
