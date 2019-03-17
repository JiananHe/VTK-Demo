
#include "dicom_volume_render.h"
#include "ui_dicom_volume_render.h"

Dicom_Volume_Render::Dicom_Volume_Render(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Dicom_Volume_Render)
{
    ui->setupUi(this);
	dicoms_reader = vtkSmartPointer<vtkDICOMImageReader>::New();
	volume_render = vtkSmartPointer<vtkRenderer>::New();
	renwin = vtkSmartPointer<vtkRenderWindow>::New();
	connect(ui->actionOpen_Folder, SIGNAL(triggered()), this, SLOT(onOpenFolderSlot()));
}

Dicom_Volume_Render::~Dicom_Volume_Render()
{
    delete ui;
}

void Dicom_Volume_Render::onOpenFolderSlot()
{
	QString filter;
	filter = "DCM image file (*.dcm)";

	QString folder_path = QFileDialog::getExistingDirectory(this, tr("Open DICOM Folder"), QDir::currentPath(), QFileDialog::ShowDirsOnly);
	// 支持带中文路径的读取  
	QByteArray ba = folder_path.toLocal8Bit();
	const char *folderName_str = ba.data();
	std::cout << "Open floder: " << folderName_str << std::endl;

	//Render
	dicoms_reader->SetDirectoryName(folderName_str);
	dicoms_reader->Update();

	int imageDims[3];
	dicoms_reader->GetOutput()->GetDimensions(imageDims); //need include <vtkimagedata.h>
	cout << "dimension[] :" << imageDims[0] << " " << imageDims[1] << " " << imageDims[2] << endl;

	//Mapper
	vtkSmartPointer<vtkGPUVolumeRayCastMapper> volumeMapperGpu = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
	volumeMapperGpu->SetInputConnection(dicoms_reader->GetOutputPort());

	// The opacity transfer function is used to control the opacity
	// of different tissue types.
	vtkSmartPointer<vtkPiecewiseFunction> volumeScalarOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
	volumeScalarOpacity->AddPoint(0, 0.00);
	volumeScalarOpacity->AddPoint(500, 0.15);
	volumeScalarOpacity->AddPoint(1000, 0.15);
	volumeScalarOpacity->AddPoint(1150, 0.85);

	// The opacity transfer function is used to control the opacity
	// of different tissue types.
	vtkSmartPointer<vtkColorTransferFunction> volumeColor = vtkSmartPointer<vtkColorTransferFunction>::New();
	volumeColor->AddRGBPoint(0, 0.0, 0.0, 0.0);
	volumeColor->AddRGBPoint(64, 1, 0.52, 0.30);
	volumeColor->AddRGBPoint(190.0, 1, 1, 1);
	volumeColor->AddRGBPoint(220, 0.2, 0.2, 0.2);

	// The gradient opacity function is used to decrease the opacity
	// in the "flat" regions of the volume while maintaining the opacity
	 // at the boundaries between tissue types.  The gradient is measured
	// as the amount by which the intensity changes over unit distance.
	// For most medical data, the unit distance is 1mm.
	vtkSmartPointer<vtkPiecewiseFunction> volumeGradientOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
	volumeGradientOpacity->AddPoint(0, 0.0);
	volumeGradientOpacity->AddPoint(90, 0.5);
	volumeGradientOpacity->AddPoint(100, 1.0);

	//vtkVolumeProperty
	vtkSmartPointer<vtkVolumeProperty> volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();//定义对象属性
	volumeProperty->SetInterpolationTypeToLinear();
	volumeProperty->ShadeOn();
	volumeProperty->SetAmbient(0.4);
	volumeProperty->SetDiffuse(0.6);
	volumeProperty->SetSpecular(0.2);
	volumeProperty->SetColor(volumeColor);
	volumeProperty->SetScalarOpacity(volumeScalarOpacity);
	volumeProperty->SetGradientOpacity(volumeGradientOpacity);

	// The vtkVolume is a vtkProp3D (like a vtkActor) and controls the position
	// and orientation of the volume in world coordinates.
	vtkSmartPointer<vtkVolume> volume = vtkSmartPointer<vtkVolume>::New();
	//volume->SetMapper(volumeMapper);
	volume->SetMapper(volumeMapperGpu);
	volume->SetProperty(volumeProperty);

	volume_render->AddViewProp(volume);
	renwin->AddRenderer(volume_render);

	ui->myQvtkWidget->SetRenderWindow(renwin);
	volume_render->Render();
	ui->myQvtkWidget->GetInteractor()->Start();
}