#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "OtsuMultipleThresholds.h"
#include "V3DITKFilterSingleImage.h"

// ITK Header Files
#include "itkOtsuMultipleThresholdsImageFilter.h"


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(OtsuMultipleThresholds, OtsuMultipleThresholdsPlugin)


QStringList OtsuMultipleThresholdsPlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK OtsuMultipleThresholds")
            << QObject::tr("about this plugin");
}

QStringList OtsuMultipleThresholdsPlugin::funclist() const
{
    return QStringList();
}


template <typename TPixelType>
class PluginSpecialized : public V3DITKFilterSingleImage< TPixelType, TPixelType >
{
  typedef V3DITKFilterSingleImage< TPixelType, TPixelType >   Superclass;
  typedef typename Superclass::Input3DImageType               ImageType;

  typedef itk::OtsuMultipleThresholdsImageFilter< ImageType, ImageType > FilterType;

public:

  PluginSpecialized( V3DPluginCallback * callback ): Superclass(callback)
    {
    this->m_Filter = FilterType::New();
    }

  virtual ~PluginSpecialized() {};


  void Execute(const QString &menu_name, QWidget *parent)
    {
    V3DITKGenericDialog dialog("Otsu Multiple Threshold");

    dialog.AddDialogElement("HistogramBins", 128, 1, 256);
    dialog.AddDialogElement("Thresholds", 5, 1, 10);
    dialog.AddDialogElement("FirstLabel", 1, 1, 500);

    if( dialog.exec() == QDialog::Accepted )
      {
      // this->m_Filter->SetNumberOfHistogramBins( dialog.GetValue("HistogramBins") );
      // this->m_Filter->SetNumberOfThresholds( dialog.GetValue("Thresholds") );
      // this->m_Filter->SetLabelOffset( dialog.GetValue("FirstLabel") );

      this->Compute();
      }

    }

  virtual void ComputeOneRegion()
    {

    this->m_Filter->SetInput( this->GetInput3DImage() );

    this->m_Filter->Update();

    this->SetOutputImage( this->m_Filter->GetOutput() );
    }


private:

    typename FilterType::Pointer   m_Filter;

};


#define EXECUTE_PLUGIN_FOR_ONE_IMAGE_TYPE( v3d_pixel_type, c_pixel_type ) \
  case v3d_pixel_type: \
    { \
    PluginSpecialized< c_pixel_type > runner( &callback ); \
    runner.Execute( menu_name, parent ); \
    break; \
    }


void OtsuMultipleThresholdsPlugin::dofunc(const QString & func_name,
    const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent)
{
  // empty by now
}


void OtsuMultipleThresholdsPlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
  if (menu_name == QObject::tr("about this plugin"))
    {
    QMessageBox::information(parent, "Version info", "ITK OtsuMultipleThresholds 1.0 (2010-Jul-29): this plugin is developed by Luis Ibanez.");
    return;
    }

  v3dhandle curwin = callback.currentImageWindow();
  if (!curwin)
    {
    v3d_msg(tr("You don't have any image open in the main window."));
    return;
    }

  Image4DSimple *p4DImage = callback.getImage(curwin);
  if (! p4DImage)
    {
    v3d_msg(tr("The input image is null."));
    return;
    }

  EXECUTE_PLUGIN_FOR_ALL_PIXEL_TYPES;
}

