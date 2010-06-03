#ifndef __V3DITKFilterSingleImage_H__
#define __V3DITKFilterSingleImage_H__

#include <v3d_interface.h>
#include "itkImage.h"
#include "itkImportImageFilter.h"


template <typename TInputPixelType, typename TOutputPixelType>
class V3DITKFilterSingleImage
{
public:
  
  typedef TInputPixelType                   InputPixelType;
  typedef itk::Image< InputPixelType, 2 >   Input2DImageType;
  typedef itk::Image< InputPixelType, 3 >   Input3DImageType;

  typedef TOutputPixelType                  OutputPixelType;
  typedef itk::Image< OutputPixelType, 2 >  Output2DImageType;
  typedef itk::Image< OutputPixelType, 3 >  Output3DImageType;

public:
  
  V3DITKFilterSingleImage( V3DPluginCallback * callback );
  virtual ~V3DITKFilterSingleImage();

protected:

  virtual void Compute();
  virtual void Initialize();
  virtual void TransferInput( int channel, int x1, int x2, int y1, int y2, int z1, int z2 );
  virtual void ComputeOneRegion() = 0;
  virtual void TransferOutput() const;

  const Input2DImageType * GetInput2DImage() const;
  const Input3DImageType * GetInput3DImage() const;

  void SetOutputImage( Output2DImageType * image );
  void SetOutputImage( Output3DImageType * image );

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

  typename Output2DImageType::Pointer       m_Output2DImage;
  typename Output3DImageType::Pointer       m_Output3DImage;

};

#include "V3DITKFilterSingleImage.txx"

#endif
