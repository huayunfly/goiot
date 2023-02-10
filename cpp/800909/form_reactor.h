#ifndef FORM_REACTOR_H
#define FORM_REACTOR_H

#include "form_common.h"

namespace Ui {
class FormReactor;
}

class FormReactor : public FormCommon
{
    Q_OBJECT

public:
    explicit FormReactor(QWidget *parent = nullptr);
    ~FormReactor();

    bool event(QEvent *event) override;

    void InitUiState() override;

    QString GetDisplayName() override
    {
        return QString::fromUtf8("反应器");
    }

private:
    Ui::FormReactor *ui;
};

#endif // FORM_REACTORA_H
