#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "Sigmoid.h"

// ITK Header Files
#include "itkSigmoidImageFilter.h"
#include "itkImportImageFilter.h"


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(Sigmoid, SigmoidPlugin)


QStringList SigmoidPlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK Invert Intensity")
						<< QObject::tr("about this plugin");
}

QStringList SigmoidPlugin::funclist() const
{
    return QStringList();
}


template <typename TPixelType>
class SigmoidSpecialized
{
public:
  void Execute(const QString &menu_name,  V3DPluginCallback & callback, QWidget *parent)
    {
    typedef TPixelType  PixelType;

    v3dhandle curwin = callback.currentImageWindow();
	
    V3D_GlobalSetting globalSetting = callback.getGlobalSetting();
    Image4DSimple *p4DImage = callback.getImage(curwin);

    PixelType * data1d = reinterpret_cast< PixelType * >( p4DImage->getRawData() );

    // long pagesz = p4DImage->getTotalUnitNumberPerChannel();
	
    long nx = p4DImage->getXDim();
    long ny = p4DImage->getYDim();
    long nz = p4DImage->getZDim();
    long sc = p4DImage->getCDim();  // Number of channels
  
    unsigned long int numberOfPixels = nx * ny * nz;

    int channelToFilter = globalSetting.iChannel_for_plugin;

    if( channelToFilter >= sc )
      {
      v3d_msg(QObject::tr("You are selecting a channel that doesn't exist in this image."));
      return;
      }


    const unsigned int Dimension = 3;
	
    typedef itk::Image< PixelType, Dimension > ImageType;
    typedef itk::ImportImageFilter< PixelType, Dimension > ImportFilterType;

    typename ImportFilterType::Pointer importFilter = ImportFilterType::New();

    typename ImportFilterType::SizeType size;
    size[0] = nx;
    size[1] = ny;
    size[2] = nz;

    typename ImportFilterType::IndexType start;
    start.Fill( 0 );

    typename ImportFilterType::RegionType region;
    region.SetIndex( start );
    region.SetSize(  size  );

    importFilter->SetRegion( region );

    region.SetSize( size );

    typename ImageType::PointType origin;
    origin.Fill( 0.0 );
  
    importFilter->SetOrigin( origin );


    typename ImportFilterType::SpacingType spacing;
    spacing.Fill( 1.0 );
  
    importFilter->SetSpacing( spacing );

    PixelType * componentData1d = data1d + channelToFilter * numberOfPixels;

    const bool importImageFilterWillOwnTheBuffer = false;
    importFilter->SetImportPointer( componentData1d, numberOfPixels, importImageFilterWillOwnTheBuffer );

    typedef itk::SigmoidImageFilter< ImageType, ImageType > InvertFilterType;
    typename InvertFilterType::Pointer filter = InvertFilterType::New();

    filter->SetInput( importFilter->GetOutput() );

    filter->InPlaceOn(); // Reuse the buffer


    //define datatype here
    //
    
    //input
    //update the pixel value
    if(menu_name == QObject::tr("ITK Invert Intensity"))
      {
      SigmoidDialog d(p4DImage, parent);
      
      if (d.exec()!=QDialog::Accepted)
        {
        return;
        }
      else
        {
        filter->Update();
        }
      
      }
    else if (menu_name == QObject::tr("about this plugin"))
      {
      QMessageBox::information(parent, "Version info", "ITK Invert Intensity 1.0 (2010-May-12): this plugin is developed by Luis Ibanez.");
      }
    else
      {
      return;
      }
    }

};

#define EXECUTE( v3d_pixel_type, c_pixel_type ) \
  case v3d_pixel_type: \
    { \
    SigmoidSpecialized< c_pixel_type > runner; \
    runner.Execute( menu_name, callback, parent ); \
    break; \
    } 

#define EXECUTE_ALL_PIXEL_TYPES \
    Image4DSimple *p4DImage = callback.getImage(curwin); \
    if (! p4DImage) return; \
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
 
void SigmoidPlugin::dofunc(const QString & func_name,
		const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent)
{
  // empty by now
}


void SigmoidPlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
	v3dhandle curwin = callback.currentImageWindow();
	if (!curwin)
    {
		v3d_msg(tr("You don't have any image open in the main window."));
		return;
    }
	
  EXECUTE_ALL_PIXEL_TYPES; 
}

