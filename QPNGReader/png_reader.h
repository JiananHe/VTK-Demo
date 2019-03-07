#ifndef PNG_READER_H
#define PNG_READER_H

#include <QMainWindow>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkEventQtSlotConnect.h>
#include <vtkSmartPointer.h>
#include <vtkPNGReader.h>
#include <vtkImageViewer2.h>
#include <vtkImageActor.h>  
#include <QFileDialog>  
#include <QDir> 
#include "ui_png_reader.h"

class vtkEventQtSlotConnect;
class vtkRenderer;

namespace Ui {
class png_reader;
}

class png_reader : public QMainWindow
{
    Q_OBJECT

public:
    explicit png_reader(QWidget *parent = nullptr);
    ~png_reader();

private:
    Ui::png_reader *ui;
	vtkEventQtSlotConnect* vtk_qt_connection;
	//vtkSmartPointer<vtkRenderer> ren;
	//vtkSmartPointer<vtkImageViewer2> img_viewer;

private slots:
	void updateCoords(vtkObject*);
private slots:
	void onOpenSlot();
};

#endif // PNG_READER_H
