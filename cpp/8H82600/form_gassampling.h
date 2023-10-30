#ifndef FORM_GASSAMPLING_H
#define FORM_GASSAMPLING_H

#include "form_common.h"

namespace Ui {
class FormGasSampling;
}

class FormGasSampling : public FormCommon
{
    Q_OBJECT

public:
    explicit FormGasSampling(QWidget *parent = nullptr, bool admin = true);
    ~FormGasSampling();

    bool event(QEvent *event) override;

    void InitUiState() override;

private:
    void SendGCStartPulseSignal(const QString& button_id);

private slots:
    void on_pushButton_GC1_start_clicked();

    void on_pushButton_GC2_start_clicked();

private:
    Ui::FormGasSampling *ui;
};

#endif // FORM_GASSAMPLING_H
