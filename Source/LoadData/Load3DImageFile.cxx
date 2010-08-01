#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "Load3DImageFile.h"
#include "V3DITKFilterSingleImage.h"

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
class PluginSpecialized : public V3DITKFilterSingleImage< TPixelType, TPixelType >
{
  typedef V3DITKFilterSingleImage< TPixelType, TPixelType >   Superclass;
  typedef typename Superclass::Input3DImageType               ImageType;

  typedef itk::ImageFileReader< ImageType > FilterType;

public:

  PluginSpecialized( V3DPluginCallback * callback ): Superclass(callback)
    {
    this->m_Filter = FilterType::New();
    this->RegisterInternalFilter( this->m_Filter, 1.0 );
    }

  virtual ~PluginSpecialized() {};


  void Execute(const QString &menu_name, QWidget *parent)
    {
    V3DITKGenericDialog dialog("Load3DImageFile");

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

    if( !listOfFiles.isEmpty() )
      {
      std::string firstStringInListOfFiles = listOfFiles[0].toStdString();

      this->m_Filter->SetFileName( firstStringInListOfFiles );

      this->Compute();
      }
    }

  virtual void ComputeOneRegion()
    {
    this->m_Filter->Update();

    this->SetOutputImage( this->m_Filter->GetOutput() );
    }


private:

    typename FilterType::Pointer   m_Filter;

};


#define EXECUTE_PLUGIN_FOR_ONE_IMAGE_TYPE( v3d_pixel_type, c_pixel_type ) \
  case v3d_pixel_type: \
    { \
    PluginSpecialized< c_pixel_type > runner( &callback ); \
    runner.Execute( menu_name, parent ); \
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

  v3dhandle curwin = callback.currentImageWindow();
  if (!curwin)
    {
    v3d_msg(tr("You don't have any image open in the main window."));
    return;
    }

  Image4DSimple *p4DImage = callback.getImage(curwin);
  if (! p4DImage)
    {
    v3d_msg(tr("The input image is null."));
    return;
    }

  EXECUTE_PLUGIN_FOR_ALL_PIXEL_TYPES;
}

