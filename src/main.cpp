#include "fhex.h"
#include <QFile>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString filepath = "";

    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            filepath = argv[i];
            if (QFile::exists(filepath)) {
                Fhex *window = new Fhex(nullptr, &a, filepath);
                window->show();
            } else {
                cout << "Error: Cannot load file '" << filepath.toStdString() << "'" << endl;
                return 1;
            }
        }
    } else {
        Fhex *window = new Fhex(nullptr, &a);
        window->show();
    }

    return a.exec();
}
