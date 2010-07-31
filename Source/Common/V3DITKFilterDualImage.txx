#ifndef __V3DITKFilterDualImage_TXX__
#define __V3DITKFilterDualImage_TXX__

#include "V3DITKFilterDualImage.h"


template <typename TInputPixelType, typename TOutputPixelType>
V3DITKFilterDualImage< TInputPixelType, TOutputPixelType >
::V3DITKFilterDualImage( V3DPluginCallback * callback ):Superclass( callback )
{
  this->m_Impor2DFilter1 = Import2DFilterType::New();
  this->m_Impor2DFilter2 = Import2DFilterType::New();

  this->m_Impor3DFilter1 = Import3DFilterType::New();
  this->m_Impor3DFilter2 = Import3DFilterType::New();
}


template <typename TInputPixelType, typename TOutputPixelType>
V3DITKFilterDualImage< TInputPixelType, TOutputPixelType >
::~V3DITKFilterDualImage()
{
}


template <typename TInputPixelType, typename TOutputPixelType>
const typename V3DITKFilterDualImage< TInputPixelType, TOutputPixelType >::Input2DImageType *
V3DITKFilterDualImage< TInputPixelType, TOutputPixelType >
::GetInput2DImage1() const
{
  return this->m_Impor2DFilter1->GetOutput();
}


template <typename TInputPixelType, typename TOutputPixelType>
const typename V3DITKFilterDualImage< TInputPixelType, TOutputPixelType >::Input2DImageType *
V3DITKFilterDualImage< TInputPixelType, TOutputPixelType >
::GetInput2DImage2() const
{
  return this->m_Impor2DFilter2->GetOutput();
}


template <typename TInputPixelType, typename TOutputPixelType>
const typename V3DITKFilterDualImage< TInputPixelType, TOutputPixelType >::Input3DImageType *
V3DITKFilterDualImage< TInputPixelType, TOutputPixelType >
::GetInput3DImage1() const
{
  return this->m_Impor3DFilter1->GetOutput();
}


template <typename TInputPixelType, typename TOutputPixelType>
const typename V3DITKFilterDualImage< TInputPixelType, TOutputPixelType >::Input3DImageType *
V3DITKFilterDualImage< TInputPixelType, TOutputPixelType >
::GetInput3DImage2() const
{
  return this->m_Impor3DFilter2->GetOutput();
}


template <typename TInputPixelType, typename TOutputPixelType>
void
V3DITKFilterDualImage< TInputPixelType, TOutputPixelType >
::TransferInputImages( V3DPluginCallback * callback )
{
  //get image pointers
  v3dhandleList wndlist = callback->getImageWindowList();
  if(wndlist.size()<2)
    {
    v3d_msg(QObject::tr("Registration need at least two images!"));
    return;
    }

  Image4DSimple* p4DImage_fix = callback->getImage(wndlist[0]);
  Image4DSimple* p4DImage_mov = callback->getImage(wndlist[1]);

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


  //get global setting
  V3D_GlobalSetting globalSetting = callback->getGlobalSetting();
    int channelToFilter = globalSetting.iChannel_for_plugin;
    if( channelToFilter >= p4DImage_fix->getCDim())
  {
    v3d_msg(QObject::tr("You are selecting a channel that doesn't exist in this image."));
    return;
  }


  //------------------------------------------------------------------
  //import images from V3D

  //set ROI region
  typename Import3DFilterType::RegionType region;
  typename Import3DFilterType::IndexType start;
  start.Fill(0);

  typename Import3DFilterType::SizeType size;
  size[0] = p4DImage_fix->getXDim();
  size[1] = p4DImage_fix->getYDim();
  size[2] = p4DImage_fix->getZDim();

  region.SetIndex(start);
  region.SetSize(size);

  this->m_Impor3DFilter1->SetRegion(region);
  this->m_Impor3DFilter2->SetRegion(region);

  //set image Origin
  typename Input3DImageType::PointType origin;
  origin.Fill(0.0);

  this->m_Impor3DFilter1->SetOrigin(origin);
  this->m_Impor3DFilter2->SetOrigin(origin);

  //set spacing
  typename Import3DFilterType::SpacingType spacing;
  spacing.Fill(1.0);

  this->m_Impor3DFilter1->SetSpacing(spacing);
  this->m_Impor3DFilter2->SetSpacing(spacing);

  //set import image pointer
  TInputPixelType * data1d_fix = reinterpret_cast< TInputPixelType * > (p4DImage_fix->getRawData());
  TInputPixelType * data1d_mov = reinterpret_cast< TInputPixelType * > (p4DImage_mov->getRawData());

  unsigned long int numberOfPixels = p4DImage_fix->getTotalBytes();
  const bool importImageFilterWillOwnTheBuffer = false;

  this->m_Impor3DFilter1->SetImportPointer(data1d_fix, numberOfPixels,importImageFilterWillOwnTheBuffer);
  this->m_Impor3DFilter2->SetImportPointer(data1d_mov, numberOfPixels,importImageFilterWillOwnTheBuffer);

  this->m_Impor3DFilter1->Update();
  this->m_Impor3DFilter2->Update();
}



template <typename TInputPixelType, typename TOutputPixelType>
void
V3DITKFilterDualImage< TInputPixelType, TOutputPixelType >
::Compute()
{
  this->Initialize();

  QList< V3D_Image3DBasic > inputImageList =
    getChannelDataForProcessingFromGlobalSetting( this->m_4DImage, *(this->m_V3DPluginCallback) );

  QList< V3D_Image3DBasic > outputImageList;

  const unsigned int numberOfChannelsToProcess = inputImageList.size();
  if (numberOfChannelsToProcess<=0)
    {
    return;
    }

  for( unsigned int channel = 0; channel < numberOfChannelsToProcess; channel++ )
    {
    const V3D_Image3DBasic inputImage = inputImageList.at(channel);

    this->TransferInputImages( this->m_V3DPluginCallback );

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

#endif
