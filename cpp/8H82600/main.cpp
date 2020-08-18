#include "mainwindow.h"

#include <QApplication>
#include "qredisclient/include/redisclient.h"
#include "qredisclient/include/asyncfuture.h"
#include "data_manager.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    goiot::DataManager data_manager(a.applicationDirPath().toStdString());
    data_manager.LoadJsonConfig();
    data_manager.Start();
    return a.exec();
}
