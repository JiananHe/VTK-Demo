
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

	volumeColor = vtkSmartPointer<vtkColorTransferFunction>::New();
	volumeScalarOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();

	connect(ui->actionOpen_Folder, SIGNAL(triggered()), this, SLOT(onOpenFolderSlot()));
	connect(ui->actionSet_BgColor, SIGNAL(triggered()), this, SLOT(onSetBgColorSlot()));
	connect(ui->actionBoneStyle, SIGNAL(triggered()), this, SLOT(onSetBoneRender()));
	connect(ui->actionMuscleStyle, SIGNAL(triggered()), this, SLOT(onSetMuscleRender()));
	connect(ui->actionSkinStyle, SIGNAL(triggered()), this, SLOT(onSetSkinRender()));
	connect(ui->actionBone2Style, SIGNAL(triggered()), this, SLOT(onSetBone2Render()));
	connect(ui->actionBinaryStyle, SIGNAL(triggered()), this, SLOT(onSetBinaryRender()));
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

color_bp* Dicom_Volume_Render::getAllColorBPoints()
{
	try
	{
		if (color_bps.empty())
			throw "color transfer empty error";
		else
		{
			int n = color_bps.size();
			color_bp* color_bps_list = new color_bp[n];
			priority_queue<color_bp*> color_bps_temp;
			for (int i = 0; i < n; ++i)
			{
				color_bps_list[i] = *(color_bps.top());
				color_bps_temp.push(color_bps.top());
				color_bps.pop();
			}
			color_bps = color_bps_temp;
			return color_bps_list;
		}
	}
	catch (...)
	{
		cout << "color transfer empty error" << endl;
		abort();
	}
	return nullptr;
}

void Dicom_Volume_Render::drawColorTransfer()
{
	ui->min_gray_label->setText(QString::number(min_gray_value, 10, 2));
	ui->max_gray_label->setText(QString::number(max_gray_value, 10, 2));

	QPainter painter(ui->color_transfer_widget);
	painter.setRenderHint(QPainter::Antialiasing, true);

	int w = ui->color_transfer_widget->geometry().width();
	int h = ui->color_transfer_widget->geometry().height();
	int n = color_bps.size();
	int d = 11; //the diameter of cirlces in breakpoints

	//linear gradient color
	color_bp* color_bp_list = getAllColorBPoints();
	QLinearGradient linearGradient(d, h/2, w-d, h/2);
	double * point_list = new double[n];
	for (int i = 0; i < n; ++i)
	{
		color_bp temp = color_bp_list[i];
		double point = (temp.gray_value - min_gray_value) / (max_gray_value - min_gray_value);
		point_list[i] = point;
		linearGradient.setColorAt(point, QColor(temp.r, temp.g, temp.b));
	}
	painter.setBrush(QBrush(linearGradient));
	painter.drawRect(d, 0, w - 2 * d, h);

	//draw circle in break point
	
	for (int i = 0; i < n; ++i)
	{
		painter.setPen(QPen(Qt::gray, 5, Qt::SolidLine));
		//the first and second params is the left top coordiantes of ellipse
		painter.drawEllipse((w - 2 * d)*point_list[i] + d - d / 2, h / 2 - d / 2, d, d);
		painter.setPen(QPen(Qt::darkBlue, 4));
		painter.setBrush(Qt::NoBrush);
		painter.drawEllipse((w - 2 * d)*point_list[i] + d - d / 2 - 4, h / 2 - d / 2 - 4, d + 8, d + 8);
	}
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
	cout << max_gray_value << endl;
	cout << min_gray_value << endl;

	//Mapper
	vtkSmartPointer<vtkGPUVolumeRayCastMapper> volumeMapperGpu = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
	volumeMapperGpu->SetInputConnection(dicoms_reader->GetOutputPort());

	// The opacity transfer function is used to control the opacity of different tissue types.
	//bone
	volumeScalarOpacity->AddPoint(-3024, 0.00);
	volumeScalarOpacity->AddPoint(143.56, 0.00);
	volumeScalarOpacity->AddPoint(166.22, 0.69);
	volumeScalarOpacity->AddPoint(214.39, 0.70);
	volumeScalarOpacity->AddPoint(419.74, 0.83);
	volumeScalarOpacity->AddPoint(3071, 0.80);

	// The opacity transfer function is used to control the opacity of different tissue types.
	//bone
	int max_point = 3071 > max_gray_value ? max_gray_value : 3071;
	int min_point = -3024 < min_gray_value ? min_gray_value : -3024;
	volumeColor->AddRGBPoint(min_point, 0.0, 0.0, 0.0);
	volumeColor->AddRGBPoint(143.56, 157/255.0, 91/255.0, 47/255.0);
	volumeColor->AddRGBPoint(166.22, 1.0, 154/255.0, 74/255.0);
	volumeColor->AddRGBPoint(214.39, 1.0, 1.0, 1.0);
	volumeColor->AddRGBPoint(419.74, 1.0, 239/255.0, 244/255.0);
	volumeColor->AddRGBPoint(max_point, 211/255.0, 168/255.0, 255/255.0);

	color_bps.push(new color_bp(min_point, 0.0, 0.0, 0.0));
	color_bps.push(new color_bp(143.56, 157 / 255.0, 91 / 255.0, 47 / 255.00));
	color_bps.push(new color_bp(166.22, 1.0, 154 / 255.0, 74 / 255.0));
	color_bps.push(new color_bp(214.39, 1.0, 1.0, 1.0));
	color_bps.push(new color_bp(419.74, 1.0, 239 / 255.0, 244 / 255.0));
	color_bps.push(new color_bp(max_point, 211 / 255.0, 168 / 255.0, 255 / 255.0));


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
	vtkSmartPointer<vtkVolumeProperty> volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
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

	//change color transfer widget through event
	ui->color_transfer_widget->setVisible(true);
	ui->color_transfer_widget->repaint();

	ui->myQvtkWidget->GetRenderWindow()->Render();
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
		ui->myQvtkWidget->GetRenderWindow()->Render();
		ui->myQvtkWidget->update();
	}
}

void Dicom_Volume_Render::onSetBoneRender()
{
	volumeColor->RemoveAllPoints();
	volumeScalarOpacity->RemoveAllPoints();
	//bone
	volumeColor->AddRGBPoint(-3024, 0.0, 0.0, 0.0);
	volumeColor->AddRGBPoint(143.56, 157 / 255.0, 91 / 255.0, 47 / 255.0);
	volumeColor->AddRGBPoint(166.22, 1.0, 154 / 255.0, 74 / 255.0);
	volumeColor->AddRGBPoint(214.39, 1.0, 1.0, 1.0);
	volumeColor->AddRGBPoint(419.74, 1.0, 239 / 255.0, 244 / 255.0);
	volumeColor->AddRGBPoint(3071, 211 / 255.0, 168 / 255.0, 255 / 255.0);

	priority_queue<color_bp*> color_bps_temp;
	int max_point = 3071 > max_gray_value ? max_gray_value : 3071;
	int min_point = -3024 < min_gray_value ? min_gray_value : -3024;
	color_bps_temp.push(new color_bp(min_point, 0.0, 0.0, 0.0));
	color_bps_temp.push(new color_bp(143.56, 157 / 255.0, 91 / 255.0, 47 / 255.00));
	color_bps_temp.push(new color_bp(166.22, 1.0, 154 / 255.0, 74 / 255.0));
	color_bps_temp.push(new color_bp(214.39, 1.0, 1.0, 1.0));
	color_bps_temp.push(new color_bp(419.74, 1.0, 239 / 255.0, 244 / 255.0));
	color_bps_temp.push(new color_bp(max_point, 211 / 255.0, 168 / 255.0, 255 / 255.0));
	color_bps = color_bps_temp;

	volumeScalarOpacity->AddPoint(-3024, 0.00);
	volumeScalarOpacity->AddPoint(143.56, 0.00);
	volumeScalarOpacity->AddPoint(166.22, 0.69);
	volumeScalarOpacity->AddPoint(214.39, 0.70);
	volumeScalarOpacity->AddPoint(419.74, 0.83);
	volumeScalarOpacity->AddPoint(3071, 0.80);

	ui->myQvtkWidget->GetRenderWindow()->Render();
	ui->color_transfer_widget->repaint();
	ui->myQvtkWidget->update();
}

void Dicom_Volume_Render::onSetMuscleRender()
{
	volumeColor->RemoveAllPoints();
	volumeScalarOpacity->RemoveAllPoints();
	//muscle
	volumeColor->AddRGBPoint(-3024, 0, 0, 0, 0.5, 0.0);
	volumeColor->AddRGBPoint(-155, .55, .25, .15, 0.5, .92);
	volumeColor->AddRGBPoint(420, 1, .94, .95, 0.5, 0.0);
	volumeColor->AddRGBPoint(217, .88, .60, .29, 0.33, 0.45);
	volumeColor->AddRGBPoint(3071, .83, .66, 1, 0.5, 0.0);

	priority_queue<color_bp*> color_bps_temp;
	int max_point = 3071 > max_gray_value ? max_gray_value : 3071;
	int min_point = -3024 < min_gray_value ? min_gray_value : -3024;
	color_bps_temp.push(new color_bp(min_point, 0.0, 0.0, 0.0));
	color_bps_temp.push(new color_bp(-155, .55, .25, .15));
	color_bps_temp.push(new color_bp(420, 1, .94, .95));
	color_bps_temp.push(new color_bp(217, .88, .60, .29));
	color_bps_temp.push(new color_bp(max_point, .83, .66, 1));
	color_bps = color_bps_temp;

	volumeScalarOpacity->AddPoint(-3024, 0, 0.5, 0.0);
	volumeScalarOpacity->AddPoint(-155, 0, 0.5, 0.92);
	volumeScalarOpacity->AddPoint(217, .68, 0.33, 0.45);
	volumeScalarOpacity->AddPoint(420, .83, 0.5, 0.0);
	volumeScalarOpacity->AddPoint(3071, .80, 0.5, 0.0);

	ui->myQvtkWidget->GetRenderWindow()->Render();
	ui->color_transfer_widget->repaint();
	ui->myQvtkWidget->update();
}

void Dicom_Volume_Render::onSetSkinRender()
{
	volumeColor->RemoveAllPoints();
	volumeScalarOpacity->RemoveAllPoints();
	//skin
	volumeColor->AddRGBPoint(-3024, 0, 0, 0, 0.5, 0.0);
	volumeColor->AddRGBPoint(-1000, .62, .36, .18, 0.5, 0.0);
	volumeColor->AddRGBPoint(-500, .88, .60, .29, 0.33, 0.45);
	volumeColor->AddRGBPoint(3071, .83, .66, 1, 0.5, 0.0);

	priority_queue<color_bp*> color_bps_temp;
	int max_point = 3071 > max_gray_value ? max_gray_value : 3071;
	int min_point = -3024 < min_gray_value ? min_gray_value : -3024;
	color_bps_temp.push(new color_bp(min_point, 0.0, 0.0, 0.0));
	color_bps_temp.push(new color_bp(-1000, .62, .36, .18));
	color_bps_temp.push(new color_bp(-500, .88, .60, .29));
	color_bps_temp.push(new color_bp(max_point, .83, .66, 1));
	color_bps = color_bps_temp;

	volumeScalarOpacity->AddPoint(-3024, 0, 0.5, 0.0);
	volumeScalarOpacity->AddPoint(-1000, 0, 0.5, 0.0);
	volumeScalarOpacity->AddPoint(-500, 1.0, 0.33, 0.45);
	volumeScalarOpacity->AddPoint(3071, 1.0, 0.5, 0.0);

	ui->myQvtkWidget->GetRenderWindow()->Render();
	ui->color_transfer_widget->repaint();
	ui->myQvtkWidget->update();
}

void Dicom_Volume_Render::onSetBone2Render()
{
	volumeColor->RemoveAllPoints();
	volumeScalarOpacity->RemoveAllPoints();

	volumeColor->AddRGBPoint(-3024, 0, 0, 0, 0.5, 0.0);
	volumeColor->AddRGBPoint(-16, 0.73, 0.25, 0.30, 0.49, .61);
	volumeColor->AddRGBPoint(641, .90, .82, .56, .5, 0.0);
	volumeColor->AddRGBPoint(3071, 1, 1, 1, .5, 0.0);

	priority_queue<color_bp*> color_bps_temp;
	int max_point = 3071 > max_gray_value ? max_gray_value : 3071;
	int min_point = -3024 < min_gray_value ? min_gray_value : -3024;
	color_bps_temp.push(new color_bp(min_point, 0.0, 0.0, 0.0));
	color_bps_temp.push(new color_bp(-16, 0.73, 0.25, 0.30));
	color_bps_temp.push(new color_bp(641, .90, .82, .56));
	color_bps_temp.push(new color_bp(max_point, 1, 1, 1));
	color_bps = color_bps_temp;

	volumeScalarOpacity->AddPoint(-3024, 0, 0.5, 0.0);
	volumeScalarOpacity->AddPoint(-16, 0, .49, .61);
	volumeScalarOpacity->AddPoint(641, .72, .5, 0.0);
	volumeScalarOpacity->AddPoint(3071, .71, 0.5, 0.0);

	ui->myQvtkWidget->GetRenderWindow()->Render();
	ui->color_transfer_widget->repaint();
	ui->myQvtkWidget->update();
}

void Dicom_Volume_Render::onSetBinaryRender()
{
	volumeColor->RemoveAllPoints();
	volumeScalarOpacity->RemoveAllPoints();

	volumeColor->AddRGBPoint(-1024, 0.0, 0.0, 0.0);
	volumeColor->AddRGBPoint(-1013, 0.5, 0.5, 0.5);
	volumeColor->AddRGBPoint(-769, 1, 1, 1);

	priority_queue<color_bp*> color_bps_temp;
	int max_point = 3071 > max_gray_value ? max_gray_value : 3071;
	int min_point = -3024 < min_gray_value ? min_gray_value : -3024;
	color_bps_temp.push(new color_bp(min_point, 0.0, 0.0, 0.0));
	color_bps_temp.push(new color_bp(-1013, 0.5, 0.5, 0.5));
	color_bps_temp.push(new color_bp(max_point, .83, .66, 1));
	color_bps = color_bps_temp;

	volumeScalarOpacity->AddPoint(-1024, 0.0);
	volumeScalarOpacity->AddPoint(-1013, 1.0);
	volumeScalarOpacity->AddPoint(-769, 1.0);

	ui->myQvtkWidget->GetRenderWindow()->Render();
	ui->color_transfer_widget->repaint();
	ui->myQvtkWidget->update();
}
