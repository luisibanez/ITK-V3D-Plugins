/* ITKCannyEdgeDetection.h
 * 2010-06-02: create this program by Yang Yu
 */

#ifndef __ITKCANNYEDGEDETECTION_H__
#define __ITKCANNYEDGEDETECTION_H__

//   Canny edge detection.
//

#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>

class ITKCannyEdgeDetectionPlugin : public QObject, public V3DPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface);
	
public:
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent);
	
	QStringList funclist() const {return QStringList();}
	void dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, QWidget *parent) {}
	
};

class ITKCannyEdgeDetectionDialog : public QDialog
{
	Q_OBJECT
	
public:
	ITKCannyEdgeDetectionDialog(V3DPluginCallback &callback, QWidget *parent)
	{
		Image4DSimple* p4DImage = callback.getImage(callback.currentImageWindow());
		
		if (! p4DImage) return;
		
		printf("Passing data to data1d\n");
		
		ok     = new QPushButton("OK");
		cancel = new QPushButton("Cancel");
		
		gridLayout = new QGridLayout();
		
		gridLayout->addWidget(cancel, 0,0); gridLayout->addWidget(ok, 0,1);
		setLayout(gridLayout);
		setWindowTitle(QString("Gradient Magnitude Recursive Gaussian"));
		
		connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
	}
	
	~ITKCannyEdgeDetectionDialog(){}
	
	public slots:
	
	
public:
	QGridLayout *gridLayout;
	
	QPushButton* ok;
	QPushButton* cancel;
};


#endif
