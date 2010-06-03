#ifndef __V3DITKFilterSingleImage_H__
#define __V3DITKFilterSingleImage_H__

#include <v3d_interface.h>


template <typename TInputPixelType>
class V3DITKFilterSingleImage
{
public:
  
  typedef TInputPixelType                   InputPixelType;
  typedef itk::Image< InputPixelType, 2 >   Input2DImageType;
  typedef itk::Image< InputPixelType, 3 >   Input3DImageType;

public:
  
  V3DITKFilterSingleImage( V3DPluginCallback * callback );
  virtual ~V3DITKFilterSingleImage();

  virtual void Compute();

  virtual void TransferOutput();

protected:

  virtual void TransferInput( int channel, int x1, int x2, int y1, int y2, int z1, int z2 );
  virtual void ComputeOneRegion();


private:

  typedef itk::ImportImageFilter< InputPixelType, 2 > Import2DFilterType;
  typedef itk::ImportImageFilter< InputPixelType, 3 > Import3DFilterType;

  typename Import2DFilterType::Pointer      m_Impor2DFilter;
  typename Import3DFilterType::Pointer      m_Impor3DFilter;

  V3DPluginCallback  *                      m_V3DPluginCallback;

  v3dhandle                                 m_CurrentWindow;

  V3D_GlobalSetting                         m_GlobalSetting;

  Image4DSimple *                           m_4DImage;

  InputPixelType *                          m_Data1D;
      
  V3DLONG                                   m_NumberOfPixelsAlongX;
  V3DLONG                                   m_NumberOfPixelsAlongY;
  V3DLONG                                   m_NumberOfPixelsAlongZ;
  V3DLONG                                   m_NumberOfChannels;
 
  int                                       m_ChannelToFilter;
};

#endif
