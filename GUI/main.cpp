#include "imageviewer.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ImageViewer l;
    l.show();

    return a.exec();
}
