#ifndef __V3DITKFilterSingleImage_H__
#define __V3DITKFilterSingleImage_H__

#include <v3d_interface.h>
#include "V3DITKGenericDialog.h"
#include "itkImage.h"
#include "itkImportImageFilter.h"


#define EXECUTE_PLUGIN_FOR_ALL_PIXEL_TYPES \
    ImagePixelType pixelType = p4DImage->getDatatype(); \
    switch( pixelType )  \
      {  \
      EXECUTE_PLUGIN_FOR_ONE_IMAGE_TYPE( V3D_UINT8, unsigned char );  \
      EXECUTE_PLUGIN_FOR_ONE_IMAGE_TYPE( V3D_UINT16, unsigned short int );  \
      EXECUTE_PLUGIN_FOR_ONE_IMAGE_TYPE( V3D_FLOAT32, float );  \
      case V3D_UNKNOWN:  \
        {  \
        }  \
      }

#define EXECUTE_PLUGIN_FOR_INTEGER_PIXEL_TYPES \
    ImagePixelType pixelType = p4DImage->getDatatype(); \
    switch( pixelType )  \
      {  \
      EXECUTE_PLUGIN_FOR_ONE_IMAGE_TYPE( V3D_UINT8, unsigned char );  \
      EXECUTE_PLUGIN_FOR_ONE_IMAGE_TYPE( V3D_UINT16, unsigned short int );  \
      case V3D_UNKNOWN:  \
        {  \
        }  \
      }


#define EXECUTE_PLUGIN_FOR_ALL_INPUT_AND_OUTPUT_PIXEL_TYPES \
    ImagePixelType inputPixelType = p4DImage->getDatatype(); \
    ImagePixelType outputPixelType = p4DImage->getDatatype(); \
    switch( inputPixelType )  \
      {  \
      case V3D_UINT8: \
        switch( outputPixelType )  \
          {  \
          EXECUTE_PLUGIN_FOR_INPUT_AND_OUTPUT_IMAGE_TYPE( V3D_UINT8, V3D_UINT8, unsigned char, unsigned char );  \
          EXECUTE_PLUGIN_FOR_INPUT_AND_OUTPUT_IMAGE_TYPE( V3D_UINT8, V3D_UINT16, unsigned char, unsigned short int );  \
          EXECUTE_PLUGIN_FOR_INPUT_AND_OUTPUT_IMAGE_TYPE( V3D_UINT8, V3D_FLOAT32, unsigned char, float );  \
          case V3D_UNKNOWN:  \
            {  \
            }  \
          } \
      case V3D_UINT16: \
        switch( outputPixelType )  \
          {  \
          EXECUTE_PLUGIN_FOR_INPUT_AND_OUTPUT_IMAGE_TYPE( V3D_UINT16, V3D_UINT8, unsigned short int, unsigned char );  \
          EXECUTE_PLUGIN_FOR_INPUT_AND_OUTPUT_IMAGE_TYPE( V3D_UINT16, V3D_UINT16, unsigned short int, unsigned short int );  \
          EXECUTE_PLUGIN_FOR_INPUT_AND_OUTPUT_IMAGE_TYPE( V3D_UINT16, V3D_FLOAT32, unsigned short int, float );  \
          case V3D_UNKNOWN:  \
            {  \
            }  \
          } \
      case V3D_FLOAT32: \
        switch( outputPixelType )  \
          {  \
          EXECUTE_PLUGIN_FOR_INPUT_AND_OUTPUT_IMAGE_TYPE( V3D_FLOAT32, V3D_UINT8, float, unsigned char );  \
          EXECUTE_PLUGIN_FOR_INPUT_AND_OUTPUT_IMAGE_TYPE( V3D_FLOAT32, V3D_UINT16, float, unsigned short int );  \
          EXECUTE_PLUGIN_FOR_INPUT_AND_OUTPUT_IMAGE_TYPE( V3D_FLOAT32, V3D_FLOAT32, float, float );  \
          case V3D_UNKNOWN:  \
            {  \
            }  \
          } \
      case V3D_UNKNOWN:  \
        {  \
        }  \
      }


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

  void Execute(const QString &menu_name, QWidget *parent);

protected:

  virtual void Compute();
  virtual void Initialize();

  virtual void TransferInput( const V3D_Image3DBasic & inputImage,
    V3DLONG x1, V3DLONG x2, V3DLONG y1, V3DLONG y2, V3DLONG z1, V3DLONG z2 );

  virtual void ComputeOneRegion() = 0;  //this needs to be implemented for new plugin code
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


#include "V3DITKFilterSingleImage.txx"

#endif
