cmake_minimum_required(VERSION 3.3...3.12 FATAL_ERROR)


project (QDicomVolumeRender)
find_package(VTK REQUIRED)
vtk_module_config(VTK
  vtkCommonCore
  vtkIOImage
  vtkIOXML
  vtkInteractionWidgets
  vtkFiltersSources
  vtkGUISupportQt
  vtkInteractionImage
  vtkInteractionStyle
  vtkRenderingCore
  vtkRenderingFreeType
  vtkRenderingVolumeOpenGL2
)
include(${VTK_USE_FILE})

if("${VTK_QT_VERSION}" STREQUAL "")
  message(FATAL_ERROR "VTK was not built with Qt")
endif()

set( Srcs main.cpp dicom_volume_render.cpp )

set( Hdrs dicom_volume_render.h )

set( MOC_Hdrs dicom_volume_render.h )

set( UIs dicom_volume_render.ui )

# Use the include path and library for Qt that is used by VTK.
include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Widgets Core REQUIRED QUIET)
qt5_wrap_ui(UI_Srcs ${UIs})

add_executable(QDicomVolumeRender
  MACOSX_BUNDLE  ${Srcs} ${Hdrs} ${UI_Srcs} ${MOC_Hdrs})
target_link_libraries(QDicomVolumeRender Qt5::Core Qt5::Gui Qt5::Widgets)
target_link_libraries(QDicomVolumeRender ${VTK_LIBRARIES})