/* ITKThresholdSegmentation.cxx
 * 2010-06-03: create this program by Yang Yu
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "ITKThresholdSegmentation.h"
#include "V3DITKFilterSingleImage.h"

// ITK Header Files
#include "itkImage.h"

#include "itkImportImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

#include "itkZeroCrossingImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"

#include "itkFastMarchingImageFilter.h"
#include "itkThresholdSegmentationLevelSetImageFilter.h"

// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(ITKThresholdSegmentation, ITKThresholdSegmentationPlugin)

//plugin funcs
const QString title = "ITK ThresholdSegmentation";
QStringList ITKThresholdSegmentationPlugin::menulist() const
{
	return QStringList() << QObject::tr("ITK ThresholdSegmentation")
						 << QObject::tr("about this plugin");
}

QStringList ITKThresholdSegmentationPlugin::funclist() const
{
    return QStringList();
}

void ITKThresholdSegmentationPlugin::dofunc(const QString & func_name,
											const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent)
{
	// empty by now
}


template <typename TInputPixelType, typename TOutputPixelType>
class ITKThresholdSegmentationSpecializaed // : public V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >
{
public:
	//typedef V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >    Superclass;
	
	//ITKThresholdSegmentationSpecializaed( V3DPluginCallback * callback ): Superclass(callback) {}
	virtual ~ITKThresholdSegmentationSpecializaed() {};
	
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
		
		typedef itk::CastImageFilter< InputImageType, OutputImageType> CastImageFilterType;
		typename CastImageFilterType::Pointer castImageFilter = CastImageFilterType::New();
		
		//Declaration of fileters
		typedef itk::BinaryThresholdImageFilter< OutputImageType, InputImageType    >    ThresholdingFilterType;
		typename ThresholdingFilterType::Pointer thresholder = ThresholdingFilterType::New();
		
		typedef itk::ThresholdSegmentationLevelSetImageFilter< OutputImageType, OutputImageType >  ThresholdSegmentationLevelSetImageFilterType;
		typename ThresholdSegmentationLevelSetImageFilterType::Pointer thresholdSegmentation = ThresholdSegmentationLevelSetImageFilterType::New();
		
		typedef itk::FastMarchingImageFilter< OutputImageType, OutputImageType > FastMarchingFilterType;
		typename FastMarchingFilterType::Pointer fastMarching = FastMarchingFilterType::New();
		
		typedef typename FastMarchingFilterType::NodeContainer	NodeContainer;
		typedef typename FastMarchingFilterType::NodeType		NodeType;
		typename NodeContainer::Pointer seeds = NodeContainer::New();
		
		typename OutputImageType::IndexType  seedPosition; // seedPosition[0]  seedPosition[1]  seedPosition[2] 
		
		NodeType node;
		
		//set \pars
		const double initialDistance = 5.0; //
		const double seedValue = - initialDistance;
		
		LandmarkList list_landmark_sub=callback.getLandmark(curwin);
		if(list_landmark_sub.size()<1)
		{
			v3d_msg(QObject::tr("You should select one seed from your image."));
			return;
		}
		else
		{
			//seeds
			seeds->Initialize();
			
			for(int i=0;  i<list_landmark_sub.size(); i++)
			{
				//
				seedPosition[0] = list_landmark_sub[i].x -1; // notice 0-based and 1-based difference
				seedPosition[1] = list_landmark_sub[i].y -1;
				seedPosition[2] = list_landmark_sub[i].z -1;
				
				node.SetValue( seedValue );
				node.SetIndex( seedPosition );
				
				seeds->InsertElement( i, node );
			}
		}
		
		//const double stoppingTime = sqrt(nx*nx + ny*ny + nz*nz);
		
		const double curvatureScaling   = 1.0; // Level Set 
		const double propagationScaling = 1.0; 
		
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
				
				castImageFilter->SetInput( importFilter->GetOutput() );
				
				try
				{
					castImageFilter->Update();
				}
				catch( itk::ExceptionObject & excp)
				{
					std::cerr << "Error run this filter." << std::endl;
					std::cerr << excp << std::endl;
					return;
				}
				
				// threshold segmentation algorithm
				thresholdSegmentation->SetInput( fastMarching->GetOutput() );
				thresholdSegmentation->SetFeatureImage( castImageFilter->GetOutput() );
				
				
				fastMarching->SetTrialPoints(  seeds  );
				fastMarching->SetOutputSize( importFilter->GetOutput()->GetBufferedRegion().GetSize() );
				//fastMarching->SetStoppingValue(  stoppingTime  );
				fastMarching->SetSpeedConstant( 1.0 );
				
				thresholdSegmentation->SetPropagationScaling( propagationScaling );
				thresholdSegmentation->SetCurvatureScaling( curvatureScaling );
				
				thresholdSegmentation->SetMaximumRMSError( 0.02 );
				thresholdSegmentation->SetNumberOfIterations( 50 ); //stoppingTime
				
				thresholdSegmentation->SetUpperThreshold( 255 );
				thresholdSegmentation->SetLowerThreshold( 200 );
				thresholdSegmentation->SetIsoSurfaceValue(0.0);
				
				//thresholdSegmentation
				thresholdSegmentation->GetOutput()->GetPixelContainer()->SetImportPointer( p, pagesz, filterWillDeleteTheInputBuffer);
				
				try
				{
					thresholdSegmentation->Update();
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
			
			castImageFilter->SetInput( importFilter->GetOutput() );
			
			try
			{
				castImageFilter->Update();
			}
			catch( itk::ExceptionObject & excp)
			{
				std::cerr << "Error run this filter." << std::endl;
				std::cerr << excp << std::endl;
				return;
			}
			
			// threshold segmentation algorithm
			thresholdSegmentation->SetInput( fastMarching->GetOutput() );
			thresholdSegmentation->SetFeatureImage( castImageFilter->GetOutput() );
			
			
			fastMarching->SetTrialPoints(  seeds  );
			fastMarching->SetOutputSize( importFilter->GetOutput()->GetBufferedRegion().GetSize() );
			//fastMarching->SetStoppingValue(  stoppingTime  );
			fastMarching->SetSpeedConstant( 1.0 );
			
//			fastMarching->SetOutputRegion( output1d->GetBufferedRegion() );
//			fastMarching->SetOutputSpacing( output1d->GetSpacing() );
//			fastMarching->SetOutputOrigin( output1d->GetOrigin() );
//			fastMarching->SetOutputDirection( output1d->GetDirection() );
			
			thresholdSegmentation->SetPropagationScaling( propagationScaling );
			thresholdSegmentation->SetCurvatureScaling( curvatureScaling );
			
			thresholdSegmentation->SetMaximumRMSError( 0.02 );
			thresholdSegmentation->SetNumberOfIterations( 100 ); //stoppingTime
			
			thresholdSegmentation->SetUpperThreshold( 255 );
			thresholdSegmentation->SetLowerThreshold( 100 );
			thresholdSegmentation->SetIsoSurfaceValue(0.0);
			
			thresholder->SetInput( thresholdSegmentation->GetOutput() );
			
			thresholder->SetLowerThreshold( -1000.0 );
			thresholder->SetUpperThreshold(     0.0 );
			
			thresholder->SetOutsideValue(  0  );
			thresholder->SetInsideValue(  255 );
			
			//thresholdSegmentation
			try
			{
				thresholder->Update();
			}
			catch( itk::ExceptionObject & excp)
			{
				std::cerr << "Error run this filter." << std::endl;
				std::cerr << excp << std::endl;
				return;
			}
			
			// output
			typename InputImageType::PixelContainer * container;
			
			container =thresholder->GetOutput()->GetPixelContainer();
			container->SetContainerManageMemory( false );
			
			typedef TInputPixelType InputPixelType;
			InputPixelType * output1d = container->GetImportPointer();
			
			setPluginOutputAndDisplayUsingGlobalSetting(output1d, nx, ny, nz, 1, callback);
			
		}
		
		std::cout << std::endl;
		std::cout << "Max. no. iterations: " << thresholdSegmentation->GetNumberOfIterations() << std::endl;
		std::cout << "Max. RMS error: " << thresholdSegmentation->GetMaximumRMSError() << std::endl;
		std::cout << std::endl;
		std::cout << "No. elpased iterations: " << thresholdSegmentation->GetElapsedIterations() << std::endl;
		std::cout << "RMS change: " << thresholdSegmentation->GetRMSChange() << std::endl;
		
		
	}	
		
	
};

#define EXECUTE( v3d_pixel_type, input_pixel_type, output_pixel_type ) \
	case v3d_pixel_type: \
	{ \
		ITKThresholdSegmentationSpecializaed< input_pixel_type, output_pixel_type > runner; \
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



void ITKThresholdSegmentationPlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
    if (menu_name == QObject::tr("ITK ThresholdSegmentation"))
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
		QMessageBox::information(parent, "Version info", "ITK Threshold Segmentation 1.0 (2010-June-04): this plugin is developed by Yang Yu.");
	}
}



