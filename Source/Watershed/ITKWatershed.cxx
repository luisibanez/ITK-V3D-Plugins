/* ITKWatershed.cxx
 * 2010-06-03: create this program by Yang Yu
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "ITKWatershed.h"
#include "V3DITKFilterSingleImage.h"

// ITK Header Files
#include "itkImage.h"

#include "itkImportImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkRelabelComponentImageFilter.h"

#include "itkGradientMagnitudeRecursiveGaussianImageFilter.h"
#include "itkWatershedImageFilter.h"

// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(ITKWatershed, ITKWatershedPlugin)

//plugin funcs
const QString title = "ITK Watershed";
QStringList ITKWatershedPlugin::menulist() const
{
	return QStringList() << QObject::tr("ITK Watershed")
						 << QObject::tr("about this plugin");
}

QStringList ITKWatershedPlugin::funclist() const
{
    return QStringList();
}

void ITKWatershedPlugin::dofunc(const QString & func_name,
											const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent)
{
	// empty by now
}


template <typename TInputPixelType, typename TOutputPixelType>
class ITKWatershedSpecializaed // : public V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >
{
public:
	//typedef V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >    Superclass;
	
	//ITKWatershedSpecializaed( V3DPluginCallback * callback ): Superclass(callback) {}
	virtual ~ITKWatershedSpecializaed() {};
	
	//
	void Execute(V3DPluginCallback &callback, QWidget *parent)
	{
		v3dhandle curwin = callback.currentImageWindow();
		if (!curwin)
		{
			v3d_msg(QObject::tr("You don't have any image open in the main window."));
			return;
		}
		
		Image4DSimple *p4DImage = callback.getImage(curwin);
		if (! p4DImage)
		{
			v3d_msg(QObject::tr("The input image is null."));
			return;
		}	
			
		V3D_GlobalSetting globalSetting = callback.getGlobalSetting();
		
		
		//init
		typedef TInputPixelType  PixelType;
		
		PixelType * data1d = reinterpret_cast< PixelType * >( p4DImage->getRawData() );
		unsigned long int numberOfPixels = p4DImage->getTotalBytes();
		
		long pagesz = p4DImage->getTotalUnitNumberPerChannel();
		
		long nx = p4DImage->getXDim();
		long ny = p4DImage->getYDim();
		long nz = p4DImage->getZDim();
		long nc = p4DImage->getCDim();  // Number of channels
		
		int channelToFilter = globalSetting.iChannel_for_plugin;
		
		if( channelToFilter >= nc )
		{
			v3d_msg(QObject::tr("You are selecting a channel that doesn't exist in this image."));
			return;
		}
		
		long offsets=0; 
		if(channelToFilter>=0) offsets = channelToFilter*pagesz; 
		
		const unsigned int Dimension = 3; // \par
		
		typedef itk::Image< TInputPixelType, Dimension > InputImageType;
		typedef itk::Image< TOutputPixelType, Dimension > OutputImageType;
		typedef itk::ImportImageFilter< TInputPixelType, Dimension > ImportFilterType;
		
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
		
		typename InputImageType::PointType origin;
		origin.Fill( 0.0 );
		
		importFilter->SetOrigin( origin );
		
		typename ImportFilterType::SpacingType spacing;
		spacing.Fill( 1.0 );
		
		importFilter->SetSpacing( spacing );
		
		const bool importImageFilterWillOwnTheBuffer = false;
		
		//Declaration of fileters
		typedef   itk::GradientMagnitudeRecursiveGaussianImageFilter< InputImageType, OutputImageType > GradientMagnitudeFilterType;
		typename GradientMagnitudeFilterType::Pointer gradienMagnitudeFilter = GradientMagnitudeFilterType::New();
		
		typedef  itk::WatershedImageFilter< OutputImageType > WatershedFilterType;
		typename WatershedFilterType::Pointer watershedFilter = WatershedFilterType::New();
		
		typedef typename WatershedFilterType::OutputImageType  LabeledImageType;
		
		typedef itk::RelabelComponentImageFilter< LabeledImageType, OutputImageType > RelabelComponentImageFilterType;
		typename RelabelComponentImageFilterType::Pointer relabelComponent = RelabelComponentImageFilterType::New();
		
		//set \pars		
		
		//consider multiple channels
		if(channelToFilter==-1)
		{
			TOutputPixelType *output1d;
			try
			{
				output1d = new TOutputPixelType [numberOfPixels];
			}
			catch(...)
			{
				std::cerr << "Error memroy allocating." << std::endl;
				return;
			}
			
			const bool filterWillDeleteTheInputBuffer = false;
			
			for(long ch=0; ch<nc; ch++)
			{
				offsets = ch*pagesz;
				
				TOutputPixelType *p = output1d+offsets;
				
				importFilter->SetImportPointer( data1d+offsets, pagesz, importImageFilterWillOwnTheBuffer );
				
				// watershed
				gradienMagnitudeFilter->SetInput( importFilter->GetOutput() );
				
				watershedFilter->SetInput( gradienMagnitudeFilter->GetOutput() );
				relabelComponent->SetInput( watershedFilter->GetOutput() );
				
				gradienMagnitudeFilter->SetSigma( 1.0 );
				
				watershedFilter->SetThreshold( 100.0 );
				watershedFilter->SetLevel( 127.5 );
				
				//watershedSegmentation
				relabelComponent->GetOutput()->GetPixelContainer()->SetImportPointer( p, pagesz, filterWillDeleteTheInputBuffer);
				
				try
				{
					relabelComponent->Update();
				}
				catch( itk::ExceptionObject & excp)
				{
					std::cerr << "Error run this filter." << std::endl;
					std::cerr << excp << std::endl;
					return;
				}
				
			}
			
			setPluginOutputAndDisplayUsingGlobalSetting(output1d, nx, ny, nz, nc, callback);
		}
		else if(channelToFilter<nc)
		{
			importFilter->SetImportPointer( data1d+offsets, pagesz, importImageFilterWillOwnTheBuffer );
			
			// watershed
			gradienMagnitudeFilter->SetInput( importFilter->GetOutput() );
			
			watershedFilter->SetInput( gradienMagnitudeFilter->GetOutput() );
			relabelComponent->SetInput( watershedFilter->GetOutput() );
			
			gradienMagnitudeFilter->SetSigma( 1.0 );
			
			watershedFilter->SetThreshold( 0.0 ); // (0.0, 1.0)
			watershedFilter->SetLevel( 0.05 ); // (0.0, 1.0)
			
			//watershedSegmentation
			try
			{
				relabelComponent->Update();
			}
			catch( itk::ExceptionObject & excp)
			{
				std::cerr << "Error run this filter." << std::endl;
				std::cerr << excp << std::endl;
				return;
			}
			
			// output
			typename OutputImageType::PixelContainer * container;
			
			container =relabelComponent->GetOutput()->GetPixelContainer();
			container->SetContainerManageMemory( false );
			
			typedef TOutputPixelType OutputPixelType;
			OutputPixelType * output1d = container->GetImportPointer();
			
			setPluginOutputAndDisplayUsingGlobalSetting(output1d, nx, ny, nz, 1, callback);
			
		}
		
	} //Execute	
		
	
};

#define EXECUTE( v3d_pixel_type, input_pixel_type, output_pixel_type ) \
	case v3d_pixel_type: \
	{ \
		ITKWatershedSpecializaed< input_pixel_type, output_pixel_type > runner; \
		runner.Execute( callback, parent ); \
		break; \
	} 

#define EXECUTE_ALL_PIXEL_TYPES \
	ImagePixelType pixelType = p4DImage->getDatatype(); \
	switch( pixelType )  \
	{  \
		EXECUTE( V3D_UINT8, unsigned char, float );  \
		EXECUTE( V3D_UINT16, unsigned short int, float );  \
		EXECUTE( V3D_FLOAT32, float, float );  \
		case V3D_UNKNOWN:  \
		{  \
		}  \
	}  


void ITKWatershedPlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
    if (menu_name == QObject::tr("ITK Watershed"))
    {
    	v3dhandle curwin = callback.currentImageWindow();
		if (!curwin)
		{
			v3d_msg(QObject::tr("You don't have any image open in the main window."));
			return;
		}
		
		Image4DSimple *p4DImage = callback.getImage(curwin);
		if (! p4DImage)
		{
			v3d_msg(QObject::tr("The input image is null."));
			return;
		}	
		
		EXECUTE_ALL_PIXEL_TYPES;
    }
	else if (menu_name == QObject::tr("about this plugin"))
	{
		QMessageBox::information(parent, "Version info", "ITK Watershed 1.0 (2010-June-04): this plugin is developed by Yang Yu.");
	}
}



