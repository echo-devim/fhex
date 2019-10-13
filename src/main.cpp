#include "fhex.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Fhex w(nullptr, &a);

    w.show();

    return a.exec();
}
