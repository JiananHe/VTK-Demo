
#include "dicom_volume_render.h"
#include "ui_dicom_volume_render.h"

Dicom_Volume_Render::Dicom_Volume_Render(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Dicom_Volume_Render)
{
    ui->setupUi(this);
	ui->color_transfer_widget->installEventFilter(this);
	ui->color_transfer_widget->setVisible(false);

	dicoms_reader = vtkSmartPointer<vtkDICOMImageReader>::New();
	volume_render = vtkSmartPointer<vtkRenderer>::New();

	connect(ui->actionOpen_Folder, SIGNAL(triggered()), this, SLOT(onOpenFolderSlot()));
	connect(ui->actionSet_BgColor, SIGNAL(triggered()), this, SLOT(onSetBgColorSlot()));
}

Dicom_Volume_Render::~Dicom_Volume_Render()
{
    delete ui;
}

bool Dicom_Volume_Render::eventFilter(QObject *watched, QEvent *event)
{
	if (watched == ui->color_transfer_widget)
	{
		if (event->type() == QEvent::Paint)
		{
			drawColorTransfer();
		}
		else if (event->type() == QEvent::MouseButtonRelease)
		{
			QColor init_color = QColor(Qt::blue);
			QColor color = QColorDialog::getColor(init_color, this, "select color");
			if (color.isValid())
			{
				
			}

		}
	}

	return QMainWindow::eventFilter(watched, event);
}

void Dicom_Volume_Render::drawColorTransfer()
{
	ui->min_gray_label->setText(QString::number(min_gray_value, 10, 2));
	ui->max_gray_label->setText(QString::number(max_gray_value, 10, 2));

	QPainter painter(ui->color_transfer_widget);
	painter.setRenderHint(QPainter::Antialiasing, true);
	int w = ui->color_transfer_widget->geometry().width();
	int h = ui->color_transfer_widget->geometry().height();

	//linear gradient color
	QLinearGradient linearGradient(0, h/2, w, h/2);

	linearGradient.setColorAt(0, Qt::black);
	linearGradient.setColorAt(1, Qt::white);
	painter.setBrush(QBrush(linearGradient));
	painter.drawRect(0, 0, w, h);

	//draw circle in break point with corresponding color
}

void Dicom_Volume_Render::onOpenFolderSlot()
{
	QString filter;
	filter = "DCM image file (*.dcm)";

	// read from folder
	QString folder_path = QFileDialog::getExistingDirectory(this, tr("Open DICOM Folder"), "C:\\Users\\13249\\Documents\\VTK_Related\\dataset", QFileDialog::ShowDirsOnly);
	QByteArray ba = folder_path.toLocal8Bit();
	const char *folderName_str = ba.data();
	cout << "Open floder: " << folderName_str << std::endl;

	//set backgroud color blue
	vtkSmartPointer<vtkNamedColors> colors = vtkSmartPointer<vtkNamedColors>::New();
	colors->SetColor("bgColor_blue", 0.0, 0.0, 1.0);
	volume_render->SetBackground(colors->GetColor3d("bgColor_blue").GetData());

	//reader
	dicoms_reader->SetDirectoryName(folderName_str);
	dicoms_reader->Update();

	int imageDims[3];
	dicoms_reader->GetOutput()->GetDimensions(imageDims); //need include <vtkimagedata.h>
	cout << "dimension[] :" << imageDims[0] << " " << imageDims[1] << " " << imageDims[2] << endl;

	const char* patient_name = dicoms_reader->GetPatientName();
	const char* study_id = dicoms_reader->GetStudyID();
	cout << "patient name: " << patient_name << endl;
	cout << "study id: " << study_id << endl;

	//get pixel
	double* ps = dicoms_reader->GetPixelSpacing();
	cout << ps[0] << " " << ps[1] << " " << ps[2] << endl;

	vtkSmartPointer<vtkImageAccumulate> histogram = vtkSmartPointer<vtkImageAccumulate>::New();
	histogram->SetInputData(dicoms_reader->GetOutput());
	histogram->Update();
	max_gray_value = *(histogram->GetMax());
	min_gray_value = *(histogram->GetMin());
	color_bps.push(color_breakpoint(max_gray_value, 1.0, 1.0, 1.0));
	color_bps.push(color_breakpoint(min_gray_value, 0.0, 0.0, 0.0));
	cout << max_gray_value << endl;
	cout << min_gray_value << endl;

	//change color transfer widget through event
	ui->color_transfer_widget->setVisible(true);
	ui->color_transfer_widget->repaint();


	//Mapper
	vtkSmartPointer<vtkGPUVolumeRayCastMapper> volumeMapperGpu = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
	volumeMapperGpu->SetInputConnection(dicoms_reader->GetOutputPort());

	// The opacity transfer function is used to control the opacity
	// of different tissue types.
	vtkSmartPointer<vtkPiecewiseFunction> volumeScalarOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
	volumeScalarOpacity->AddPoint(-3024, 0.00);
	volumeScalarOpacity->AddPoint(143.56, 0.00);
	volumeScalarOpacity->AddPoint(166.22, 0.69);
	volumeScalarOpacity->AddPoint(214.39, 0.70);
	volumeScalarOpacity->AddPoint(419.74, 0.83);
	volumeScalarOpacity->AddPoint(3071, 0.80);

	// The opacity transfer function is used to control the opacity
	// of different tissue types.
	vtkSmartPointer<vtkColorTransferFunction> volumeColor = vtkSmartPointer<vtkColorTransferFunction>::New();
	volumeColor->AddRGBPoint(-3024, 0.0, 0.0, 0.0);
	volumeColor->AddRGBPoint(143.56, 157/255.0, 91/255.0, 47/255.0);
	volumeColor->AddRGBPoint(166.22, 1.0, 154/255.0, 74/255.0);
	volumeColor->AddRGBPoint(214.39, 1.0, 1.0, 1.0);
	volumeColor->AddRGBPoint(419.74, 1.0, 239/255.0, 244/255.0);
	volumeColor->AddRGBPoint(3071, 211/255.0, 168/255.0, 255/255.0);

	// The gradient opacity function is used to decrease the opacity
	// in the "flat" regions of the volume while maintaining the opacity
	 // at the boundaries between tissue types.  The gradient is measured
	// as the amount by which the intensity changes over unit distance.
	// For most medical data, the unit distance is 1mm.
	/*vtkSmartPointer<vtkPiecewiseFunction> volumeGradientOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
	volumeGradientOpacity->AddPoint(0, 0.0);
	volumeGradientOpacity->AddPoint(90, 0.5);
	volumeGradientOpacity->AddPoint(100, 1.0);*/

	//vtkVolumeProperty
	vtkSmartPointer<vtkVolumeProperty> volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();//定义对象属性
	volumeProperty->SetInterpolationTypeToLinear();
	volumeProperty->ShadeOn();
	volumeProperty->SetAmbient(0.1);
	volumeProperty->SetDiffuse(0.9);
	volumeProperty->SetSpecular(0.2);
	volumeProperty->SetSpecularPower(10.0);
	volumeProperty->SetColor(volumeColor);
	volumeProperty->SetScalarOpacity(volumeScalarOpacity);
	//volumeProperty->SetGradientOpacity(volumeGradientOpacity);

	// The vtkVolume is a vtkProp3D (like a vtkActor) and controls the position
	// and orientation of the volume in world coordinates.
	vtkSmartPointer<vtkVolume> volume = vtkSmartPointer<vtkVolume>::New();
	//volume->SetMapper(volumeMapper);
	volume->SetMapper(volumeMapperGpu);
	volume->SetProperty(volumeProperty);

	volume_render->AddViewProp(volume);
	ui->myQvtkWidget->GetRenderWindow()->AddRenderer(volume_render);

	ui->myQvtkWidget->GetRenderWindow()->Render();
	//qApp->processEvents();
	//ui->myQvtkWidget->GetInteractor()->Start();
}

color_breakpoint * Dicom_Volume_Render::getAllColorBPoints()
{

	return nullptr;
}

void Dicom_Volume_Render::onSetBgColorSlot()
{

	QColor init_color = QColor(Qt::blue);
	QColor color = QColorDialog::getColor(init_color, this, "select background color");

	if (color.isValid())
	{
		vtkSmartPointer<vtkNamedColors> colors = vtkSmartPointer<vtkNamedColors>::New();
		colors->SetColor("bgColor_yellow", color.red()/255.0, color.green()/255.0, color.blue()/255.0);
		volume_render->SetBackground(colors->GetColor3d("bgColor_yellow").GetData());
		//volume_render->Render();
		ui->myQvtkWidget->GetRenderWindow()->Render();
		//qApp->processEvents();
	}
}