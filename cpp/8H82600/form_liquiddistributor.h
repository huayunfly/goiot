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
#include <atomic>
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

struct RecipeUITableSetting
{
    RecipeUITableSetting(int rows, int cols, int ch_groups, int ln_items, int pos_num):
        row_count(rows), col_count(cols),
        channel_groups(ch_groups), line_items(ln_items), work_positions(pos_num)
    {
    }
    int row_count;
    int col_count;
    int channel_groups; // Liquid sampling line group
    int line_items;
    int work_positions;
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

    void mouseDoubleClickEvent(QMouseEvent *event) override;

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    // Detect the image contoures
    void UpdateImage(int index);

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

private:
    //(Deprecated) Save the procedure parameter to a record string.
    QString SaveLiquidSamplingProcedure();
    //(Deprecated) Load procedure from a record string.
    void LoadLiquidSamplingProcedure(const QString& record);
    //(Deprecated) Unpack the record string to the parameter list.
    std::list<std::vector<int>> SamplingRecordToList(const QString& record);

    // Get the setting for the recipe table
    RecipeUITableSetting GetRecipeUITableSetting();

    // Initialize video captrues.
    void InitVideoCaps();

    // Initialize recipe runtime view.
    void InitRecipeRuntimeView();

    // Initialize recipe runtime view, with circles representing the sampling status.
    // @param <x_gap>: circle gap in x.
    // @param <y_gap>: circle gap in y.
    // @param <radius>: circle radius.
    // @Param <x_count>: circle count in x.
    // @param <y_count>: circle count in y.
    // @param <y_section>: circle section for sink positions in y.
    void InitRecipeRuntimeView(int x_gap, int y_gap,
                               double radius, int x_count, int y_count, int y_section);

    // Set the recipe runtime item to unsigned states.
    void ClearRecipeRuntimeView();

    // Initialize recipe setting table.
    void InitRecipeSettingTable();

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

    // Initialize log window document's max lines.
    void InitLogWindow();

    // Show menu.
    void LogWindowShowMenu();

    // Enable/disable the setting table edit mode.
    // Call after InitRecipeSettingTable()
    // @param <enable>: editable
    void EnableRecipeSettingTable(bool enable);

    // Recipe setting table's channel selection changed.
    // Display channel No. on the runtime view.
    // @param <row>: QTable row
    // @param <col>: QTable column
    void SelectChannelChanged(const QString& text);

    // Save the liquid sampling or collection recipe to DB.
    bool SaveRecipe(const QString& recipe_name);

    // Load the liquid sampling or collection recipe from DB.
    bool LoadRecipe(const QString& recipe_name);

    // Delete recipe from DB.
    bool DeleteRecipe(const QString& recipe_name);

    // Read a recipe from DB and dispatch recipe task to queue.
    bool DispatchRecipeTask(const QString& recipe_name);

    // Run recipe backend thread.
    void RunRecipeWorker();

    // Update the runtime view while running the recipe.
    // Call by RunRecipeWorker();
    void UpdateRuntimeView(const RecipeTaskEntity& entity, bool run_a, bool run_b,
                            SamplingUIItem::SamplingUIItemStatus sampling_status,
                            SamplingUIItem::SamplingUIItemStatus purge_status);

    // Send PLC command to stop taking liquid.
    bool StopTakingLiquidCmd(StatusCheckGroup group);

    // Liquid sampling status check task A, using timeout
    // The thread will exit if the recipe does not run.
    qint64 SamplingStatusCheckByTime(StatusCheckGroup group, int timeout_secs);

    // Liquid sampling status check task B, using liquid image recognition.
    // The thread will exit if the recipe does not run.
    qint64 SamplingStatusCheckByImageDetection(StatusCheckGroup group, int timeout_sec);

    // Detect liquid level in image. Return true if it succeeded.
    bool DetectImage(int index);

    // Clear the UI setting table by category
    void ClearUITable();

    // Fill channel information in the liquid sampling UI table.
    void FillUITableChannelInfo(int pos, int channel, int flowlimit,
                                int duration, int cleanport);

    // Log to text window.
    void Log2Window(const QString& recipe_name, const QString& status);

    // Log task running step by calling Log2Window.
    void LogTaskStep(const RecipeTaskEntity& entity);

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

    // Read recipe name list from table by 'sampling' or 'collection'
    bool ReadRecipeNamesFromDB(std::vector<QString>& recipe_names, QString& error_message);

    // Delete recipe
    bool DeleteRecipeFromDB(const QString& tablename, const QString& recipe_name,
                            QString& error_message);

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

    // Loaded
    QString loaded_recipe_name_;

    // sampling runtime UI group
    std::vector<std::shared_ptr<SamplingUIItem>> sampling_ui_items;

    // runtime
    std::mutex mut_;
    std::condition_variable task_run_cond_;
    std::vector<std::thread> threads_;
    goiot::ThreadSafeQueue<std::shared_ptr<std::vector<RecipeTaskEntity>>> recipe_task_queue_;
    std::atomic<bool> task_running_; // recipe run status
    std::atomic<bool> dist_a_run_; // PLC feedback a group
    std::atomic<bool> dist_b_run_; // PLC feedback b group

    // video captures
    std::vector<cv::VideoCapture> vcaps_;
    std::vector<cv::Mat> vframes_;
    std::vector<QTimer> timers_;
    std::vector<QLabel*> image_labels_;
    void LogTaskStep();

    // UI
    std::shared_ptr<QMenu> log_menu;
};

#endif // FORM_LIQUIDDISTRIBUTOR_H
