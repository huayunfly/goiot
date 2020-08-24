#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "data_model.h"
#include "data_manager.h"

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
    // Refresh UI callback registered in DataManager.
    void RefreshUi(std::shared_ptr<std::vector<goiot::DataInfo>> data_info_vec);

private:
    Ui::MainWindow *ui_;
    DataModel data_model_;
    goiot::DataManager data_manager_;
};
#endif // MAINWINDOW_H
