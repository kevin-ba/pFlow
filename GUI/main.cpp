#include "mainwindow.h"
#include "imageviewer.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
//    ImageViewer l;
//    l.show();

    return a.exec();
}
