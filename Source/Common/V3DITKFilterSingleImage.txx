
#include "V3DITKFilterSingleImage.h"

template <typename TInputPixelType>
V3DITKFilterSingleImage< TInputPixelType >
::V3DITKFilterSingleImage( V3DPluginCallback * callback )
{
  this->m_V3DPluginCallback = callback;

  this->m_Impor2DFilter = Import2DFilterType::New();
  this->m_Impor3DFilter = Import3DFilterType::New();
}


template <typename TInputPixelType>
V3DITKFilterSingleImage< TInputPixelType >
::~V3DITKFilterSingleImage()
{
}


template <typename TInputPixelType>
V3DITKFilterSingleImage< TInputPixelType >
::Initialize()
{
  this->m_CurrentWindow = this->m_V3DPluginCallback->currentImageWindow();

  this->m_GlobalSetting = this->m_V3DPluginCallback->getGlobalSetting();

  this->m_4DImage = this->m_V3DPluginCallback->getImage( this->m_CurrentWindow );

  this->m_Data1D = reinterpret_cast< PixelType * >( this->m_4DImage->getRawData() );

  this->m_NumberOfPixelsAlongX = p4DImage->getXDim();
  this->m_NumberOfPixelsAlongY = p4DImage->getYDim();
  this->m_NumberOfPixelsAlongZ = p4DImage->getZDim();
  this->m_NumberOfChannels =     p4DImage->getCDim();
      
  this->m_ChannelToFilter = globalSetting.iChannel_for_plugin;

  if( this->m_ChannelToFilter >= this->m_NumberOfChannels )
    {
    v3d_msg(QObject::tr("You are selecting a channel that doesn't exist in this image."));
    return;
    }
 
}


template <typename TInputPixelType>
V3DITKFilterSingleImage< TInputPixelType >
::TransferInput( int channel, int x1, int x2, int y1, int y2, int z1, int z2 )
{
  const unsigned int Dimension = 3;
  
  typedef itk::Image< PixelType, Dimension > ImageType;
  
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
}
      

template <typename TInputPixelType>
V3DITKFilterSingleImage< TInputPixelType >
::TransferOutput()
{
  typename CharImageType::PixelContainer * container;
  
  container = filter_b->GetOutput()->GetPixelContainer();
  container->SetContainerManageMemory( false );
  
  CharPixelType * output1d = container->GetImportPointer();
  
  setPluginOutputAndDisplayUsingGlobalSetting( output1d, nx, ny, nz, 1, this->m_V3DPluginCallback );
}
