#include "InvertIntensity.h"
#include "itkInvertIntensityImageFilter.h"


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(InvertIntensity, InvertIntensityPlugin)


QStringList InvertIntensityPlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK Invert Intensity")
						<< QObject::tr("about this plugin");
}

template <typename TPixelType>
class InvertIntensitySpecialized : V3DITKFilterSingleImage< TPixelType, TPixelType >
{
public:
  typedef V3DITKFilterSingleImage< TPixelType, TPixelType >    Superclass;

  InvertIntensitySpecialized( V3DPluginCallback * callback ): Superclass(callback) {}
  virtual ~InvertIntensitySpecialized() {};

  
  void Execute(const QString &menu_name, QWidget *parent)
    {
    this->Compute(); 
    }

  virtual void ComputeOneRegion()
    {
    typedef TPixelType  PixelType;

    typedef typename Superclass::Input3DImageType   ImageType;

    typedef itk::InvertIntensityImageFilter< ImageType, ImageType > InvertFilterType;
    typename InvertFilterType::Pointer filter = InvertFilterType::New();

    filter->SetInput( this->GetInput3DImage() );
    filter->InPlaceOn();
    filter->Update();

    this->SetOutputImage( filter->GetOutput() );
    }
	
  virtual void SetupParameters()
	{
	}
};

void InvertIntensityPlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
  if (menu_name == QObject::tr("about this plugin"))
    {
    QMessageBox::information(parent, "Version info", "ITK Invert Intensity 1.0 (2010-May-12): this plugin is developed by Luis Ibanez.");
    return;
    }

  if( this->Initialize( callback ) )
    {
    EXECUTE_ALL_PIXEL_TYPES( InvertIntensity );
    }
}

