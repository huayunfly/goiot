#ifndef FORM_LIQUIDDISTRIBUTOR_H
#define FORM_LIQUIDDISTRIBUTOR_H

#include <QWidget>
#include <QTimer>
#include <QLabel>
#include <QSqlDatabase>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <thread>
#include <mutex>
#include <future>
#include <form_common.h>
#include "ThreadSafeQueue.h"
#include "sampling_ui_item.h"

// For QPSQL driver, copy PostgreSQL 64bit or 32bit DLLs to the execute folder:
// libcrypto-1_1-x64.dll, libiconv-2.dll, libintl-8.dll, libpq.dll, libssl-1_1-x64.dll

namespace Ui {
class FormLiquidDistributor;
}

enum class LiquidDistributorCategory
{
    SAMPLING = 0,
    COLLECTION = 1
};

struct RecipeTaskEntity
{
    QString recipe_name;
    int type;
    int channel_a;
    int channel_b;
    int x;
    int y;
    int pos_a;
    int pos_b;
    int flowlimit_a;
    int flowlimit_b;
    int cleanport_a;
    int cleanport_b;
    int duration_a;
    int duration_b;
    bool run_a;
    bool run_b;
    uint control_code;

    std::vector<QString> ToValues()
    {
        return std::vector<QString>({recipe_name,
                                     QString::number(type),
                                     QString::number(channel_a),
                                     QString::number(channel_b),
                                     QString::number(x),
                                     QString::number(y),
                                     QString::number(pos_a),
                                     QString::number(pos_b),
                                     QString::number(flowlimit_a),
                                     QString::number(flowlimit_b),
                                     QString::number(cleanport_a),
                                     QString::number(cleanport_b),
                                     QString::number(duration_a),
                                     QString::number(duration_b),
                                     QString::number(run_a),
                                     QString::number(run_b),
                                     QString::number(control_code),});
    }
};

class FormLiquidDistributor : public FormCommon
{
    Q_OBJECT

    enum class StatusCheckGroup
    {
        A = 0,
        B = 1
    };

public:
    explicit FormLiquidDistributor(QWidget *parent = nullptr,
                                   const QString& object_name = QString(),
                                   const QString& display_name = QString(),
                                   const QString& connection_path = QString(),
                                   LiquidDistributorCategory group = LiquidDistributorCategory::SAMPLING
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
    void UpdateImage(int index);

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

private:
    //(Deprecated) Save the procedure parameter to a record string.
    QString SaveLiquidSamplingProcedure();
    //(Deprecated) Load procedure from a record string.
    void LoadLiquidSamplingProcedure(const QString& record);
    // (Deprecated) Unpack the record string to the parameter list.
    std::list<std::vector<int>> SamplingRecordToList(const QString& record);

    // Initialize video captrues.
    void InitVideoCaps();

    // Initialize recipe runtime view.
    void InitRecipeRuntimeView();

    // Initialize recipe runtime view.
    void InitRecipeRuntimeView(LiquidDistributorCategory category);

    // Initialize recipe runtime view, with circles representing the sampling status.
    // @param <x_gap>: circle gap in x.
    // @param <y_gap>: circle gap in y.
    // @param <radius>: circle radius.
    // @Param <x_count>: circle count in x.
    // @param <y_count>: circle count in y.
    // @param <y_section>: circle section for sink positions in y.
    void InitRecipeRuntimeView(int x_gap, int y_gap,
                               int radius, int x_count, int y_count, int y_section);

    // Initialize recipe setting table.
    void InitRecipeSettingTable();

    // Initialize recipe setting table by category.
    void InitRecipeSettingTable(LiquidDistributorCategory category);

    // Initialize recipe setting table.
    // @param <line_seperator>: UI seperator
    // @param <channel_groups>: liquid sampling and collection group numbers
    // @param <col_count>: column number
    // @param <row_count>: row number
    // @param <line_items>: UI line (column) containing cell items
    // @param <heads>: UI table head names
    void InitRecipeSettingTable(int line_seperator,
                                int channel_groups,
                                int col_count,
                                int row_count,
                                const QStringList& heads);

    // Save the liquid sampling recipe to DB.
    bool SaveLiquidSamplingRecipe(const QString& recipe_name);

    // Load the liquid sampling recipe from DB.
    bool LoadLiquidSamplingRecipe(const QString& recipe_name);

    // Read a recipe from DB and dispatch recipe task to queue.
    bool DispatchRecipeTask(const QString& recipe_name);

    // Run recipe backend thread.
    void RunRecipeWorker();

    // Liquid sampling status check task A, using timeout
    qint64 SamplingStatusCheckByTime(int second, StatusCheckGroup status);

    // Liquid sampling status check task B, using liquid image recognition.
    void SamplingStatusCheckByImageDetection(const cv::VideoCapture& v_cap);

    // Fill item setting table.
    void FillTable(const std::list<std::vector<int>>& record_list);

    // Fill status chart.
    void FillStatusChart(const std::list<std::vector<int>>& record_list);

    // Clear the UI setting table by category
    void ClearUITable(LiquidDistributorCategory category);

    // Clear the recipe setting table.
    void ClearUITable(int row_count, int channel_groups, int line_items,
                      LiquidDistributorCategory category);

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
                             const std::vector<QString> values, QSqlQuery& query, QString& error_message,
                             qint64 msecs = 0);

    // Write to recipe table.
    bool WriteRecipeToDB(const QString& tablename, const std::vector<QString> columns,
                                    const std::vector<std::vector<QString>> value_list,
                                    QSqlQuery& query, QString& error_message);

private:
    Ui::FormLiquidDistributor *ui;
    LiquidDistributorCategory category_;

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
    const QString control_code_name_ = "control_code";
    const QString channel_a_run_name_ = "channel_a_run";
    const QString channel_b_run_name_ = "channel_b_run";
    const int TYPE_SAMPLING = 0;
    const int TYPE_SAMPLING_PURGE = 1;
    const int TYPE_COLLECTION = 2;
    const int TYPE_COLLECTION_PURGE = 3;
    const int INDEX_RECIPE_NAME = 0;
    const int INDEX_TYPE = 1;
    const int INDEX_CHANNEL_A = 2;
    const int INDEX_CHANNEL_B = 3;
    const int INDEX_X = 4;
    const int INDEX_Y = 5;
    const int INDEX_POS_A = 6;
    const int INDEX_POS_B = 7;
    const int INDEX_FLOWLIMIT_A = 8;
    const int INDEX_FLOWLIMIT_B = 9;
    const int INDEX_CLEANPORT_A = 10;
    const int INDEX_CLEANPORT_B = 11;
    const int INDEX_DURATION_A = 12;
    const int INDEX_DURATION_B = 13;
    const int INDEX_RUN_A = 14;
    const int INDEX_RUN_B = 15;
    const int INDEX_CONTROL_CODE = 16;
    const int CHANNEL_A_STOP_AND_MASK = 0xdfffffff;
    const int CHANNEL_B_STOP_AND_MASK = 0xbfffffff;

    // sampling setting UI group
    const int MAX_SAMPLING_POS = 128; // Max. sampling tube positions
    const int LINE_GROUPS = 4; // Liquid sampling line group
    const int LINE_ITEMS = 5;  // Liquid sampling line record items
    const int ROW_COUNT = 32 + 1; // 1 seperator row
    const int COL_COUNT = 20 + 1; // 1 seperator column

    const int MAX_COLLECTION_POS = 16; // Max. collection vessel positions
    const int LINE_GROUPS_COLLECTION = 2; // Liquid collection line group
    const int LINE_ITEMS_COLLECTION = 5;  // Liquid collection line record items
    const int ROW_COUNT_COLLECTION = 16 + 1; // 1 seperator row
    const int COL_COUNT_COLLECTION = 10 + 1; // 1 seperator column

    const int COL_POS = 0;
    const int COL_CHANNEL = 1;
    const int COL_FLOW_LIMIT = 2;
    const int COL_SAMPLING_TIME = 3;
    const int COL_SOLVENT_TYPE = 4;

    // sampling runtime UI group
    std::vector<std::shared_ptr<SamplingUIItem>> sampling_ui_items;

    // runtime
    std::mutex mut;
    std::vector<std::thread> threads_;
    goiot::ThreadSafeQueue<std::shared_ptr<std::vector<RecipeTaskEntity>>> recipe_task_queue_;

    // video captures
    std::vector<cv::VideoCapture> vcaps_;
    std::vector<cv::Mat> vframes_;
    std::vector<QTimer> timers_;
    std::vector<QLabel*> image_labels_;
};

#endif // FORM_LIQUIDDISTRIBUTOR_H
