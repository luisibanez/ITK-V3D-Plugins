/*****************************************************************************************************
*
* V3D's plug-in interface of QObject
*
* Copyright: Hanchuan Peng (Howard Hughes Medical Institute, Janelia Farm Research Campus).
* The License Information and User Agreement should be seen at http://penglab.janelia.org/proj/v3d .
* 
* 2009-Aug-21
* Last edit: 2010-06-01
* Last edit: 2010-06-02: add setPluginOutputAndDisplayUsingGlobalSetting()
********************************************************************************************************
*/

#ifndef _V3D_INTERFACE_H_
#define _V3D_INTERFACE_H_

#include <QtCore>

#include "basic_4dimage.h"
#include "basic_surf_objs.h"
#include "basic_landmark.h"
#include "v3d_global_preference.h"
#include "v3d_message.h"

struct V3DPluginArgItem
{
	QString type;	void * p;
};
typedef QList<V3DPluginArgItem> V3DPluginArgList;

typedef QList<QPolygon>        ROIList;
typedef QList<LocationSimple>  LandmarkList;
//typedef QList<ImageMarker>     MarkerList;
//typedef QList<LabelSurf>       LabelSurfList;
//typedef QList<NeuronSWC>       SWCList;
//typedef QList<CellAPO>         APOList;

typedef void* v3dhandle;
typedef QList<v3dhandle>       v3dhandleList;

class V3DPluginCallback
{
public:
	virtual ~V3DPluginCallback() {}
	virtual bool callPluginFunc(const QString & plugin_name, const QString & func_name,
			const V3DPluginArgList & input, V3DPluginArgList & output) = 0;

	virtual v3dhandleList getImageWindowList() const = 0;
	virtual v3dhandle currentImageWindow() = 0;
	virtual v3dhandle newImageWindow(QString name="new_image") = 0;
	virtual void updateImageWindow(v3dhandle image_window) = 0;

	virtual QString getImageName(v3dhandle image_window) const = 0;
	virtual void setImageName(v3dhandle image_window, QString name) = 0;

	virtual Image4DSimple * getImage(v3dhandle image_window) = 0;
	virtual bool setImage(v3dhandle image_window, Image4DSimple * image) = 0;

	virtual LandmarkList  getLandmark(v3dhandle image_window) = 0;
	virtual bool setLandmark(v3dhandle image_window, LandmarkList & landmark_list) = 0;

	virtual ROIList getROI(v3dhandle image_window) = 0;
	virtual bool setROI(v3dhandle image_window, ROIList & roi_list) = 0;

	virtual NeuronTree getSWC(v3dhandle image_window) = 0;
	virtual bool setSWC(v3dhandle image_window, NeuronTree & nt) = 0;
	
	virtual V3D_GlobalSetting getGlobalSetting() = 0;
	virtual bool setGlobalSetting( V3D_GlobalSetting & gs ) = 0;
};

//this is the major V3D plugin interface, and will be enhanced continuously
class V3DPluginInterface
{
public:
	virtual ~V3DPluginInterface() {}

	virtual QStringList menulist() const = 0;
	virtual void domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent) = 0;

	virtual QStringList funclist() const = 0;
	virtual void dofunc(const QString & func_name,
			const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent) = 0;
};


inline QList<V3DLONG> getChannelListForProcessingFromGlobalSetting( V3DLONG nc, V3DPluginCallback & callback ) //nc is the # of channels in an image
{
	QList<V3DLONG> chlist;
	if ( nc<=0 )
	{
		v3d_msg(QString("Invalid # channels parameter to getChannelListForProcessingFromGlobalSetting()."));
		return chlist;
	}
	
	//get the list of channels for processing
	
	int chano_preference = callback.getGlobalSetting().iChannel_for_plugin;
	if ( chano_preference >= nc )
	{
		v3d_msg(QString("The global setting uses a channel id that is bigger than the # of channels of this image. Apply to processing to the last channel of this image."));
		chano_preference = nc-1;
	}
	
	if (chano_preference < 0)
	{
		for (V3DLONG i=0;i<nc;i++) 
			chlist << i;
	}
	else {
		chlist << chano_preference;
	}
	
	return chlist;	
}


inline QList<V3D_Image3DBasic> getChannelDataForProcessingFromGlobalSetting( Image4DSimple * p, V3DPluginCallback & callback)
{
	QList<V3D_Image3DBasic> dlist;
	if ( !p || !p->valid() )
	{
		v3d_msg(QString("Invalid inputs to getChannelDataForProcessingFromGlobalSetting(). Don't output the plugin results.\n"));
		return dlist;
	}
	
	//get the list of channels for processing
	QList<V3DLONG> chlist = getChannelListForProcessingFromGlobalSetting( p->getCDim(), callback );
	for (V3DLONG i=0; i<chlist.size(); i++)
	{
		V3D_Image3DBasic v;
		v.setData(p, chlist.at(i));
		dlist << v;
	}
	return dlist;
}

template <class T> bool setPluginOutputAndDisplayUsingGlobalSetting(T * pluginoutputimg1d, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, V3DPluginCallback & callback)
{
	if (!pluginoutputimg1d || sz0<=0 || sz1<=0 || sz2<=0 || sz3<=0 )
	{
		v3d_msg(QString("Invalid inputs to setPluginOutputAndDisplayUsingGlobalSetting(). Don't output the plugin results.\n"));
		return false;
	}
	
	V3DLONG totalunits = sz0*sz1*sz2*sz3;
	if (totalunits<=0) 
	{
		v3d_msg(QString("Overflow of the *long* data type. Don't output the plugin results.\n"));
		return false;
	}
	
	V3D_GlobalSetting gs = callback.getGlobalSetting();
	
	unsigned char * output1d = 0; 
	V3DLONG i;

	if ( gs.b_plugin_outputImgRescale ) //rescale to [0, 255]
	{
		T mm = pluginoutputimg1d[0], MM = pluginoutputimg1d[0];
		for (i=0; i<totalunits; ++i)
		{
			if (pluginoutputimg1d[i]<mm) 
				mm = pluginoutputimg1d[i];
			else if (pluginoutputimg1d[i]>MM)
				MM = pluginoutputimg1d[i];
		}
		
		if ( gs.b_plugin_outputImgConvert2UINT8 || mm==MM ) //if mm=MM, then no need to use float even the origianl data is float
		{
			output1d = new unsigned char [totalunits];
			if (mm==MM)
				for (i=0; i<totalunits; ++i)
					output1d[i] = 0;
			else
			{
				double w = 255.0/(double(MM)-double(mm));
				for (i=0; i<totalunits; ++i)
					output1d[i] = (unsigned char) ((double)pluginoutputimg1d[i] * w);			
			}	
		}
		else //not convert to unsigned char, but to float
		{
			float * output1d_float = new float [totalunits];
			output1d = (unsigned char *)output1d_float;

			double w = 255.0/(double(MM)-double(mm)); //MM must not equal mm now
			for (i=0; i<totalunits; ++i)
				output1d_float[i] = (float) (((double)pluginoutputimg1d[i]-double(mm)) * w);			
		}
	}		
	else //not rescale to [0, 255]
	{
		if ( gs.b_plugin_outputImgConvert2UINT8 )
		{
			output1d = new unsigned char [totalunits];
			for (i=0; i<totalunits; ++i)
				output1d[i] = (unsigned char)(pluginoutputimg1d[i]);			
		}
		else //not convert to unsigned char, but to float
		{
			float * output1d_float = new float [totalunits];
			output1d = (unsigned char *)output1d_float;
			
			for (i=0; i<totalunits; ++i)
				output1d_float[i] = (float)(pluginoutputimg1d[i]);			
		}
	}
	
	//now set up the output window and data
	
	Image4DSimple p4DImage;
	if ( gs.b_plugin_outputImgConvert2UINT8 )
	{
		p4DImage.setData(output1d, sz0, sz1, sz2, sz3, V3D_UINT8);
	}
	else 
	{
		p4DImage.setData(output1d, sz0, sz1, sz2, sz3, V3D_FLOAT32);		
	}

	v3dhandle mywin = ( gs.b_plugin_dispResInNewWindow ) ? callback.newImageWindow() : callback.currentImageWindow();

	callback.setImage(mywin, &p4DImage);
	callback.setImageName(mywin, QString("plugin_output_image"));
	callback.updateImageWindow(mywin);
	
	return true;
}

// obsolete interface for manipulating only one image at a time. 
class V3DSingleImageInterface
{
public:
    virtual ~V3DSingleImageInterface() {}

    virtual QStringList menulist() const = 0;
    virtual void processImage(const QString & menu_name, Image4DSimple * image, QWidget * parent) = 0;
};


QT_BEGIN_NAMESPACE

	Q_DECLARE_INTERFACE(V3DPluginInterface, "com.janelia.v3d.V3DPluginInterface/1.0");

	Q_DECLARE_INTERFACE(V3DSingleImageInterface, "com.janelia.v3d.V3DSingleImageInterface/1.0");

QT_END_NAMESPACE

#endif /* _V3D_INTERFACE_H_ */

