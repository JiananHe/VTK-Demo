#include <vtkSmartPointer.h>
#include <vtkDICOMImageReader.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCellPicker.h>
#include <vtkImagePlaneWidget.h>
#include <vtkMarchingCubes.h>
#include <vtkImageCast.h>
#include <vtkFixedPointVolumeRayCastMapper.h>
#include <vtkVolumeProperty.h>
#include <vtkVolume.h>
#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkImageMapToColors.h>
#include <vtkProperty.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkSmartPointer.h>
#include <vtkCamera.h>
 
int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		cout << "Usage: " << argv[0] << " Dicom folder" << endl;
		return EXIT_FAILURE;
	}

    //std::string folder = "C:\\Users\\13249\\Documents\\VTK_Related\\dataset\\ct_1\\Abdomen";
 
    //read all the dicom files with the received path.
    vtkSmartPointer<vtkDICOMImageReader> reader = vtkSmartPointer<vtkDICOMImageReader>::New();
    reader->SetDirectoryName(argv[1]);
    reader->Update();
 
    int imageDims[3];
    reader->GetOutput()->GetDimensions(imageDims); //need include <vtkimagedata.h>
	cout << "dimension[] :" << imageDims[0] << " " << imageDims[1] << " " << imageDims[2] << endl;
 
	/*vtkSmartPointer<vtkImageCast> cast = vtkSmartPointer<vtkImageCast>::New();
	cast->SetInputData((vtkDataObject*)reader->GetOutput());
	cast->SetOutputScalarTypeToFloat();
	cast->SetOutputScalarTypeToUnsignedChar();
	cast->Update();*/

	// render, render window, render window interactor
	vtkSmartPointer<vtkRenderer> rendererVolume = vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();

	renderWindow->AddRenderer(rendererVolume);
	renderWindowInteractor->SetRenderWindow(renderWindow);
 
	//**********************************************************************
	//method 1
	/*vtkSmartPointer<vtkFixedPointVolumeRayCastMapper> volumeMapper =
		vtkSmartPointer<vtkFixedPointVolumeRayCastMapper>::New();
	volumeMapper->SetInputConnection(reader->GetOutputPort());*/
 
	////method 2
	vtkSmartPointer<vtkGPUVolumeRayCastMapper> volumeMapperGpu = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
	volumeMapperGpu->SetInputConnection(reader->GetOutputPort());
	//volumeMapperGpu->SetInputData(cast->GetOutput());
	//volumeMapperGpu->SetImageSampleDistance(0.5);
	//volumeMapperGpu->SetSampleDistance(1.0);
	//volumeMapperGpu->SetAutoAdjustSampleDistances(1);

	//***********************************************************************
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

	// Finally, add the volume to the renderer
	rendererVolume->AddViewProp(volume);

	// Set up an initial view of the volume.  The focal point will be the
	// center of the volume, and the camera position will be 400mm to the
	 // patient's left (which is our right).
	vtkCamera *camera = rendererVolume->GetActiveCamera();
	double *c = volume->GetCenter();
	camera->SetViewUp(0, 0, -1);
	camera->SetPosition(c[0], c[1] - 400, c[2]);
	camera->SetFocalPoint(c[0], c[1], c[2]);
	camera->Azimuth(30.0);
	camera->Elevation(30.0);

	renderWindow->Render();
	renderWindowInteractor->Start();
 
	return 0;
}