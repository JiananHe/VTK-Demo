#include "dicom_series_reader.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Dicom_Series_Reader w;
    w.show();

    return a.exec();
}
