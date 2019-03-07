#ifndef DICOM_SERIES_READER_H
#define DICOM_SERIES_READER_H

#include <QMainWindow>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkEventQtSlotConnect.h>
#include <vtkSmartPointer.h>
#include <vtkDICOMImageReader.h>
#include <vtkImageViewer2.h>
#include <vtkImageActor.h>  
#include <QFileDialog>  
#include <QDir> 
#include "ui_dicom_series_reader.h"

class vtkEventQtSlotConnect;
class vtkRenderer;

namespace Ui {
class Dicom_Series_Reader;
}

class Dicom_Series_Reader : public QMainWindow
{
    Q_OBJECT

public:
    explicit Dicom_Series_Reader(QWidget *parent = nullptr);
    ~Dicom_Series_Reader();

private:
    Ui::Dicom_Series_Reader *ui;
	vtkSmartPointer<vtkImageViewer2> img_viewer;
	vtkSmartPointer<vtkDICOMImageReader> dicoms_reader;
	//vtkEventQtSlotConnect* vtk_qt_connection;
	void drawDicomSeries(const char*);

private slots:
	void onOpenFileSlot();
	void onOpenFolderSlot();
	void onSlideMove(int);
};

#endif // DICOM_SERIES_READER_H
