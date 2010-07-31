#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "LabelShapeStatistics.h"
#include "V3DITKFilterSingleImage.h"

// ITK Header Files
#include "itkLabelObject.h"
#include "itkLabelMap.h"
#include "itkLabelImageToShapeLabelMapFilter.h"


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(LabelShapeStatistics, LabelShapeStatisticsPlugin)


QStringList LabelShapeStatisticsPlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK LabelShapeStatistics")
            << QObject::tr("about this plugin");
}

QStringList LabelShapeStatisticsPlugin::funclist() const
{
    return QStringList();
}


template <typename TPixelType>
class PluginSpecialized : public V3DITKFilterSingleImage< TPixelType, TPixelType >
{
  typedef V3DITKFilterSingleImage< TPixelType, TPixelType >   Superclass;
  typedef typename Superclass::Input3DImageType               ImageType;

  typedef itk::ShapeLabelObject< TPixelType, 3 >       LabelObjectType;
  typedef itk::LabelMap< LabelObjectType >             LabelMapType;
 
  typedef itk::LabelImageToShapeLabelMapFilter< ImageType, LabelMapType > FilterType;

public:

  PluginSpecialized( V3DPluginCallback * callback ): Superclass(callback)
    {
    this->m_Filter = FilterType::New();
    this->RegisterInternalFilter( this->m_Filter, 1.0 );
    }

  virtual ~PluginSpecialized() {};


  void Execute(const QString &menu_name, QWidget *parent)
    {
    V3DITKGenericDialog dialog("LabelShapeStatistics");

    dialog.AddDialogElement("BackgroundValue",1.0, -5.0, 5.0);

    // FIXME : add support for selecting other Attributes...
    // this->m_Filter->SetComputePhysicalSize( true );

    if( dialog.exec() == QDialog::Accepted )
      {
      this->m_Filter->SetBackgroundValue( dialog.GetValue("BackgroundValue") );

      this->Compute();
      }
    }

  virtual void ComputeOneRegion()
    {

    this->m_Filter->SetInput( this->GetInput3DImage() );

std::cout << "BEFORE CALLING UPDATE()" << std::endl;
    this->m_Filter->Update();
std::cout << "AFTER CALLING UPDATE()" << std::endl;

    LabelMapType * outputLabelMap = this->m_Filter->GetOutput();
std::cout << "outputLabelMap = " << outputLabelMap << std::endl;
    unsigned long numberOfLabelMapObjects = outputLabelMap->GetNumberOfLabelObjects();

    for( unsigned long labelCounter = 0; labelCounter < numberOfLabelMapObjects; labelCounter++ )
      {
      LabelObjectType * labelObject = outputLabelMap->GetNthLabelObject( labelCounter );
std::cout << labelCounter << " labelObject = " << labelObject << std::endl;
      std::cout << labelCounter << " : " << labelObject->GetPhysicalSize() << std::endl;
      }

    // this->SetOutputImage( this->m_Filter->GetOutput() );
    // FIXME: overload Compute() to make sure that we don't send any images back...
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


void LabelShapeStatisticsPlugin::dofunc(const QString & func_name,
    const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent)
{
  // empty by now
}


void LabelShapeStatisticsPlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
  if (menu_name == QObject::tr("about this plugin"))
    {
    QMessageBox::information(parent, "Version info", "ITK LabelShapeStatistics 1.0 (2010-Jun-21): this plugin is developed by Luis Ibanez.");
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

