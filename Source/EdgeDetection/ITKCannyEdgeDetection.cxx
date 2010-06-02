/* ITKCannyEdgeDetection.cpp
 * 2010-06-02: create this program by Yang Yu
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "ITKCannyEdgeDetection.h"

// ITK Header Files
#include "itkCannyEdgeDetectionImageFilter.h"
#include "itkImportImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkImageFileWriter.h"
#include "itkCastImageFilter.h"

/** \class CannyEdgeDetectionImageFilter
 *
 * This filter is an implementation of a Canny edge detector for scalar-valued
 * images.  Based on John Canny's paper "A Computational Approach to Edge 
 * Detection"(IEEE Transactions on Pattern Analysis and Machine Intelligence, 
 * Vol. PAMI-8, No.6, November 1986),  there are four major steps used in the 
 * edge-detection scheme:
 * (1) Smooth the input image with Gaussian filter.
 * (2) Calculate the second directional derivatives of the smoothed image. 
 * (3) Non-Maximum Suppression: the zero-crossings of 2nd derivative are found,
 *     and the sign of third derivative is used to find the correct extrema. 
 * (4) The hysteresis thresholding is applied to the gradient magnitude
 *      (multiplied with zero-crossings) of the smoothed image to find and 
 *      link edges.
 *
 * \par Inputs and Outputs
 * The input to this filter should be a scalar, real-valued Itk image of
 * arbitrary dimension.  The output should also be a scalar, real-value Itk
 * image of the same dimensionality.
 *
 * \par Parameters
 * There are four parameters for this filter that control the sub-filters used
 * by the algorithm.
 *
 * \par 
 * Variance and Maximum error are used in the Gaussian smoothing of the input
 * image.  See  itkDiscreteGaussianImageFilter for information on these
 * parameters.
 *
 * \par
 * Threshold is the lowest allowed value in the output image.  Its data type is 
 * the same as the data type of the output image. Any values below the
 * Threshold level will be replaced with the OutsideValue parameter value, whose
 * default is zero.
 * 
 * \todo Edge-linking will be added when an itk connected component labeling
 * algorithm is available.
 *
 * \sa DiscreteGaussianImageFilter
 * \sa ZeroCrossingImageFilter
 * \sa ThresholdImageFilter
 */

// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(ITKCannyEdgeDetection, ITKCannyEdgeDetectionPlugin)

QStringList ITKCannyEdgeDetectionPlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK Canny Edge Detection")
						 << QObject::tr("about this plugin");
}

template <typename TInputPixelType, typename TOutputPixelType>
class ITKCannyEdgeDetectionImageFilterSpecializaed
{
public:
	void Execute(const QString &arg, Image4DSimple *p4DImage, QWidget *parent)
	{
		typedef TInputPixelType  PixelType;
		
		PixelType * data1d = reinterpret_cast< PixelType * >( p4DImage->getRawData() );
		unsigned long int numberOfPixels = p4DImage->getTotalBytes();
		
		// long pagesz = p4DImage->getTotalUnitNumberPerChannel();
		
		long nx = p4DImage->getXDim();
		long ny = p4DImage->getYDim();
		long nz = p4DImage->getZDim();
		// long sc = p4DImage->getCDim();  // Number of channels
		
		const unsigned int Dimension = 3;
		
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
		importFilter->SetImportPointer( data1d, numberOfPixels, importImageFilterWillOwnTheBuffer );
		
		typedef itk::CastImageFilter< InputImageType, OutputImageType> CastImageFilterType;
		typename CastImageFilterType::Pointer castImageFilter = CastImageFilterType::New();
		
		castImageFilter->SetInput( importFilter->GetOutput() );
		castImageFilter->Update();
		
		typedef itk::CannyEdgeDetectionImageFilter<OutputImageType, OutputImageType> CannyEdgeDetectionType;
		typename CannyEdgeDetectionType::Pointer cannyedgedetectionFilter = CannyEdgeDetectionType::New();
		
		cannyedgedetectionFilter->SetInput( castImageFilter->GetOutput() );
		
		//set Parameters
		// m_Variance m_MaximumError m_Threshold m_UpperThreshold m_LowerThreshold m_OutsideValue
		
		// DiscreteGaussianImageFilter: Variance and Maximum error
		// ZeroCrossingImageFilter
		// ThresholdImageFilter
		
		float var = 10.0;
		float maxerr = 0.5; //Maximum Error Must be in the range [ 0.0 , 1.0 ]
		unsigned char th = 25;
		
		cannyedgedetectionFilter->SetVariance(var);
		cannyedgedetectionFilter->SetMaximumError(maxerr);
		cannyedgedetectionFilter->SetThreshold(th);
		
		
		try
		{
			cannyedgedetectionFilter->Update();
		}
		catch( itk::ExceptionObject & excp)
		{
			std::cerr << "Error run this filter." << std::endl;
			std::cerr << excp << std::endl;
			return;
		}
		
		// output
		typedef itk::RescaleIntensityImageFilter<OutputImageType, InputImageType > RescaleFilterType;
		typename RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
		rescaleFilter->SetInput( cannyedgedetectionFilter->GetOutput() );
		
		typedef itk::ImageFileWriter< InputImageType >  WriterType;
		
		typename WriterType::Pointer writer = WriterType::New();
		writer->SetFileName("output.tif");
		writer->SetInput(rescaleFilter->GetOutput());
		try
		{
			writer->Update();
		}
		catch( itk::ExceptionObject & excp)
		{
			std::cerr << "Error run this filter." << std::endl;
			std::cerr << excp << std::endl;
			return;
		}
		
		//define datatype here
		//
		
		//input
		//update the pixel value
		if(arg == QObject::tr("ITK Canny Edge Detction"))
		{
			ITKCannyEdgeDetectionDialog d(p4DImage, parent);
			
			if (d.exec()!=QDialog::Accepted)
			{
				return;
			}
			else
			{
				try
				{
					cannyedgedetectionFilter->Update();
				}
				catch( itk::ExceptionObject & excp)
				{
					std::cerr << "Error run this filter." << std::endl;
					std::cerr << excp << std::endl;
					return;
				}
				
				// output
				typedef itk::RescaleIntensityImageFilter<OutputImageType, InputImageType > RescaleFilterType;
				typename RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
				rescaleFilter->SetInput( cannyedgedetectionFilter->GetOutput() );
				
				typedef itk::ImageFileWriter< InputImageType >  WriterType;
				
				typename WriterType::Pointer writer = WriterType::New();
				writer->SetFileName("output.tif");
				writer->SetInput(rescaleFilter->GetOutput());
				try
				{
					writer->Update();
				}
				catch( itk::ExceptionObject & excp)
				{
					std::cerr << "Error run this filter." << std::endl;
					std::cerr << excp << std::endl;
					return;
				}
			}
			
		}
		else if (arg == QObject::tr("about this plugin"))
		{
			QMessageBox::information(parent, "Version info", "ITK Canny Edge Detction 1.0 (2010-June-02): this plugin is developed by Yang Yu.");
		}
		else
		{
			return;
		}
	}
	
};

#define EXECUTE( v3d_pixel_type, input_pixel_type, output_pixel_type ) \
	case v3d_pixel_type: \
	{ \
		ITKCannyEdgeDetectionImageFilterSpecializaed< input_pixel_type, output_pixel_type > runner; \
		runner.Execute( arg, p4DImage, parent ); \
		break; \
	} 

#define EXECUTE_ALL_PIXEL_TYPES \
	if (! p4DImage) return; \
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

void ITKCannyEdgeDetectionPlugin::processImage(const QString &arg, Image4DSimple *p4DImage, QWidget *parent)
{
	EXECUTE_ALL_PIXEL_TYPES; 
}


