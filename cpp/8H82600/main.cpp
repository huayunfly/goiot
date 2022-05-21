#include "mainwindow.h"
#include "log.h"

#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    logger::FileLog log("log");
    MainWindow w;
    w.show();
    return a.exec();
}
