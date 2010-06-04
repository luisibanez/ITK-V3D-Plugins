#ifndef __InvertIntensity_H__
#define __InvertIntensity_H__


#include <V3DITKPluginBaseSingleImage.h>

class InvertIntensityPlugin : public V3DITKPluginBaseSingleImage
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface)

public:
	InvertIntensityPlugin() {}
  QStringList menulist() const;
	QStringList funclist() const;

	void domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent);

	virtual void dofunc(const QString & func_name,
			const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent);

};

#endif
