/* DemonsRegistration.cpp
 * 2010-06-03: create this program by Luis Ibanez
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "V3DITKFilterDualImage.h"
#include "DemonsRegistration.h"

// ITK Header Files
#include "itkImportImageFilter.h"
#include "itkDemonsRegistrationFilter.h"
#include "itkWarpImageFilter.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkImage.h"
#include "itkCommand.h"

// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(DemonsRegistration, ITKDemonsRegistrationPlugin)

QStringList ITKDemonsRegistrationPlugin::menulist() const
{
  return QStringList() << QObject::tr("ITK Demons Registration...");
}


template<typename TPixelType>
class PluginSpecialized : public V3DITKFilterDualImage< TPixelType, TPixelType >
{
public:

  typedef V3DITKFilterDualImage< TPixelType, TPixelType >   Superclass;
  typedef typename Superclass::Input3DImageType               Input3DImageType;

  typedef TPixelType PixelType;

  itkStaticConstMacro(Image3Dimension, unsigned int, 3);

  typedef itk::Vector< float, Image3Dimension >    VectorPixelType;
  typedef itk::Image<  VectorPixelType, Image3Dimension > DeformationFieldType;
  typedef itk::DemonsRegistrationFilter<
    Input3DImageType,
    Input3DImageType,
    DeformationFieldType>   RegistrationFilterType;


  class CommandIterationUpdate : public itk::Command 
  {
  public:
    typedef  CommandIterationUpdate   Self;
    typedef  itk::Command             Superclass;
    typedef  itk::SmartPointer<CommandIterationUpdate>  Pointer;
    itkNewMacro( CommandIterationUpdate );
  protected:
    CommandIterationUpdate() {};
    
  public:

    void Execute(itk::Object *caller, const itk::EventObject & event)
      {
      Execute( (const itk::Object *)caller, event);
      }

    void Execute(const itk::Object * object, const itk::EventObject & event)
      {
      static int iteration = 0;

      const RegistrationFilterType * filter = 
        dynamic_cast< const RegistrationFilterType * >( object );
      if( !(itk::IterationEvent().CheckEvent( &event )) )
        {
        return;
        }
      std::cout << "Iteration: " << iteration;
      std::cout << "  Metric : " << filter->GetMetric();
      std::cout << "  RMS Change: " << filter->GetRMSChange() << std::endl;
      }
  };

public:

  PluginSpecialized( V3DPluginCallback * callback ): Superclass(callback)
    {
    this->m_Filter = RegistrationFilterType::New();
    }

  void Execute(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
  {
    //get image pointers
    v3dhandleList wndlist = callback.getImageWindowList();
    if(wndlist.size()<2)
    {
      v3d_msg(QObject::tr("Registration need at least two images!"));
      return;
    }
    v3dhandle oldwin = wndlist[1];
    Image4DSimple* p4DImage_fix=callback.getImage(wndlist[0]);
    Image4DSimple* p4DImage_mov=callback.getImage(wndlist[1]);

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
    V3D_GlobalSetting globalSetting = callback.getGlobalSetting();
      int channelToFilter = globalSetting.iChannel_for_plugin;
      if( channelToFilter >= p4DImage_fix->getCDim())
    {
      v3d_msg(QObject::tr("You are selecting a channel that doesn't exist in this image."));
      return;
    }

    //------------------------------------------------------------------
    //import images from V3D
    typedef itk::ImportImageFilter<PixelType, Image3Dimension> ImportFilterType;

    typename ImportFilterType::Pointer importFilter_fix = ImportFilterType::New();
    typename ImportFilterType::Pointer importFilter_mov = ImportFilterType::New();

    //set ROI region
    typename ImportFilterType::RegionType region;
    typename ImportFilterType::IndexType start;
    start.Fill(0);
    typename ImportFilterType::SizeType size;
    size[0] = p4DImage_fix->getXDim();
    size[1] = p4DImage_fix->getYDim();
    size[2] = p4DImage_fix->getZDim();
    region.SetIndex(start);
    region.SetSize(size);
    importFilter_fix->SetRegion(region);
    importFilter_mov->SetRegion(region);

    //set image Origin
    typename Input3DImageType::PointType origin;
    origin.Fill(0.0);
    importFilter_fix->SetOrigin(origin);
    importFilter_mov->SetOrigin(origin);
    //set spacing
    typename ImportFilterType::SpacingType spacing;
    spacing.Fill(1.0);
    importFilter_fix->SetSpacing(spacing);
    importFilter_mov->SetSpacing(spacing);

    //set import image pointer
    PixelType * data1d_fix = reinterpret_cast<PixelType *> (p4DImage_fix->getRawData());
    PixelType * data1d_mov = reinterpret_cast<PixelType *> (p4DImage_mov->getRawData());
    unsigned long int numberOfPixels = p4DImage_fix->getTotalBytes();
    const bool importImageFilterWillOwnTheBuffer = false;
    importFilter_fix->SetImportPointer(data1d_fix, numberOfPixels,importImageFilterWillOwnTheBuffer);
    importFilter_mov->SetImportPointer(data1d_mov, numberOfPixels,importImageFilterWillOwnTheBuffer);


    // RUN DEMONS FILTER HERE

    this->m_Filter->SetFixedImage( importFilter_fix->GetOutput() );
    this->m_Filter->SetMovingImage( importFilter_mov->GetOutput() );

    this->SetupParameters();

    typename CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();

    this->m_Filter->AddObserver( itk::IterationEvent(), observer );

    this->m_Filter->Update();

    typedef itk::WarpImageFilter<
                            Input3DImageType, 
                            Input3DImageType,
                            DeformationFieldType  >     WarperType;

    typedef itk::LinearInterpolateImageFunction<
                                     Input3DImageType,
                                     double          >  InterpolatorType;

    typename WarperType::Pointer warper = WarperType::New();

    typename InterpolatorType::Pointer interpolator = InterpolatorType::New();
    typename Input3DImageType::Pointer fixedImage = importFilter_fix->GetOutput();

    warper->SetInput( importFilter_mov->GetOutput() );
    warper->SetInterpolator( interpolator );
    warper->SetOutputSpacing( fixedImage->GetSpacing() );
    warper->SetOutputOrigin( fixedImage->GetOrigin() );
    warper->SetOutputDirection( fixedImage->GetDirection() );

#ifdef MANUALLY_COPYING_IMAGE_BACK_TO_V3D
    //------------------------------------------------------------------
    typedef itk::ImageRegionConstIterator<Input3DImageType> IteratorType;
    IteratorType it(caster->GetOutput(), caster->GetOutput()->GetRequestedRegion());
    it.GoToBegin();

//    if(!globalSetting.b_plugin_dispResInNewWindow)
//    {
      printf("display results in a new window\n");
      //copy data back to V3D
      while(!it.IsAtEnd())
      {
        *data1d_mov=it.Get();
        ++it;
        ++data1d_mov;
      }

      callback.setImageName(oldwin, callback.getImageName(oldwin)+"_new");
      callback.updateImageWindow(oldwin);
      }
#endif

  }

  virtual void ComputeOneRegion()
    {
    }

  virtual void SetupParameters()
    {
    //
    // These values should actually be provided by the Qt Dialog...
    //
    this->m_Filter->SetNumberOfIterations( 50 );
    this->m_Filter->SetStandardDeviations( 1.0 );
    this->m_Filter->SetMaximumRMSError( 0.01 );
    }

private:

    typename RegistrationFilterType::Pointer   m_Filter;

};

#define EXECUTE_PLUGING_FOR_ONE_IMAGE_TYPE( v3d_pixel_type, c_pixel_type ) \
  case v3d_pixel_type: \
    { \
    PluginSpecialized< c_pixel_type > runner( &callback ); \
    runner.Execute(  menu_name, callback, parent ); \
    break; \
    } 

void ITKDemonsRegistrationPlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
  if (menu_name == QObject::tr("about this plugin"))
    {
    QMessageBox::information(parent, "Version info", "Demons Registration 1.0 (2010-Jun-3): this plugin is developed by Luis Ibanez.");
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

  EXECUTE_PLUGIN_FOR_ALL_PIXEL_TYPES; // Defined in V3DITKFilterDualImage.h
}
