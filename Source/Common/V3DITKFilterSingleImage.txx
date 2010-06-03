#ifndef __V3DITKFilterSingleImage_TXX__
#define __V3DITKFilterSingleImage_TXX__

#include "V3DITKFilterSingleImage.h"

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
      
  this->m_ChannelToFilter = this->m_GlobalSetting.iChannel_for_plugin;

  if( this->m_ChannelToFilter >= this->m_NumberOfChannels )
    {
    v3d_msg(QObject::tr("You are selecting a channel that doesn't exist in this image."));
    return;
    }
 
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
::TransferInput( int channel, int x1, int x2, int y1, int y2, int z1, int z2 )
{
  typename Import3DFilterType::SizeType size;
  size[0] = x2 - x1 + 1;
  size[1] = y2 - y1 + 1;
  size[2] = z2 - z1 + 1;
  
  typename Import3DFilterType::IndexType start;
  start[0] = x1;
  start[1] = y1;
  start[2] = z1;
  
  typename Import3DFilterType::RegionType region;
  region.SetIndex( start );
  region.SetSize(  size  );
  
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

  this->m_Impor3DFilter->SetImportPointer( this->m_Data1D, numberOfPixels, importImageFilterWillOwnTheBuffer );

}
      

template <typename TInputPixelType, typename TOutputPixelType>
void
V3DITKFilterSingleImage< TInputPixelType, TOutputPixelType >
::Compute()
{
  this->Initialize();

  const int x1 = 0;
  const int y1 = 0;
  const int z1 = 0;

  const int x2 = this->m_NumberOfPixelsAlongX;
  const int y2 = this->m_NumberOfPixelsAlongY;
  const int z2 = this->m_NumberOfPixelsAlongZ;

  if( this->m_ChannelToFilter < 0 )
    {
    std::cout << "Processing all channels " << this->m_ChannelToFilter << std::endl;

    for( unsigned int channel = 0; channel < this->m_NumberOfChannels; channel++ )
      {
      // FIXME Use new API
      this->TransferInput( channel, x1, x2, y1, y2, z1, z2 );

      this->ComputeOneRegion();
      this->TransferOutput();
      }
    }
  else
    {
    std::cout << "Processing a single channel # " << this->m_ChannelToFilter << std::endl;
    this->TransferInput( this->m_ChannelToFilter, x1, x2, y1, y2, z1, z2 );

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
