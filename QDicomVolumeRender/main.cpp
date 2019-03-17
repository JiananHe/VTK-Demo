#include "dicom_volume_render.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Dicom_Volume_Render w;
    w.show();

    return a.exec();
}
