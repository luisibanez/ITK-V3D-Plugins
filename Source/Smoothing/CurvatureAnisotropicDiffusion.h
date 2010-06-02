/* CurvatureAnisotropicDiffusion.h
 * 2010-06-02: create this program by Lei Qu
 */

#ifndef __CURVATUREANISOTROPICDIFFUSION_H__
#define __CURVATUREANISOTROPICDIFFUSION_H__

#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>

class ITKCurvatureAnisotropicDiffusionPlugin: public QObject, public V3DSingleImageInterface
{
	Q_OBJECT
	Q_INTERFACES(V3DSingleImageInterface)

public:
	ITKCurvatureAnisotropicDiffusionPlugin()
	{
	}
	QStringList menulist() const;
	void processImage(const QString &arg, Image4DSimple *p4DImage, QWidget *parent);

};

class ITKCurvatureAnisotropicDiffusionDialog: public QDialog
{
Q_OBJECT

public:
	ITKCurvatureAnisotropicDiffusionDialog(Image4DSimple *p4DImage, QWidget *parent)
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
		setWindowTitle(QString("Gradient Anisotropic Diffusion"));

		connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
	}

	~ITKCurvatureAnisotropicDiffusionDialog()
	{
	}

public slots:

public:
	QGridLayout *gridLayout;

	QPushButton* ok;
	QPushButton* cancel;
};

#endif

