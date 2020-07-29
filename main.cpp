#include "beyerfilter.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    BayerFilter w;
    w.show();
    return a.exec();
}
