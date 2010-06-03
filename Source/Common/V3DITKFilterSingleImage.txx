#ifndef __V3DITKFilterSingleImage_TXX__
#define __V3DITKFilterSingleImage_TXX__

#include "V3DITKFilterSingleImage.h"

// DEBUG
#include "itkImageFileWriter.h"
// DEBUG


template <typename TInputPixelType, typename TOutputPixelType>
V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >
::V3DITKFilterSingleImage( V3DPluginCallback * callback )
{
  this->m_V3DPluginCallback = callback;

  this->m_Impor2DFilter = Import2DFilterType::New();
  this->m_Impor3DFilter = Import3DFilterType::New();
}


template <typename TInputPixelType, typename TOutputPixelType>
V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >
::~V3DITKFilterSingleImage()
{
}


template <typename TInputPixelType, typename TOutputPixelType>
void
V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >
::Initialize()
{
  this->m_CurrentWindow = this->m_V3DPluginCallback->currentImageWindow();

  this->m_GlobalSetting = this->m_V3DPluginCallback->getGlobalSetting();

  this->m_4DImage = this->m_V3DPluginCallback->getImage( this->m_CurrentWindow );

  this->m_Data1D = reinterpret_cast< InputPixelType * >( this->m_4DImage->getRawData() );

  this->m_NumberOfPixelsAlongX = this->m_4DImage->getXDim();
  this->m_NumberOfPixelsAlongY = this->m_4DImage->getYDim();
  this->m_NumberOfPixelsAlongZ = this->m_4DImage->getZDim();
  this->m_NumberOfChannels =     this->m_4DImage->getCDim();

  std::cout << "Nx = " << this->m_NumberOfPixelsAlongX << std::endl;
  std::cout << "Ny = " << this->m_NumberOfPixelsAlongY << std::endl;
  std::cout << "Nz = " << this->m_NumberOfPixelsAlongZ << std::endl;
  std::cout << "Nc = " << this->m_NumberOfChannels     << std::endl;
}


template <typename TInputPixelType, typename TOutputPixelType>
const typename V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >::Input2DImageType *
V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >
::GetInput2DImage() const
{
  return this->m_Impor2DFilter->GetOutput();
}


template <typename TInputPixelType, typename TOutputPixelType>
const typename V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >::Input3DImageType *
V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >
::GetInput3DImage() const
{
  return this->m_Impor3DFilter->GetOutput();
}


template <typename TInputPixelType, typename TOutputPixelType>
void
V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >
::TransferInput( InputPixelType * inputBuffer, V3DLONG x1, V3DLONG x2, V3DLONG y1, V3DLONG y2, V3DLONG z1, V3DLONG z2 )
{
  typename Import3DFilterType::SizeType size;
  size[0] = x2 - x1;
  size[1] = y2 - y1;
  size[2] = z2 - z1;
  
  typename Import3DFilterType::IndexType start;
  start[0] = x1;
  start[1] = y1;
  start[2] = z1;
  
  typename Import3DFilterType::RegionType region;
  region.SetIndex( start );
  region.SetSize(  size  );
  
  std::cout << "Region = " << region  << std::endl;

  this->m_Impor3DFilter->SetRegion( region );
  
  region.SetSize( size );
  
  typename Input3DImageType::PointType origin;
  origin.Fill( 0.0 );
  
  this->m_Impor3DFilter->SetOrigin( origin );
  
  
  typename Import3DFilterType::SpacingType spacing;
  spacing.Fill( 1.0 );
  
  this->m_Impor3DFilter->SetSpacing( spacing );
  
  const unsigned int numberOfPixels = region.GetNumberOfPixels();

  const bool importImageFilterWillOwnTheBuffer = false;

  this->m_Impor3DFilter->SetImportPointer( inputBuffer, numberOfPixels, importImageFilterWillOwnTheBuffer );

  this->m_Impor3DFilter->Update();

  this->m_Impor3DFilter->GetOutput()->Print( std::cout );

  // DEBUG
  typedef itk::ImageFileWriter< Input3DImageType > WriterType;
  typename WriterType::Pointer writer = WriterType::New();
  writer->SetFileName("importedImage.mha");
  writer->SetInput( this->m_Impor3DFilter->GetOutput() );
  writer->Update();
  // DEBUG
}
      

template <typename TInputPixelType, typename TOutputPixelType>
void
V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >
::Compute()
{
  this->Initialize();

  const V3DLONG x1 = 0;
  const V3DLONG y1 = 0;
  const V3DLONG z1 = 0;

  const V3DLONG x2 = this->m_NumberOfPixelsAlongX;
  const V3DLONG y2 = this->m_NumberOfPixelsAlongY;
  const V3DLONG z2 = this->m_NumberOfPixelsAlongZ;

  QList< V3D_Image3DBasic > imageList = 
    getChannelDataForProcessingFromGlobalSetting( this->m_4DImage, *(this->m_V3DPluginCallback) );

  const unsigned int numberOfChannelsToProcess = imageList.size();

  for( unsigned int channel = 0; channel < numberOfChannelsToProcess; channel++ )
    {
    V3D_Image3DBasic inputImage = imageList.at(channel);

    InputPixelType * inputBuffer = reinterpret_cast<InputPixelType *>( inputImage.data1d );

    std::cout << "channel = " << channel << std::endl;

    this->TransferInput( inputBuffer, x1, x2, y1, y2, z1, z2 );

    this->ComputeOneRegion();

    this->TransferOutput();
    }
}


template <typename TInputPixelType, typename TOutputPixelType>
void
V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >
::SetOutputImage( Output3DImageType * image )
{
  this->m_Output3DImage = image;
}


template <typename TInputPixelType, typename TOutputPixelType>
void
V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >
::SetOutputImage( Output2DImageType * image )
{
  this->m_Output2DImage = image;
}



template <typename TInputPixelType, typename TOutputPixelType>
void
V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >
::TransferOutput() const
{
  typedef typename Output3DImageType::PixelContainer  PixelContainer3DType;

  PixelContainer3DType * container = this->m_Output3DImage->GetPixelContainer();

  container->SetContainerManageMemory( false );
  
  OutputPixelType * output1d = container->GetImportPointer();
  
  bool transferResult = setPluginOutputAndDisplayUsingGlobalSetting(
    output1d, 
    this->m_NumberOfPixelsAlongX,
    this->m_NumberOfPixelsAlongY,
    this->m_NumberOfPixelsAlongZ,
    1,
    *(this->m_V3DPluginCallback)
    );

  if( !transferResult )
    {
    v3d_msg(QObject::tr("Error while transfering output image."));
    }
}

#endif
