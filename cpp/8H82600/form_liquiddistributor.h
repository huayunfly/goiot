#ifndef FORM_LIQUIDDISTRIBUTOR_H
#define FORM_LIQUIDDISTRIBUTOR_H

#include <QWidget>
#include <QTimer>
#include <QLabel>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <form_common.h>
#include "sampling_ui_item.h"
#include <QSqlDatabase>

// For QPSQL driver, copy PostgreSQL 64bit or 32bit DLLs to the execute folder:
// libcrypto-1_1-x64.dll, libiconv-2.dll, libintl-8.dll, libpq.dll, libssl-1_1-x64.dll

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
                                   const QString& connection_path = QString(),
                                   LiquidDistributorGroup group = LiquidDistributorGroup::SAMPLING
                                   );
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
    // Save the procedure parameter to a record string.
    QString SaveLiquidSamplingProcedure();

    // Save the liquid sampling recipe to DB.
    bool SaveLiquidSamplingRecipe(const QString& recipe_name);

    // Load procedure from a record string.
    void LoadLiquidSamplingProcedure(const QString& record);

    // Load the liquid sampling recipe from DB.
    bool LoadLiquidSamplingRecipe(const QString& recipe_name);

    // Unpack the record string to the parameter list.
    std::list<std::vector<int>> SamplingRecordToList(const QString& record);

    // Fill item setting table.
    void FillTable(const std::list<std::vector<int>>& record_list);

    // Fill status chart.
    void FillStatusChart(const std::list<std::vector<int>>& record_list);

    // Clear the liquid sampling UI table.
    void ClearUITable();

    // Fill channel information in the liquid sampling UI table.
    void FillUITableChannelInfo(int pos, int channel, int flowlimit,
                                int duration, int cleanport);

    // Prepare DB environment.
    void PrepareDB(const QString& connection_path, const QString& instance_name);

    // Create sampling recipe and runtime DB tables.
    // @return true if it succeeded.
    bool CreateDBTable(const QSqlDatabase& db, const QString& table_name,
                       const std::vector<QString> columns, QString& error_massage);

    // Read sampling recipe table and return the cursor.
    // @return true if it succeeded.
    bool ReadRecipeFromDB(const QString& tablename, const QString& recipe_name,
                          const std::vector<QString> columns, QSqlQuery& query,
                          std::vector<std::shared_ptr<std::vector<QString>>>& value_list,
                          QString& error_message);

    // Write a step to the recipe-runtime table.
    // Every row in the recipe-runtime table will generate two steps in the runtime table:
    // 'run' column = 1 or 0
    bool WriteRecipeStepToDB(const QString& tablename, const std::vector<QString> columns,
                             const std::vector<QString> values, QSqlQuery& query, QString& error_message);

    // Write to recipe table.
    bool WriteRecipeToDB(const QString& tablename, const std::vector<QString> columns,
                                    const std::vector<std::vector<QString>> value_list,
                                    QSqlQuery& query, QString& error_message);

private:
    Ui::FormLiquidDistributor *ui;
    LiquidDistributorGroup group_;

    // DB variables
    QSqlDatabase db_;
    QString db_title_;
    QString dbdriver_;
    QString hostname_;
    int port_;
    QString dbname_;
    QString username_;
    QString password_;
    bool db_ready_;
    std::vector<QString> table_columns_;
    const QString recipe_table_name_ = "liquid_distribute_recipe";
    const QString runtime_table_name_ = "liquid_distribute_runtime";
    const int TYPE_SAMPLING = 0;
    const int TYPE_SAMPLING_PURGE = 1;
    const int TYPE_COLLECTION = 2;
    const int TYPE_COLLECTION_PURGE = 3;

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

    const int INDEX_TYPE = 1;
    const int INDEX_CHANNEL_A = 2;
    const int INDEX_CHANNEL_B = 3;
    const int INDEX_POS_A = 6;
    const int INDEX_POS_B = 7;
    const int INDEX_FLOWLIMIT_A = 8;
    const int INDEX_FLOWLIMIT_B = 9;
    const int INDEX_CLEANPORT_A = 10;
    const int INDEX_CLEANPORT_B = 11;
    const int INDEX_DURATION_A = 12;
    const int INDEX_DURATION_B = 13;
    const int INDEX_CONTROL_CODE = 16;

    std::vector<std::shared_ptr<SamplingUIItem>> sampling_ui_items;

    QLabel* image_label_0; // raw pointer!!
    QTimer video_timer_0;
    cv::VideoCapture video_cap_0;
    cv::Mat video_frame_0;
};

#endif // FORM_LIQUIDDISTRIBUTOR_H
