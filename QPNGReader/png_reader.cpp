#include "png_reader.h"
#include "ui_png_reader.h"

png_reader::png_reader(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::png_reader)
{
    ui->setupUi(this);

	vtk_qt_connection = vtkEventQtSlotConnect::New();
	//ren = vtkSmartPointer<vtkRenderer>::New();
	//img_viewer = vtkSmartPointer<vtkImageViewer2>::New();

	//为qvtk控件添加render
	//ui->myQVTKWidget->GetRenderWindow()->AddRenderer(ren);

	//将qvtk控件的iteractor的信号(MouseMoveEvent),与槽(updateCoords操作)连接起来
	vtk_qt_connection->Connect(ui->myQVTKWidget->GetRenderWindow()->GetInteractor(), 
		vtkCommand::MouseMoveEvent, this, SLOT(updateCoords(vtkObject*)));

	//将actionOpen的信号(triggered)，与槽(onOpenSlot())连接起来
	connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(onOpenSlot()));
}

void png_reader::updateCoords(vtkObject* obj)
{
	//SafeDownCast用于基类向子类的类型转换，称为向下转型
	vtkRenderWindowInteractor* iren = vtkRenderWindowInteractor::SafeDownCast(obj);
	int event_pos[2];
	iren->GetEventPosition(event_pos);

	QString coords_str;
	coords_str.sprintf("x=%d, y=%d", event_pos[0], event_pos[1]);
	ui->myLabel->setText(coords_str);
}

void png_reader::onOpenSlot()
{
	vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkImageViewer2> img_viewer = vtkSmartPointer<vtkImageViewer2>::New();
	QString filter;
	filter = "PNG image file (*.png)";

	QDir dir;
	QString fileName = QFileDialog::getOpenFileName(this, QString(tr("Open PNG Image")), dir.absolutePath(), filter);
	if (fileName.isEmpty()) return;

	// 支持带中文路径的读取  
	QByteArray ba = fileName.toLocal8Bit();
	const char *fileName_str = ba.data();

	vtkSmartPointer<vtkPNGReader> png_reader = vtkSmartPointer<vtkPNGReader>::New();
	png_reader->SetFileName(fileName_str);

	// 将reader的输出作为img_viewer的输入，并设置img_viewer与myQVTKWidget的关联(RenderWindow, Interactor)
	img_viewer->SetInputConnection(png_reader->GetOutputPort());
	img_viewer->SetRenderer(ren);
	img_viewer->SetRenderWindow(ui->myQVTKWidget->GetRenderWindow());
	//img_viewer->SetupInteractor(ui->myQVTKWidget->GetRenderWindow()->GetInteractor()); //comment out so that we can use the raw interactor of img_viewer
	img_viewer->SetSliceOrientationToXY();
	img_viewer->GetImageActor()->InterpolateOff();

	ren->ResetCamera();
	ren->DrawOn();

	ui->myQVTKWidget->GetRenderWindow()->Render();

}

png_reader::~png_reader()
{
    delete ui;
}
