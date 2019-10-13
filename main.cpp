#include "fhex.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Fhex w;

    w.show();

    return a.exec();
}
