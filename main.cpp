#include "beyerfilter.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    BeyerFilter w;
    w.show();
    return a.exec();
}
