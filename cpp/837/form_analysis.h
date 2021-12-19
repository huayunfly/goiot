#ifndef FORM_ANALYSIS_H
#define FORM_ANALYSIS_H

#include "form_common.h"

namespace Ui {
class FormAnalysis;
}

class FormAnalysis : public FormCommon
{
    Q_OBJECT

public:
    explicit FormAnalysis(QWidget *parent = nullptr);
    ~FormAnalysis();

    bool event(QEvent *event) override;

    void InitUiState() override;

    QString GetDisplayName() override
    {
        return QString::fromUtf8("分析接口");
    }

private:
    Ui::FormAnalysis *ui;
};

#endif // FORM_ANALYSIS_H
