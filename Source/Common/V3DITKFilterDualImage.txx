#ifndef __V3DITKFilterDualImage_TXX__
#define __V3DITKFilterDualImage_TXX__

#include "V3DITKFilterDualImage.h"


template <typename TInputPixelType, typename TOutputPixelType>
V3DITKFilterDualImage< TInputPixelType, TOutputPixelType >
::V3DITKFilterDualImage( V3DPluginCallback * callback )
{
  this->m_V3DPluginCallback = callback;

  this->m_Impor2DFilter = Import2DFilterType::New();
  this->m_Impor3DFilter = Import3DFilterType::New();
}


template <typename TInputPixelType, typename TOutputPixelType>
V3DITKFilterDualImage< TInputPixelType, TOutputPixelType >
::~V3DITKFilterDualImage()
{
}


template <typename TInputPixelType, typename TOutputPixelType>
void
V3DITKFilterDualImage< TInputPixelType, TOutputPixelType >
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
}

template <typename TInputPixelType, typename TOutputPixelType>
void
V3DITKFilterDualImage< TInputPixelType, TOutputPixelType >
::Execute(const QString &menu_name, QWidget *parent)
{
  this->Compute(); 
}


template <typename TInputPixelType, typename TOutputPixelType>
const typename V3DITKFilterDualImage< TInputPixelType, TOutputPixelType >::Input2DImageType *
V3DITKFilterDualImage< TInputPixelType, TOutputPixelType >
::GetInput2DImage() const
{
  return this->m_Impor2DFilter->GetOutput();
}


template <typename TInputPixelType, typename TOutputPixelType>
const typename V3DITKFilterDualImage< TInputPixelType, TOutputPixelType >::Input3DImageType *
V3DITKFilterDualImage< TInputPixelType, TOutputPixelType >
::GetInput3DImage() const
{
  return this->m_Impor3DFilter->GetOutput();
}


template <typename TInputPixelType, typename TOutputPixelType>
void
V3DITKFilterDualImage< TInputPixelType, TOutputPixelType >
::TransferInput( const V3D_Image3DBasic & inputImage, V3DLONG x1, V3DLONG x2, V3DLONG y1, V3DLONG y2, V3DLONG z1, V3DLONG z2 )
{
  const InputPixelType * constInputBuffer = reinterpret_cast<InputPixelType *>( inputImage.data1d );

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

  InputPixelType * inputBuffer = const_cast<InputPixelType *>( constInputBuffer );

  this->m_Impor3DFilter->SetImportPointer( inputBuffer, numberOfPixels, importImageFilterWillOwnTheBuffer );

  this->m_Impor3DFilter->Update();
}
      

template <typename TInputPixelType, typename TOutputPixelType>
bool
V3DITKFilterDualImage< TInputPixelType, TOutputPixelType >
::ShouldGenerateNewWindow() const
{
  return this->m_GlobalSetting.b_plugin_dispResInNewWindow;
}


template <typename TInputPixelType, typename TOutputPixelType>
void
V3DITKFilterDualImage< TInputPixelType, TOutputPixelType >
::Compute()
{
  this->Initialize();

  const V3DLONG x1 = 0;
  const V3DLONG y1 = 0;
  const V3DLONG z1 = 0;

  const V3DLONG x2 = this->m_NumberOfPixelsAlongX;
  const V3DLONG y2 = this->m_NumberOfPixelsAlongY;
  const V3DLONG z2 = this->m_NumberOfPixelsAlongZ;

  QList< V3D_Image3DBasic > inputImageList = 
    getChannelDataForProcessingFromGlobalSetting( this->m_4DImage, *(this->m_V3DPluginCallback) );

  QList< V3D_Image3DBasic > outputImageList;

  const unsigned int numberOfChannelsToProcess = inputImageList.size();
  if (numberOfChannelsToProcess<=0)
    return;

  this->SetupParameters();

  for( unsigned int channel = 0; channel < numberOfChannelsToProcess; channel++ )
    {
    const V3D_Image3DBasic inputImage = inputImageList.at(channel);

    this->TransferInput( inputImage, x1, x2, y1, y2, z1, z2 );

    this->ComputeOneRegion();

    V3D_Image3DBasic outputImage;

    outputImage.cid = inputImage.cid;

    this->TransferOutput( outputImage );

    outputImageList.append( outputImage );
    }

  bool transferResult =  assembleProcessedChannels2Image4DClass( outputImageList, *(this->m_V3DPluginCallback) );
    
  if( !transferResult )
    {
    v3d_msg(QObject::tr("Error while transfering output image."));
    }

}


template <typename TInputPixelType, typename TOutputPixelType>
void
V3DITKFilterDualImage< TInputPixelType, TOutputPixelType >
::SetOutputImage( Output3DImageType * image )
{
  this->m_Output3DImage = image;
}


template <typename TInputPixelType, typename TOutputPixelType>
void
V3DITKFilterDualImage< TInputPixelType, TOutputPixelType >
::SetOutputImage( Output2DImageType * image )
{
  this->m_Output2DImage = image;
}



template <typename TInputPixelType, typename TOutputPixelType>
void
V3DITKFilterDualImage< TInputPixelType, TOutputPixelType >
::TransferOutput( V3D_Image3DBasic & outputImage ) const
{
  typedef typename Output3DImageType::PixelContainer  PixelContainer3DType;

  PixelContainer3DType * container = this->m_Output3DImage->GetPixelContainer();

  container->SetContainerManageMemory( false );
  
  OutputPixelType * output1d = container->GetImportPointer();
  
  outputImage.data1d = reinterpret_cast< unsigned char * >( output1d );

  typename Output3DImageType::RegionType region = this->m_Output3DImage->GetBufferedRegion();

  typename Output3DImageType::SizeType size = region.GetSize();

  outputImage.sz0 = size[0];
  outputImage.sz1 = size[1];
  outputImage.sz2 = size[2];


  // 
  //  Set the pixel type id.
  //
  if( typeid(OutputPixelType) == typeid( unsigned char ) )
    {
    outputImage.datatype = V3D_UINT8;
    }
  else if ( typeid(OutputPixelType) == typeid( unsigned short int ) )
    {
    outputImage.datatype = V3D_UINT16;
    }
  else if ( typeid(OutputPixelType) == typeid( float ) )
    {
    outputImage.datatype = V3D_FLOAT32;
    }

}

#endif
