#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "EdgePotential.h"
#include "V3DITKFilterSingleImage.h"

// ITK Header Files
#include "itkEdgePotentialImageFilter.h"
#include "itkGradientRecursiveGaussianImageFilter.h"


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(EdgePotential, EdgePotentialPlugin)


QStringList EdgePotentialPlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK EdgePotential")
            << QObject::tr("about this plugin");
}

QStringList EdgePotentialPlugin::funclist() const
{
    return QStringList();
}


template <typename TPixelType>
class PluginSpecialized : public V3DITKFilterSingleImage< TPixelType, TPixelType >
{
  typedef V3DITKFilterSingleImage< TPixelType, TPixelType >   Superclass;
  typedef typename Superclass::Input3DImageType               ImageType;

  typedef itk::Image< itk::CovariantVector< float, 3 >, 3 >   GradientImageType;

  typedef itk::GradientRecursiveGaussianImageFilter< ImageType, GradientImageType > GradientFilterType;
  typedef itk::EdgePotentialImageFilter< GradientImageType, ImageType > EdgePotentialFilterType;

public:

  PluginSpecialized( V3DPluginCallback * callback ): Superclass(callback)
    {
    this->m_GradientFilter = GradientFilterType::New();
    this->m_EdgePotentialFilter = EdgePotentialFilterType::New();

    this->RegisterInternalFilter( this->m_GradientFilter, 0.8 );
    this->RegisterInternalFilter( this->m_EdgePotentialFilter, 0.2 );
    }

  virtual ~PluginSpecialized() {};


  void Execute(const QString &menu_name, QWidget *parent)
    {
    this->Compute();
    }

  virtual void ComputeOneRegion()
    {

    this->m_GradientFilter->SetInput( this->GetInput3DImage() );
    this->m_EdgePotentialFilter->SetInput( this->m_GradientFilter->GetOutput() );

    this->m_EdgePotentialFilter->Update();

    this->SetOutputImage( this->m_EdgePotentialFilter->GetOutput() );
    }


private:

    typename EdgePotentialFilterType::Pointer  m_EdgePotentialFilter;
    typename GradientFilterType::Pointer       m_GradientFilter;

};


#define EXECUTE_PLUGIN_FOR_ONE_IMAGE_TYPE( v3d_pixel_type, c_pixel_type ) \
  case v3d_pixel_type: \
    { \
    PluginSpecialized< c_pixel_type > runner( &callback ); \
    runner.Execute( menu_name, parent ); \
    break; \
    }


void EdgePotentialPlugin::dofunc(const QString & func_name,
    const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent)
{
  // empty by now
}


void EdgePotentialPlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
  if (menu_name == QObject::tr("about this plugin"))
    {
    QMessageBox::information(parent, "Version info", "ITK EdgePotential 1.0 (2010-Jun-21): this plugin is developed by Luis Ibanez.");
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

