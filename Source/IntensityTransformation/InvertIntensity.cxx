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
class InvertIntensitySpecialized : V3DITKFilterSingleImage< TPixelType, TPixelType >
{
public:
  InvertIntensitySpecialized( V3DPluginCallback & callback );
  virtual ~InvertIntensitySpecialized() {};

  typedef V3DITKFilterSingleImage< TPixelType, TPixelType >    Superclass;
  
  void Execute(const QString &menu_name, QWidget *parent)
    {
    this->Compute(); 
    }

  virtual void ComputeOneRegion()
    {
    typedef TPixelType  PixelType;

    typedef typename Superclass::Input3DImageType   ImageType;

    typedef itk::InvertIntensityImageFilter< ImageType, ImageType > InvertFilterType;
    typename InvertFilterType::Pointer filter = InvertFilterType::New();

    filter->SetInput( this->GetInput3DImage() );

    filter->InPlaceOn();
    
    filter->Update();
    }
};


#define EXECUTE( v3d_pixel_type, c_pixel_type ) \
  case v3d_pixel_type: \
    { \
    InvertIntensitySpecialized< c_pixel_type > runner( callback ); \
    runner.Execute( menu_name, parent ); \
    break; \
    } 

#define EXECUTE_ALL_PIXEL_TYPES \
    ImagePixelType pixelType = p4DImage->getDatatype(); \
    switch( pixelType )  \
      {  \
      EXECUTE( V3D_UINT8, unsigned char );  \
      EXECUTE( V3D_UINT16, unsigned short int );  \
      EXECUTE( V3D_FLOAT32, float );  \
      case V3D_UNKNOWN:  \
        {  \
        }  \
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

  EXECUTE_ALL_PIXEL_TYPES; 
}

