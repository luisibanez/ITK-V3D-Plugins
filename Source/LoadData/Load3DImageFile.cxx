#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "Load3DImageFile.h"
#include "V3DITKFilterNullImage.h"

// ITK Header Files
#include "itkImageFileReader.h"


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(Load3DImageFile, Load3DImageFilePlugin)


QStringList Load3DImageFilePlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK Load3DImageFile")
            << QObject::tr("about this plugin");
}

QStringList Load3DImageFilePlugin::funclist() const
{
    return QStringList();
}


template <typename TPixelType>
class PluginSpecialized : public V3DITKFilterNullImage< TPixelType, TPixelType >
{
  typedef V3DITKFilterNullImage< TPixelType, TPixelType >   Superclass;
  typedef typename Superclass::Input3DImageType               ImageType;

  typedef itk::ImageFileReader< ImageType > FilterType;

public:

  PluginSpecialized( V3DPluginCallback * callback ): Superclass(callback)
    {
    this->m_Filter = FilterType::New();
    this->RegisterInternalFilter( this->m_Filter, 1.0 );
    }

  virtual ~PluginSpecialized() {};


  void Execute(const QString &menu_name, QWidget *parent, const std::string & inputFileName )
    {
    this->m_Filter->SetFileName( inputFileName );
    std::cout << "Execute( " << inputFileName << ") " << std::endl;
    this->Compute();
    }

  virtual void ComputeOneRegion()
    {
    std::cout << "ComputeOneRegion() " << std::endl;
    this->m_Filter->Update();

    std::cout << "SetOutputImage() " << std::endl;
    this->SetOutputImage( this->m_Filter->GetOutput() );
    }


private:

    typename FilterType::Pointer   m_Filter;

};


#define EXECUTE_PLUGIN_FOR_ONE_ITK_IMAGE_TYPE( itk_io_pixel_type, c_pixel_type ) \
  case itk_io_pixel_type: \
    { \
    PluginSpecialized< c_pixel_type > runner( &callback ); \
    runner.Execute( menu_name, parent, inputFileName ); \
    break; \
    }


void Load3DImageFilePlugin::dofunc(const QString & func_name,
    const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent)
{
  // empty by now
}


void Load3DImageFilePlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
  if (menu_name == QObject::tr("about this plugin"))
    {
    QMessageBox::information(parent, "Version info", "ITK Load3DImageFile 1.0 (2010-Aug-1): this plugin is developed by Luis Ibanez.");
    return;
    }

  QFileDialog fileDialog(parent);

  fileDialog.setViewMode( QFileDialog::Detail );

  QStringList filters;
  filters << "Image files (*.hdr)"
          << "Video files (*.mhd)"
          << "Video files (*.mha)"
          << "Video files (*.vtk)"
          << "Video files (*.nii)"
          << "Video files (*.nii.gz)"
          << "Video files (*.nrrd)"
          << "Video files (*.tif*)";

  fileDialog.setFilters( filters );
  fileDialog.setLabelText( QFileDialog::LookIn,"Select Input");

  fileDialog.exec();

  QStringList listOfFiles = fileDialog.selectedFiles();

  if( listOfFiles.isEmpty() )
    {
    return;
    }

  std::string inputFileName = listOfFiles[0].toStdString();

  // Find out the pixel type of the image in file
  typedef itk::ImageIOBase::IOComponentType  ScalarPixelType;

  itk::ImageIOBase::Pointer imageIO =
    itk::ImageIOFactory::CreateImageIO( inputFileName.c_str(),
                                   itk::ImageIOFactory::ReadMode );

  if( !imageIO )
    {
    return;
    }

  std::cout << "Input file name = " << inputFileName << std::endl;

  // Now that we found the appropriate ImageIO class,
  // ask it to read the meta data from the image file.
  imageIO->SetFileName( inputFileName.c_str() );
  imageIO->ReadImageInformation();

  ScalarPixelType pixelType = imageIO->GetComponentType();

  std::cout << "pixelType = " << pixelType << std::endl;

  switch( pixelType )
    {
    // These are the supported pixel types (with acceptable conversions)
    EXECUTE_PLUGIN_FOR_ONE_ITK_IMAGE_TYPE( itk::ImageIOBase::CHAR, unsigned char ); // Conversion
    EXECUTE_PLUGIN_FOR_ONE_ITK_IMAGE_TYPE( itk::ImageIOBase::UCHAR, unsigned char );
    EXECUTE_PLUGIN_FOR_ONE_ITK_IMAGE_TYPE( itk::ImageIOBase::SHORT, unsigned short int ); // Conversion
    EXECUTE_PLUGIN_FOR_ONE_ITK_IMAGE_TYPE( itk::ImageIOBase::USHORT, unsigned short int );
    EXECUTE_PLUGIN_FOR_ONE_ITK_IMAGE_TYPE( itk::ImageIOBase::FLOAT, float );
    EXECUTE_PLUGIN_FOR_ONE_ITK_IMAGE_TYPE( itk::ImageIOBase::DOUBLE, float ); // Conversion

    // There are the unsupported pixel types (they simply get ignored)
    case itk::ImageIOBase::UINT:
    case itk::ImageIOBase::INT:
    case itk::ImageIOBase::ULONG:
    case itk::ImageIOBase::LONG:
    default:
      {
      }
    }

}

