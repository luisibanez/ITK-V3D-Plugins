/* updatepxlvalplugin.cpp
 * 2010-05-12: create this program by Luis Ibanez
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "GradientAnisotropicDiffusion.h"

// ITK Header Files
#include "itkImportImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkGradientAnisotropicDiffusionImageFilter.h"
#include "itkImageFileWriter.h"

// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(ITKInvertIntensity, ITKInvertIntensityPlugin)

QStringList ITKInvertIntensityPlugin::menulist() const
{
	return QStringList() << QObject::tr("ITK GradientAnisotropicDiffusion Filter ...");
}

template<typename TPixelType>
class ITKInvertIntensitySpecializaed
{
public:
	void Execute(const QString &arg, Image4DSimple *p4DImage, QWidget *parent)
	{
		const unsigned int Dimension = 2;

		//------------------------------------------------------------------
		//import image from V3D
		typedef TPixelType PixelType;
		typedef itk::Image< PixelType,  Dimension > InputImageType;
		typedef itk::ImportImageFilter<PixelType, Dimension> ImportFilterType;

		typename ImportFilterType::Pointer importFilter = ImportFilterType::New();

		//set ROI region
		typename ImportFilterType::RegionType region;
		typename ImportFilterType::IndexType start;
		start.Fill(0);
		typename ImportFilterType::SizeType size;
		size[0] = p4DImage->getXDim();
		size[1] = p4DImage->getYDim();
		size[2] = p4DImage->getZDim();
		region.SetIndex(start);
		region.SetSize(size);
		importFilter->SetRegion(region);

		//set image Origin
		typename InputImageType::PointType origin;
		origin.Fill(0.0);
		importFilter->SetOrigin(origin);
		//set spacing
		typename ImportFilterType::SpacingType spacing;
		spacing.Fill(1.0);
		importFilter->SetSpacing(spacing);

		//set import image pointer
		PixelType * data1d = reinterpret_cast<PixelType *> (p4DImage->getRawData());
		unsigned long int numberOfPixels = p4DImage->getTotalBytes();
		const bool importImageFilterWillOwnTheBuffer = false;
		importFilter->SetImportPointer(data1d, numberOfPixels,importImageFilterWillOwnTheBuffer);

		//------------------------------------------------------------------
		//setup filter: cast datatype to float for anisotropic process
		typedef itk::Image< float, Dimension >   	ImageType_mid;
		typedef itk::RescaleIntensityImageFilter<InputImageType, ImageType_mid > RescaleFilterType;

		typename RescaleFilterType::Pointer rescaler_8u_32f = RescaleFilterType::New();
		rescaler_8u_32f->SetOutputMinimum(   0 );
		rescaler_8u_32f->SetOutputMaximum( 255 );

		//------------------------------------------------------------------
		//setup filter: Gradient Anisotropic Diffusion
		typedef itk::GradientAnisotropicDiffusionImageFilter<ImageType_mid,ImageType_mid> FilterType;
		typename FilterType::Pointer filter = FilterType::New();

		//set paras
		unsigned int numberOfIterations	=50;
		double       timeStep			=0.02;
		const double conductance		=3.0;
		filter->SetNumberOfIterations( numberOfIterations );
		filter->SetTimeStep( timeStep );
		filter->SetConductanceParameter( conductance );
		filter->InPlaceOn(); // Reuse the buffer

		//------------------------------------------------------------------
		//setup filter: cast datatype back to PixelType for output
		typedef itk::RescaleIntensityImageFilter<ImageType_mid,InputImageType> RescaleFilterType_output;

		typename RescaleFilterType_output::Pointer rescaler_32f_8u = RescaleFilterType_output::New();
		rescaler_32f_8u->SetOutputMinimum(   0 );
		rescaler_32f_8u->SetOutputMaximum( 255 );

		//------------------------------------------------------------------
		//setup filter: write processed image to disk
		typedef itk::ImageFileWriter< InputImageType >  WriterType;

		typename WriterType::Pointer writer = WriterType::New();
		writer->SetFileName("/Users/qul/work/v3d_2.0/plugin_demo/ITK_V3D_Plugins-0.1.1-Source/Source/GradientAnisotropicDiffusion/output.bmp");

		//------------------------------------------------------------------
		//build pipeline
		rescaler_8u_32f->SetInput(importFilter->GetOutput());
		filter->SetInput(rescaler_8u_32f->GetOutput());
		rescaler_32f_8u->SetInput(filter->GetOutput());
		writer->SetInput(rescaler_32f_8u->GetOutput());

		//------------------------------------------------------------------
		//update the pixel value
		if (arg == QObject::tr("ITK GradientAnisotropicDiffusion Filter ..."))
		{
			ITKInvertIntensityDialog d(p4DImage, parent);

			if (d.exec() != QDialog::Accepted)
			{
				return;
			}
			else
			{
//				filter->Update();
				writer->Update();
			}

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

void ITKInvertIntensityPlugin::processImage(const QString &arg,
		Image4DSimple *p4DImage, QWidget *parent)
{
	EXECUTE_ALL_PIXEL_TYPES;
}
