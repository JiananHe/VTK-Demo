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

struct color_breakpoint
{
	double gray_value, r, g, b;
	//color_breakpoint() {};
	color_breakpoint(double gv, double r, double g, double b)
	{
		this->gray_value = gv;
		this->r = r;
		this->g = g;
		this->b = b;
	}
	bool operator<(const color_breakpoint a) const
	{
		return gray_value < a.gray_value;
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

	void drawColorTransfer();

private:
    Ui::Dicom_Volume_Render *ui;
	vtkSmartPointer<vtkDICOMImageReader> dicoms_reader;
	vtkSmartPointer<vtkRenderer> volume_render;
	vtkSmartPointer< vtkRenderWindow> renwin;

private:
	priority_queue<color_breakpoint> color_bps;
	double min_gray_value;
	double max_gray_value;

public:
	color_breakpoint* getAllColorBPoints();

private slots:
	void onOpenFolderSlot();
	void onSetBgColorSlot();
	bool eventFilter(QObject *, QEvent *);
};

#endif // DICOM_VOLUME_RENDER_H
