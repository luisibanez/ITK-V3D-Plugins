#ifndef __OtsuThreshold_H__
#define __OtsuThreshold_H__


#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>

class OtsuThresholdPlugin : public QObject, public V3DPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface)

public:
	OtsuThresholdPlugin() {}
  QStringList menulist() const;
	QStringList funclist() const;

	void domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent);

	virtual void dofunc(const QString & func_name,
			const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent);

};

#endif
