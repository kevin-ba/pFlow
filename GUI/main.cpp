#include "outlineFlow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    OutlineFlow l;
    l.show();

    return a.exec();
}
