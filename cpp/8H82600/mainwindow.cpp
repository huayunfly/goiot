#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "form_gasfeed.h"
#include "events.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QWidget* tab = new FormGasFeed();
    tab->setObjectName(QString::fromUtf8("tab"));
    ui->tabWidget->addTab(tab, QString());
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    QApplication::sendEvent(ui->tabWidget->widget(0),
                new Ui::RefreshEvent("textEdit", "xxyy", Ui::ControlStatus::OK));
}
