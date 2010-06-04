#ifndef __V3DITKFilterDualImage_H__
#define __V3DITKFilterDualImage_H__

#include <v3d_interface.h>
#include "itkImage.h"
#include "itkImportImageFilter.h"


#define EXECUTE_PLUGIN_FOR_ALL_PIXEL_TYPES \
    ImagePixelType pixelType = p4DImage->getDatatype(); \
    switch( pixelType )  \
      {  \
      EXECUTE_PLUGING_FOR_ONE_IMAGE_TYPE( V3D_UINT8, unsigned char );  \
      EXECUTE_PLUGING_FOR_ONE_IMAGE_TYPE( V3D_UINT16, unsigned short int );  \
      EXECUTE_PLUGING_FOR_ONE_IMAGE_TYPE( V3D_FLOAT32, float );  \
      case V3D_UNKNOWN:  \
        {  \
        }  \
      }  


template <typename TInputPixelType, typename TOutputPixelType>
class V3DITKFilterDualImage
{
public:
  
  typedef TInputPixelType                   InputPixelType;
  typedef itk::Image< InputPixelType, 2 >   Input2DImageType;
  typedef itk::Image< InputPixelType, 3 >   Input3DImageType;

  typedef TOutputPixelType                  OutputPixelType;
  typedef itk::Image< OutputPixelType, 2 >  Output2DImageType;
  typedef itk::Image< OutputPixelType, 3 >  Output3DImageType;

public:
  
  V3DITKFilterDualImage( V3DPluginCallback * callback );
  virtual ~V3DITKFilterDualImage();

  void Execute(const QString &menu_name, QWidget *parent);

protected:

  virtual void Compute();
  virtual void Initialize();

  virtual void TransferInput( const V3D_Image3DBasic & inputImage,
    V3DLONG x1, V3DLONG x2, V3DLONG y1, V3DLONG y2, V3DLONG z1, V3DLONG z2 );

  virtual void ComputeOneRegion() = 0;  //this needs to be implemented for new plugin code
  virtual void SetupParameters() = 0;  //this needs to be implemented for new plugin code
  virtual void TransferOutput( V3D_Image3DBasic & outputImage ) const;

  const Input2DImageType * GetInput2DImage() const;
  const Input3DImageType * GetInput3DImage() const;

  void SetOutputImage( Output2DImageType * image );
  void SetOutputImage( Output3DImageType * image );

  bool ShouldGenerateNewWindow() const;

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
 
  typename Output2DImageType::Pointer       m_Output2DImage;
  typename Output3DImageType::Pointer       m_Output3DImage;

};


#include "V3DITKFilterDualImage.txx"

#endif
