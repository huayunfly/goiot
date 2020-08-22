#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "form_gasfeed.h"
#include "events.h"
#include "dialog_setvalue.h"
#include "dialog_setposition.h"

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
    QWidget* sender = static_cast<QWidget*>(this->sender());
    QApplication::sendEvent(ui->tabWidget->widget(0),
                new Ui::RefreshTextEvent("textEdit", Ui::ControlStatus::OK, "xxyy"));
    DialogSetValue set_value_dialog(sender, "34.5", MeasurementUnit::DEGREE);

    // convert the widget position to the screen position.
    QPoint screen_pos = this->mapToGlobal(sender->pos());
    screen_pos.setX(screen_pos.x() + 25);
    screen_pos.setY(screen_pos.y() + 10);
    set_value_dialog.move(screen_pos);
    int result = set_value_dialog.exec();
    if (result == QDialog::Accepted)
    {
        float f = set_value_dialog.NewValue();
        f++;
    }
    else
    {

    }
    // dialog_setposition
    DialogSetPosition set_position_dialog(sender, 8, 4);
    set_position_dialog.exec();

}
