#ifndef FORM_GASFEED_H
#define FORM_GASFEED_H

#include "form_common.h"

namespace Ui {
class FormGasFeed;
}

class FormGasFeed : public FormCommon
{
    Q_OBJECT

public:
    explicit FormGasFeed(QWidget *parent = nullptr);
    ~FormGasFeed();

    bool event(QEvent *event) override;

    void InitUiState() override;

    QString GetDisplayName() override
    {
        return QString::fromUtf8("进气单元");
    }

private:
    Ui::FormGasFeed *ui;
};

#endif // FORM_GASFEED_H
