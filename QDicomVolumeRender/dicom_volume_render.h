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
#include <QPainter>
#include <QMouseEvent>

#include <queue>
using namespace std;

struct color_bp
{
	double gray_value;
	int r, g, b;
	color_bp() {};
	color_bp(double gv, double r, double g, double b)
	{
		this->gray_value = gv;
		this->r = int(r*255 + 0.5);
		this->g = int(g*255 + 0.5);
		this->b = int(b*255 + 0.5);
	}
	bool operator<(const color_bp* a) const
	{
		return gray_value < a->gray_value;
	}
};

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

	vtkSmartPointer<vtkColorTransferFunction> volumeColor;
	vtkSmartPointer<vtkPiecewiseFunction> volumeScalarOpacity;

private:
	priority_queue<color_bp*> color_bps;
	double min_gray_value;
	double max_gray_value;

	void drawColorTransfer();

public:
	color_bp* getAllColorBPoints();	

private slots:
	void onOpenFolderSlot();
	void onSetBgColorSlot();

	void onSetBoneRender();
	void onSetMuscleRender();
	void onSetSkinRender();
	void onSetBone2Render();
	void onSetBinaryRender();

	bool eventFilter(QObject *, QEvent *);
};

#endif // DICOM_VOLUME_RENDER_H
