# VTK-Demo
Some simple demos of VTK and Qt.<br>
The compilation and installation processes of VTK and Qt: https://blog.csdn.net/anhec/article/details/86777302

## [QPNGReader](./QPNGReader)
A simple demo to presentation the usage of vtkPNGReader and the interaction between VTK and Qt. 

## [QDicomSeriesReader](./QDicomSeriesReader)
A demo to presentation how to use vtkDICOMReader to read DICOM folder, and traverse all the pictures through a Qt slide or mouse wheel.<br>
The mouse wheel interaction implemented by redefining the interactor style class inherited from vtkInteractorStyleImage.

## [DicomVolumeRender](./DicomVolumeRender)
A volume Render demo that restructures dicom series with vtkFixedPointVolumeRayCastMapper or vtkGPUVolumeRayCastMapper.

## [QDicomVolumeRender](./QDicomVolumeRender)
DicomVolumeRender based on Qt.
