
SET(QtITK_SRCS
  V3DITKPluginBaseSingleImage.cxx
  V3DITKGenericDialog.cxx
  V3DITKProgressDialog.cxx
  itkEventToQtSignalTranslator.cxx
  )

QT4_WRAP_CPP(QT_MOC_SRCS
  V3DITKPluginBaseSingleImage.h
  V3DITKGenericDialog.h
  V3DITKProgressDialog.h
  itkEventToQtSignalTranslator.h
  )

ADD_LIBRARY(V3DITKCommon ${QtITK_SRCS} ${QT_MOC_SRCS})
