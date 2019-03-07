#include "png_reader.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    png_reader w;
    w.show();

    return a.exec();
}
