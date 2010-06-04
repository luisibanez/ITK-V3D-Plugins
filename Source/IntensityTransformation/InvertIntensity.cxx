#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "InvertIntensity.h"
#include "V3DITKFilterSingleImage.h"

// ITK Header Files
#include "itkInvertIntensityImageFilter.h"


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(InvertIntensity, InvertIntensityPlugin)


QStringList InvertIntensityPlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK Invert Intensity")
						<< QObject::tr("about this plugin");
}

QStringList InvertIntensityPlugin::funclist() const
{
    return QStringList();
}


template <typename TPixelType>
class InvertIntensitySpecialized : public V3DITKFilterSingleImage< TPixelType, TPixelType >
{
public:
  typedef V3DITKFilterSingleImage< TPixelType, TPixelType >    Superclass;

  InvertIntensitySpecialized( V3DPluginCallback * callback ): Superclass(callback) {}
  virtual ~InvertIntensitySpecialized() {};

  
  void Execute(const QString &menu_name, QWidget *parent)
    {
    this->Compute(); 
    }

  virtual void ComputeOneRegion()
    {
    typedef TPixelType  PixelType;

    typedef typename Superclass::Input3DImageType   ImageType;

    typedef itk::InvertIntensityImageFilter< ImageType, ImageType > FilterType;
    typename FilterType::Pointer filter = FilterType::New();

    filter->SetInput( this->GetInput3DImage() );

    if( !this->ShouldGenerateNewWindow() )
      {
      filter->InPlaceOn();
      }
    
    filter->Update();

    this->SetOutputImage( filter->GetOutput() );
    }
	
  virtual void SetupParameters()
	{
	}
};


#define EXECUTE_PLUGING_FOR_ONE_IMAGE_TYPE( v3d_pixel_type, c_pixel_type ) \
  case v3d_pixel_type: \
    { \
    InvertIntensitySpecialized< c_pixel_type > runner( &callback ); \
    runner.Execute( menu_name, parent ); \
    break; \
    } 

 
void InvertIntensityPlugin::dofunc(const QString & func_name,
		const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent)
{
  // empty by now
}


void InvertIntensityPlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
  if (menu_name == QObject::tr("about this plugin"))
    {
    QMessageBox::information(parent, "Version info", "ITK Invert Intensity 1.0 (2010-May-12): this plugin is developed by Luis Ibanez.");
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

