/* MinMaxCurvatureFlow.h
 * 2010-06-02: create this program by Lei Qu
 */

#ifndef __MINMAXCURVATUREFLOW_H__
#define __MINMAXCURVATUREFLOW_H__

#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>

class ITKMinMaxCurvatureFlowPlugin: public QObject, public V3DSingleImageInterface
{
	Q_OBJECT
	Q_INTERFACES(V3DSingleImageInterface)

public:
	ITKMinMaxCurvatureFlowPlugin()
	{
	}
	QStringList menulist() const;
	void processImage(const QString &arg, Image4DSimple *p4DImage, QWidget *parent);

};

class ITKMinMaxCurvatureFlowDialog: public QDialog
{
Q_OBJECT

public:
	ITKMinMaxCurvatureFlowDialog(Image4DSimple *p4DImage, QWidget *parent)
	{
		if (!p4DImage)
			return;

		printf("Passing data to data1d\n");

		ok = new QPushButton("OK");
		cancel = new QPushButton("Cancel");

		gridLayout = new QGridLayout();

		gridLayout->addWidget(cancel, 0, 0);
		gridLayout->addWidget(ok, 0, 1);
		setLayout( gridLayout);
		setWindowTitle(QString("MinMaxCurvatureFlow"));

		connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
	}

	~ITKMinMaxCurvatureFlowDialog()
	{
	}

public slots:

public:
	QGridLayout *gridLayout;

	QPushButton* ok;
	QPushButton* cancel;
};

#endif

