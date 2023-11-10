#ifndef FORM_EXPINFO_H
#define FORM_EXPINFO_H

#include <QWidget>
#include "form_common.h"

namespace Ui {
class FormExpInfo;
}

class FormExpInfo : public FormCommon
{
    Q_OBJECT

public:
    explicit FormExpInfo(QWidget *parent = nullptr, bool admin = true);
    ~FormExpInfo();

    bool event(QEvent *event) override;

    void InitUiState() override;

private slots:
    void on_button_filefolder_selection_clicked();

    void on_button_expname_clicked();

    void on_button_exp_run_clicked();

    void on_button_exp_stop_clicked();

private:
    Ui::FormExpInfo *ui;
};

#endif // FORM_EXPINFO_H
