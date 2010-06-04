/* DemonsRegistration.cpp
 * 2010-06-03: create this program by Luis Ibanez
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "DemonsRegistration.h"

// ITK Header Files
#include "itkImportImageFilter.h"
#include "itkDemonsRegistrationFilter.h"
#include "itkWarpImageFilter.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkImage.h"
#include "itkCommand.h"

// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(DemonsRegistration, ITKDemonsRegistrationPlugin)

QStringList ITKDemonsRegistrationPlugin::menulist() const
{
	return QStringList() << QObject::tr("ITK Demons Registration...");
}


template<typename TPixelType>
class ITKDemonsRegistrationSpecializaed
{
public:
	void Execute(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
	{
		//get image pointers
		v3dhandleList wndlist = callback.getImageWindowList();
		if(wndlist.size()<2)
		{
			v3d_msg(QObject::tr("Registration need at least two images!"));
			return;
		}
		v3dhandle oldwin = wndlist[1];
		Image4DSimple* p4DImage_fix=callback.getImage(wndlist[0]);
		Image4DSimple* p4DImage_mov=callback.getImage(wndlist[1]);

#ifdef CHECK_FOR_IMAGES_TO_HAVE_SAME_SIZE
		if(p4DImage_fix->getXDim()!=p4DImage_mov->getXDim() ||
		   p4DImage_fix->getYDim()!=p4DImage_mov->getYDim() ||
		   p4DImage_fix->getZDim()!=p4DImage_mov->getZDim() ||
		   p4DImage_fix->getCDim()!=p4DImage_mov->getCDim())
		{
			v3d_msg(QObject::tr("Two input images have different size!"));
			return;
		}
#endif

		//set dimention info
		const unsigned int Dimension = 3;

		//get global setting
		V3D_GlobalSetting globalSetting = callback.getGlobalSetting();
	    int channelToFilter = globalSetting.iChannel_for_plugin;
	    if( channelToFilter >= p4DImage_fix->getCDim())
		{
			v3d_msg(QObject::tr("You are selecting a channel that doesn't exist in this image."));
			return;
		}

		//------------------------------------------------------------------
		//import images from V3D
		typedef TPixelType PixelType;
		typedef itk::Image< PixelType,  Dimension > ImageType_input;
		typedef itk::ImportImageFilter<PixelType, Dimension> ImportFilterType;

		typename ImportFilterType::Pointer importFilter_fix = ImportFilterType::New();
		typename ImportFilterType::Pointer importFilter_mov = ImportFilterType::New();

		//set ROI region
		typename ImportFilterType::RegionType region;
		typename ImportFilterType::IndexType start;
		start.Fill(0);
		typename ImportFilterType::SizeType size;
		size[0] = p4DImage_fix->getXDim();
		size[1] = p4DImage_fix->getYDim();
		size[2] = p4DImage_fix->getZDim();
		region.SetIndex(start);
		region.SetSize(size);
		importFilter_fix->SetRegion(region);
		importFilter_mov->SetRegion(region);

		//set image Origin
		typename ImageType_input::PointType origin;
		origin.Fill(0.0);
		importFilter_fix->SetOrigin(origin);
		importFilter_mov->SetOrigin(origin);
		//set spacing
		typename ImportFilterType::SpacingType spacing;
		spacing.Fill(1.0);
		importFilter_fix->SetSpacing(spacing);
		importFilter_mov->SetSpacing(spacing);

		//set import image pointer
		PixelType * data1d_fix = reinterpret_cast<PixelType *> (p4DImage_fix->getRawData());
		PixelType * data1d_mov = reinterpret_cast<PixelType *> (p4DImage_mov->getRawData());
		unsigned long int numberOfPixels = p4DImage_fix->getTotalBytes();
		const bool importImageFilterWillOwnTheBuffer = false;
		importFilter_fix->SetImportPointer(data1d_fix, numberOfPixels,importImageFilterWillOwnTheBuffer);
		importFilter_mov->SetImportPointer(data1d_mov, numberOfPixels,importImageFilterWillOwnTheBuffer);


    // RUN DEMONS FILTER HERE
    typedef itk::Vector< float, Dimension >    VectorPixelType;
    typedef itk::Image<  VectorPixelType, Dimension > DeformationFieldType;
    typedef itk::DemonsRegistrationFilter<
      ImageType_input,
      ImageType_input,
      DeformationFieldType>   RegistrationFilterType;

    typename RegistrationFilterType::Pointer filter = RegistrationFilterType::New();

    filter->SetFixedImage( importFilter_fix->GetOutput() );
    filter->SetMovingImage( importFilter_mov->GetOutput() );


    filter->SetNumberOfIterations( 50 );
    filter->SetStandardDeviations( 1.0 );


    filter->Update();

    typedef itk::WarpImageFilter<
                            ImageType_input, 
                            ImageType_input,
                            DeformationFieldType  >     WarperType;

    typedef itk::LinearInterpolateImageFunction<
                                     ImageType_input,
                                     double          >  InterpolatorType;

    typename WarperType::Pointer warper = WarperType::New();

    typename InterpolatorType::Pointer interpolator = InterpolatorType::New();
    typename ImageType_input::Pointer fixedImage = importFilter_fix->GetOutput();

    warper->SetInput( importFilter_mov->GetOutput() );
    warper->SetInterpolator( interpolator );
    warper->SetOutputSpacing( fixedImage->GetSpacing() );
    warper->SetOutputOrigin( fixedImage->GetOrigin() );
    warper->SetOutputDirection( fixedImage->GetDirection() );

#ifdef MANUALLY_COPYING_IMAGE_BACK_TO_V3D
		//------------------------------------------------------------------
		typedef itk::ImageRegionConstIterator<ImageType_input> IteratorType;
		IteratorType it(caster->GetOutput(), caster->GetOutput()->GetRequestedRegion());
		it.GoToBegin();

//		if(!globalSetting.b_plugin_dispResInNewWindow)
//		{
			printf("display results in a new window\n");
			//copy data back to V3D
			while(!it.IsAtEnd())
			{
				*data1d_mov=it.Get();
				++it;
				++data1d_mov;
			}

			callback.setImageName(oldwin, callback.getImageName(oldwin)+"_new");
			callback.updateImageWindow(oldwin);
      }
#endif

	}

};

#define EXECUTE( v3d_pixel_type, c_pixel_type ) \
  case v3d_pixel_type: \
    { \
	ITKDemonsRegistrationSpecializaed< c_pixel_type > runner; \
    runner.Execute(  menu_name, callback, parent ); \
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

void ITKDemonsRegistrationPlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
	v3dhandle curwin = callback.currentImageWindow();
	if (!curwin)
    {
		v3d_msg(tr("You don't have any image open in the main window."));
		return;
    }

  EXECUTE_ALL_PIXEL_TYPES;
}
