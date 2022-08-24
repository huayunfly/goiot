#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItem>
#include "data_model.h"
#include "data_manager.h"
#include "events.h"
#include "safety_policy.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_listView_clicked(const QModelIndex &index);

private:
    void InitDataModel();

    /// <summary>
    /// Refresh UI callback registered in DataManager.
    /// Updatings include numeric conversion: value * data_info.ratio, if ratio is not 1.0.
    /// </summary>
    /// <param name="data_info_vec">DataInfo vector.</param>
    void RefreshUi(std::shared_ptr<std::vector<goiot::DataInfo>> data_info_vec);

    /// <summary>
    /// Read data cache.
    /// Readings include numeric conversion: value * data_info.ratio, if ratio is not 1.0.
    /// </summary>
    /// <param name="parent_ui_name">Parent Ui name.</param>
    /// <param name="ui_name">Ui control name.</param>
    /// <param name="value">Value reference in string.</param>
    /// <param name="status">Control status.</param>
    /// <param name="ui_info">UiInfo</param>
    /// <returns>True if it gets the data. otherwise false.</returns>
    bool ReadData(const QString& parent_ui_name, const QString& ui_name, QString& value, Ui::ControlStatus& status, UiInfo& ui_info);

public:
    /// <summary>
    /// Write data into the data_manager queue. It catches the queue exceptions.
    /// Writings include numeric conversion: set value / data_info.ratio, if ratio is not 1.0.
    /// </summary>
    /// <param name="parent_ui_name">Parent Ui name.</param>
    /// <param name="ui_name">Ui control name.</param>
    /// <param name="value">Value reference in string.</param>
    /// <returns>True if the writing data is put in the output queue</returns>
    bool WriteData(const QString& parent_ui_name, const QString& ui_name, const QString& value);

private:
    Ui::MainWindow *ui_;
    DataModel data_model_;
    goiot::DataManager data_manager_;
    SafetyPolicy safety_;
};
#endif // MAINWINDOW_H
