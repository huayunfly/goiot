#ifndef FORM_DISTRIBUTORSETTING_H
#define FORM_DISTRIBUTORSETTING_H

#include <QWidget>
#include "form_common.h"
#include "sampling_ui_item.h"

namespace Ui {
class FormDistributorSetting;
}

class FormDistributorSetting : public FormCommon
{
    Q_OBJECT

public:
    explicit FormDistributorSetting(QWidget *parent = nullptr);
    ~FormDistributorSetting();

    bool event(QEvent *event) override;

private slots:
    void on_button_servo_on_clicked();

    void on_button_servo_off_clicked();

    void on_button_ack_error_clicked();

    void on_button_speedY_clicked();

    void on_button_moveX_clicked();

    void on_button_moveY_clicked();

private:
    // Initialize runtime view.
    void InitRuntimeView();

    // Initialize the sampling item.
    // @param <x_gap>: circle gap in x.
    // @param <y_gap>: circle gap in y.
    // @param <radius>: circle radius.
    // @Param <x_count>: circle count in x.
    // @param <y_count>: circle count in y.
    // @param <y_section>: circle section for sink positions in y.
    // @param <pos_x>: position x
    // @param <pos_y>: position y
    void InitSamplingItem(int x_gap, int y_gap, double radius,
                         int x_count, int y_count, int y_section,
                         int pos_x, int pos_y);

    void InitControlPanel();

private:
    Ui::FormDistributorSetting *ui;

    // sampling runtime UI group
    std::vector<std::shared_ptr<SamplingUIItem>> sampling_ui_items;
};

#endif // FORM_DISTRIBUTORSETTING_H
