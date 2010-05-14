/* updatepxlvalplugin.cpp
 * 2010-05-12: create this program by Luis Ibanez
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "ITKInvertIntensity.h"

// ITK Header Files
#include "itkInvertIntensityImageFilter.h"
#include "itkImportImageFilter.h"


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(ITKInvertIntensity, ITKInvertIntensityPlugin)


QStringList ITKInvertIntensityPlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK Invert Intensity")
						<< QObject::tr("about this plugin");
}

template <typename TPixelType>
class ITKInvertIntensitySpecializaed
{
public:
  void Execute(const QString &arg, Image4DSimple *p4DImage, QWidget *parent)
    {
    typedef TPixelType  PixelType;

    PixelType * data1d = reinterpret_cast< PixelType * >( p4DImage->getRawData() );
    unsigned long int numberOfPixels = p4DImage->getTotalBytes();

    // long pagesz = p4DImage->getTotalUnitNumberPerChannel();
	
    long nx = p4DImage->getXDim();
    long ny = p4DImage->getYDim();
    long nz = p4DImage->getZDim();
    // long sc = p4DImage->getCDim();  // Number of channels
  
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


    const bool importImageFilterWillOwnTheBuffer = false;
    importFilter->SetImportPointer( data1d, numberOfPixels, importImageFilterWillOwnTheBuffer );

    typedef itk::InvertIntensityImageFilter< ImageType > InvertFilterType;
    typename InvertFilterType::Pointer invertFilter = InvertFilterType::New();

    invertFilter->SetInput( importFilter->GetOutput() );

    invertFilter->InPlaceOn(); // Reuse the buffer


    //define datatype here
    //
    
    //input
    //update the pixel value
    if(arg == QObject::tr("ITK Invert Intensity"))
      {
      ITKInvertIntensityDialog d(p4DImage, parent);
      
      if (d.exec()!=QDialog::Accepted)
        {
        return;
        }
      else
        {
        invertFilter->Update();
        }
      
      }
    else if (arg == QObject::tr("about this plugin"))
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
    ITKInvertIntensitySpecializaed< c_pixel_type > runner; \
    runner.Execute( arg, p4DImage, parent ); \
    break; \
    } 

#define EXECUTE_ALL_PIXEL_TYPES \
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
 
void ITKInvertIntensityPlugin::processImage(const QString &arg, Image4DSimple *p4DImage, QWidget *parent)
{
   EXECUTE_ALL_PIXEL_TYPES; 
}
