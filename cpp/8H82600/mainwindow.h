#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "data_model.h"
#include "data_manager.h"
#include "events.h"

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

private:
    void InitDataModel();
    /// Refresh UI callback registered in DataManager.
    void RefreshUi(std::shared_ptr<std::vector<goiot::DataInfo>> data_info_vec);

    /// <summary>
    /// Read data cache.
    /// </summary>
    /// <param name="parent_ui_name">Parent Ui name.</param>
    /// <param name="ui_name">Ui control name.</param>
    /// <param name="value">Value reference in string.</param>
    /// <param name="status">Control status.</param>
    /// <returns>True if it gets the data. otherwise false.</returns>
    bool ReadData(const QString& parent_ui_name, const QString& ui_name, QString& value, Ui::ControlStatus& status);

    /// <summary>
    /// Write data into the data_manager queue. It catches the queue exceptions.
    /// </summary>
    /// <param name="parent_ui_name">Parent Ui name.</param>
    /// <param name="ui_name">Ui control name.</param>
    /// <param name="value">Value reference in string.</param>
    void WriteData(const QString& parent_ui_name, const QString& ui_name, const QString& value);

private:
    Ui::MainWindow *ui_;
    DataModel data_model_;
    goiot::DataManager data_manager_;
};
#endif // MAINWINDOW_H
