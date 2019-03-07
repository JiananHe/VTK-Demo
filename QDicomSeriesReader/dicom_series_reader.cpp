#include "dicom_series_reader.h"
#include "ui_dicom_series_reader.h"
#include "myInteractorStyle.h"

vtkStandardNewMacro(myVtkInteractorStyleImage);

Dicom_Series_Reader::Dicom_Series_Reader(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Dicom_Series_Reader)
{

	img_viewer = vtkSmartPointer<vtkImageViewer2>::New();
	dicoms_reader = vtkSmartPointer<vtkDICOMImageReader>::New();
    ui->setupUi(this);
	//vtk_qt_connection = vtkEventQtSlotConnect::New();

	connect(ui->actionFile_2, SIGNAL(triggered()), this, SLOT(onOpenFileSlot()));
	connect(ui->actionFolder_2, SIGNAL(triggered()), this, SLOT(onOpenFolderSlot()));
	connect(ui->mySliderSlices, SIGNAL(valueChanged(int)), this, SLOT(onSlideMove(int)));

}

void Dicom_Series_Reader::onOpenFileSlot()
{
	vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New();

	QString filter;
	filter = "DCM image file (*.dcm)";

	QDir dir;
	QString fileName = QFileDialog::getOpenFileName(this, QString(tr("Open DCM Image")), dir.absolutePath(), filter);
	if (fileName.isEmpty()) return;

	// 支持带中文路径的读取  
	QByteArray ba = fileName.toLocal8Bit();
	const char *fileName_str = ba.data();

	vtkSmartPointer<vtkDICOMImageReader> dim_reader = vtkSmartPointer<vtkDICOMImageReader>::New();
	dim_reader->SetFileName(fileName_str);

	img_viewer->SetInputConnection(dim_reader->GetOutputPort());
	img_viewer->SetRenderer(ren);
	img_viewer->SetRenderWindow(ui->myQVTKWidget->GetRenderWindow());

	img_viewer->SetSliceOrientationToXY();
	img_viewer->GetImageActor()->InterpolateOff();

	ren->ResetCamera();
	ren->DrawOn();

	ui->myQVTKWidget->GetRenderWindow()->Render();

}

void Dicom_Series_Reader::onOpenFolderSlot()
{
	QString folder_path = QFileDialog::getExistingDirectory(this, tr("Open DICOM Folder"), QDir::currentPath(), QFileDialog::ShowDirsOnly);
	// 支持带中文路径的读取  
	QByteArray ba = folder_path.toLocal8Bit();
	const char *folderName_str = ba.data();
	std::cout << "Open floder: " << folderName_str << std::endl;
	drawDicomSeries(folderName_str);
}

void Dicom_Series_Reader::drawDicomSeries(const char* folder_path)
{
	// Read all the DICOM files in the specified directory.
	dicoms_reader->SetDirectoryName(folder_path);
	dicoms_reader->Update();

	img_viewer->SetInputConnection(dicoms_reader->GetOutputPort());

	ui->myQVTKWidget->SetRenderWindow(img_viewer->GetRenderWindow());

	img_viewer->SetupInteractor(ui->myQVTKWidget->GetInteractor());

	// set my interactor style
	vtkSmartPointer<myVtkInteractorStyleImage> myInteractorStyle = vtkSmartPointer<myVtkInteractorStyleImage>::New();
	myInteractorStyle->SetImageViewer(img_viewer);
	myInteractorStyle->SetSliderSlices(ui->mySliderSlices, ui->label_min_slice, ui->label_max_slice);
	ui->myQVTKWidget->GetInteractor()->SetInteractorStyle(myInteractorStyle);
	ui->myQVTKWidget->GetInteractor()->Initialize();


	/*img_viewer->SetSliceOrientationToXY();
	img_viewer->GetImageActor()->InterpolateOff();*/

	img_viewer->Render();
	img_viewer->GetRenderer()->ResetCamera();
	img_viewer->Render();
	ui->myQVTKWidget->GetInteractor()->Start();
}

void Dicom_Series_Reader::onSlideMove(int pos)
{
	std::cout << "slice: " << pos << std::endl;
	this->img_viewer->SetSlice(pos);
	this->img_viewer->Render();
}


Dicom_Series_Reader::~Dicom_Series_Reader()
{
    delete ui;
}
