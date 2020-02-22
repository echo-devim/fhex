#include "fhex.h"
#include <QFile>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString filepath = "";
    if ((argc > 1) && (QFile::exists(argv[1]))) {
        filepath = argv[1];
    }
    Fhex w(nullptr, &a, filepath);

    w.show();

    return a.exec();
}
