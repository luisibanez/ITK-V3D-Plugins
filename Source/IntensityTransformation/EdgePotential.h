#ifndef __EdgePotential_H__
#define __EdgePotential_H__

#include "V3DITKPluginDefaultHeader.h"

class EdgePotentialPlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(EdgePotential);
};

#endif
