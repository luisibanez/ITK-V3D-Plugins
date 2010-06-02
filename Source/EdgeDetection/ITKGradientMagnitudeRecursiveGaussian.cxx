/* ITKGradientMagnitudeRecursiveGaussian.cpp
 * 2010-06-02: create this program by Yang Yu
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "ITKGradientMagnitudeRecursiveGaussian.h"

// ITK Header Files
#include "itkImage.h"
#include "itkGradientMagnitudeRecursiveGaussianImageFilter.h"
#include "itkImportImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkImageFileWriter.h"

// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(ITKGradientMagnitudeRecursiveGaussian, ITKGradientPlugin)

QStringList ITKGradientPlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK GradientMagnitudeRecursiveGaussian")
						 << QObject::tr("about this plugin");
}

template <typename TInputPixelType, typename TOutputPixelType>
class ITKGradientSpecializaed
{
public:
	void Execute( const QString &arg, Image4DSimple *p4DImage, QWidget *parent)
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
		
		typedef itk::GradientMagnitudeRecursiveGaussianImageFilter<InputImageType, OutputImageType> GradientType;
		typename GradientType::Pointer gradientFilter = GradientType::New();
		
		gradientFilter->SetInput( importFilter->GetOutput() );
		//gradientFilter->InPlaceOn(); // Reuse the buffer
		
		//rescaleFilter->SetInput( importFilter->GetOutput() );
		//gradientFilter->SetInput( rescaleFilter->GetOutput() );
		//gradientFilter->InPlaceOn();
		
		try
		{
			gradientFilter->Update();
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
		rescaleFilter->SetInput( gradientFilter->GetOutput() );
		
		typedef itk::ImageFileWriter< InputImageType >  WriterType;
		
		typename WriterType::Pointer writer = WriterType::New();
		writer->SetFileName("../../../output.tif");
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
		
		
//		typename OutputImageType::PixelContainer * container;
//		
//		container =gradientFilter->GetOutput()->GetPixelContainer();
//		container->SetContainerManageMemory( false );
//		
//		PixelType * output1d = container->GetImportPointer();
		
		
		//define datatype here
		//
		
		//input
		//update the pixel value
		if(arg == QObject::tr("ITK "))
		{
			ITKGradientDialog d(p4DImage, parent);
			
			if (d.exec()!=QDialog::Accepted)
			{
				return;
			}
			else
			{
				gradientFilter->Update();
			}
			
		}
		else if (arg == QObject::tr("about this plugin"))
		{
			QMessageBox::information(parent, "Version info", "ITK Gradient Magnitude Recursive Gaussian 1.0 (2010-June-02): this plugin is developed by Yang Yu.");
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
		ITKGradientSpecializaed< input_pixel_type, output_pixel_type > runner; \
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

void ITKGradientPlugin::processImage(const QString &arg, Image4DSimple *p4DImage, QWidget *parent)
{
	EXECUTE_ALL_PIXEL_TYPES; 
//	
//	Image4DSimple p4DImage;
//	p4DImage.setData((unsigned char*)output1d, nx, ny, nz, 1, V3D_FLOAT32);
//	
//	v3dhandle newwin = callback.newImageWindow();
//	callback.setImage(newwin, &p4DImage);
//	callback.setImageName(newwin, QString("Gradient Image"));
//	callback.updateImageWindow(newwin);
}

