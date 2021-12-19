#ifndef FORM_LIQUIDDISTRIBUTOR_H
#define FORM_LIQUIDDISTRIBUTOR_H

#include <QWidget>
#include <QTimer>
#include <QLabel>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <form_common.h>
#include "sampling_ui_item.h"

namespace Ui {
class FormLiquidDistributor;
}

enum class LiquidDistributorGroup
{
    SAMPLING = 0,
    COLLECTION = 1
};

class FormLiquidDistributor : public FormCommon
{
    Q_OBJECT

public:
    explicit FormLiquidDistributor(QWidget *parent = nullptr,
                                   const QString& object_name = QString(),
                                   const QString& display_name = QString(),
                                   LiquidDistributorGroup group = LiquidDistributorGroup::SAMPLING);
    ~FormLiquidDistributor();

    bool event(QEvent *event) override;

    void InitUiState() override;

protected:
    void paintEvent(QPaintEvent* e) override;

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_tableWidget_cellChanged(int row, int column);

    // Detect the image contoures
    void UpdateImage();

private:
    QString SaveLiquidSamplingProcedure();
    void LoadLiquidSamplingProcedure(const QString& record);
    std::list<std::vector<int>> SamplingRecordToList(const QString& record);

    // Fill item setting table.
    void FillTable(const std::list<std::vector<int>>& record_list);
    // Fill status chart.
    void FillStatusChart(const std::list<std::vector<int>>& record_list);

private:
    Ui::FormLiquidDistributor *ui;
    LiquidDistributorGroup group_;

    const int MAX_SAMPLING_POS = 128; // Max. sampling tube positions
    const int LINE_GROUPS = 4; // Liquid sampling line group
    const int LINE_ITEMS = 5;  // Liquid sampling line record items
    const int ROW_COUNT = 32 + 1; // 1 seperator row
    const int COL_COUNT = 20 + 1; // 1 seperator column
    const int COL_POS = 0;
    const int COL_CHANNEL = 1;
    const int COL_FLOW_LIMIT = 2;
    const int COL_SAMPLING_TIME = 3;
    const int COL_SOLVENT_TYPE = 4;
    const int COL_PURGE_PORT = 5;

    std::vector<std::shared_ptr<SamplingUIItem>> sampling_ui_items;

    QLabel* image_label_0; // raw pointer!!
    QTimer video_timer_0;
    cv::VideoCapture video_cap_0;
    cv::Mat video_frame_0;
};

#endif // FORM_LIQUIDDISTRIBUTOR_H
