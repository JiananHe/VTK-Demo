#ifndef DICOM_VOLUME_RENDER_H
#define DICOM_VOLUME_RENDER_H

#include <QMainWindow>
#include <QFileDialog>  
#include <QDir>
#include <vtkEventQtSlotConnect.h>
#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkNamedColors.h>
#include <vtkDICOMImageReader.h>
#include <vtkImageData.h>
#include <vtkImageAccumulate.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkFixedPointVolumeRayCastMapper.h>
#include <vtkVolumeProperty.h>
#include <vtkVolume.h>
#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>

#include "ui_dicom_volume_render.h"
#include <qcolordialog.h>

namespace Ui {
class Dicom_Volume_Render;
}

class Dicom_Volume_Render : public QMainWindow
{
    Q_OBJECT

public:
    explicit Dicom_Volume_Render(QWidget *parent = nullptr);
    ~Dicom_Volume_Render();

private:
    Ui::Dicom_Volume_Render *ui;
	vtkSmartPointer<vtkDICOMImageReader> dicoms_reader;
	vtkSmartPointer<vtkRenderer> volume_render;
	vtkSmartPointer< vtkRenderWindow> renwin;

private slots:
	void onOpenFolderSlot();
	void onSetBgColorSlot();
};

#endif // DICOM_VOLUME_RENDER_H
