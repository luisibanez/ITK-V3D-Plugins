//basic_surf_objs.h
//defining the basic types of surface objects
//by Hanchuan Peng. 2009-03-06. This is the first step to unify all basic surface object data types in different modules
//
//090605: merge with p_objectfile.h
//090706: add neuronswc and marker write functions here

#ifndef __BASIC_SURF_OBJS_H__
#define __BASIC_SURF_OBJS_H__

#include "v3d_basicdatatype.h"
#include "color_xyz.h"
#include "v3d_message.h"

#include <QtGui> // this is for QList, QString etc types

// .ano linker files

struct P_ObjectFileType
{
	QStringList raw_image_file_list;
	QStringList labelfield_image_file_list;
	QStringList annotation_file_list;
	QStringList swc_file_list;
	QStringList pointcloud_file_list;
	QStringList surface_file_list;
};

bool importKeywordString2FileType(QString ss, QString vv, QString basedir, P_ObjectFileType & cc);
bool loadAnoFile(QString openFileNameLabel, P_ObjectFileType & cc);
bool saveAnoFile(QString openFileNameLabel, const P_ObjectFileType & cc, const QStringList & commentStrList);
bool saveAnoFile(QString openFileNameLabel, const P_ObjectFileType & cc); // a convenient overloading function for case there is no comments

struct BasicSurfObj
{
	V3DLONG n;				// index
	RGBA8 color;
	bool on;
	bool selected;
	QString name;
	QString comment;
	BasicSurfObj() {n=0; color.r=color.g=color.b=color.a=255; on=true;selected=false; name=comment="";}
};

// .marker marker files
//##########################################################################################
// 090617 RZC : image marker position is 1-based to consist with LocationSimple
// ATTENTION: it is easy to be chaos in 0/1-based coordinates!!!
//##########################################################################################
struct ImageMarker : public BasicSurfObj
{
	int type;			// 0-pxUnknown, 1-pxLocaNotUseful, 2-pxLocaUseful, 3-pxLocaUnsure, 4-pxTemp
	int shape;			// 0-pxUnset, 1-pxSphere, 2-pxCube, 3-pxCircleX, 4-pxCircleY, 5-pxCircleZ,
	// 6-pxSquareX, 7-pxSquareY, 8-pxSquareZ, 9-pxLineX, 10-pxLineY, 11-pxLineZ,
	// 12-pxTriangle, 13-pxDot;
	float x, y, z;		// point coordinates
	float radius;

	operator XYZ() const { return XYZ(x, y, z); }
	ImageMarker() {type=shape=0; radius=x=y=z=0;}
};

QList <ImageMarker> readMarker_file(const QString & filename);
bool wirteMarker_file(const QString & filename, const QList <ImageMarker> & listMarker);


// .apo pointcloud files

struct CellAPO  : public BasicSurfObj
{
	float x, y, z;		// point coordinates
	float intensity;
	float sdev, pixmax, mass;
	float volsize;		// volume size
	QString orderinfo;

	operator XYZ() const { return XYZ(x, y, z); }
	CellAPO() {x=y=z=intensity=volsize=sdev=pixmax=mass=0; orderinfo="";}
};

QList <CellAPO> readAPO_file(const QString& filename);
bool writeAPO_file(const QString& filename, const QList <CellAPO> & listCell);

// .swc neurons and other graph-describing files

struct NeuronSWC : public BasicSurfObj
{
	int type;			// 0-Undefined, 1-Soma, 2-Axon, 3-Dendrite, 4-Apical_dendrite, 5-Fork_point, 6-End_point, 7-Custom
	float x, y, z;		// point coordinates
	float r;			// radius
	V3DLONG pn;				// previous point index (-1 for the first point)

	V3DLONG seg_id, nodeinseg_id; //090925, 091027: for segment editing

	operator XYZ() const { return XYZ(x, y, z); }
	NeuronSWC () {n=type=pn=0; x=y=z=r=0; seg_id=nodeinseg_id=0;}
};

// .v3ds label surfaces

struct LabelSurf : public BasicSurfObj
{
	int label;			// label
	int label2;			// label2 (range from label to label2)

	operator int() const { return label; }
	LabelSurf() {label=label2=0;}
};

// .neuron trees

struct NeuronTree : public BasicSurfObj
{
	QList <NeuronSWC> listNeuron;
	QHash <int, int>  hashNeuron;
	QString file;
	bool editable;

	NeuronTree() {listNeuron.clear(); hashNeuron.clear(); file=""; editable=false;}
	void copy(const NeuronTree & p)
	{
		n=p.n; color=p.color; on=p.on; selected=p.selected; name=p.name; comment=p.comment;
		listNeuron = p.listNeuron;
		hashNeuron = p.hashNeuron;
		file     = p.file;
		editable = p.editable;
	}
	void copyGeometry(const NeuronTree & p)
	{
		if (p.listNeuron.size()!=listNeuron.size()) return;

		NeuronSWC *p_tmp;
		for (int i=0;i<listNeuron.size();i++)
		{
			p_tmp = (NeuronSWC *)(&(listNeuron.at(i)));
			//qDebug()<<"before:"<<p_tmp->x<<p_tmp->y<<p_tmp->z<<p_tmp->r;
			p_tmp->x = p.listNeuron.at(i).x;
			p_tmp->y = p.listNeuron.at(i).y;
			p_tmp->z = p.listNeuron.at(i).z;
			p_tmp->r = p.listNeuron.at(i).r;
			//qDebug()<<"src:"<<p.listNeuron.at(i).x<<p.listNeuron.at(i).y<<p.listNeuron.at(i).z<<p.listNeuron.at(i).r;
			//qDebug()<<"after:"<<p_tmp->x<<p_tmp->y<<p_tmp->z<<p_tmp->r;
		}
	}

};

NeuronTree readSWC_file(const QString& filename);
bool writeSWC_file(const QString& filename, const NeuronTree& nt);

//general operators

inline bool operator==(ImageMarker& a, ImageMarker& b)
{
	return XYZ(a)==XYZ(b);
}

inline bool operator==(CellAPO& a, CellAPO& b)
{
	return XYZ(a)==XYZ(b);
}

inline bool operator==(NeuronSWC& a, NeuronSWC& b)
{
	return XYZ(a)==XYZ(b);
}

inline bool operator==(LabelSurf& a, LabelSurf& b) // for test of contains
{
	return int(a)==int(b);
}
inline bool operator<(LabelSurf& a, LabelSurf& b)  // for test of sort
{
	return int(a)<int(b);
}
inline bool operator>(LabelSurf& a, LabelSurf& b)  // for test of sort
{
	return int(a)>int(b);
}

inline bool operator==(NeuronTree& a, NeuronTree& b)
{
	return QString::compare(a.file, b.file, Qt::CaseSensitive)==0;
}


#endif

