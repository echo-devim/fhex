#include "fhex.h"
#include <QFile>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString filepath = "";

    if (argc > 1) {
        for (int i = 0; i < argc; i++) {
            if (QFile::exists(argv[i])) {
                filepath = argv[i];
                Fhex *window = new Fhex(nullptr, &a, filepath);
                window->show();
            }
        }
    } else {
        Fhex *window = new Fhex(nullptr, &a);
        window->show();
    }

    return a.exec();
}
