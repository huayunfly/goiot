#ifndef FORM_LIQUIDDISTRIBUTOR_H
#define FORM_LIQUIDDISTRIBUTOR_H

#include <QWidget>
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

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    QString SaveLiquidSamplingProcedure();
    void LoadLiquidSamplingProcedure(const QString& record);
    std::list<std::vector<int>> SamplingRecordToList(const QString& record);

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
};

#endif // FORM_LIQUIDDISTRIBUTOR_H
