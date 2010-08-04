#ifndef __V3DITKImageSelectionDialog_H__
#define __V3DITKImageSelectionDialog_H__

#include <QtGui>
#include <QtGui/QLabel>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QDialog>
#include "v3d_interface.h"


class V3DITKFileDialogElement
{
public:
  V3DITKFileDialogElement() {};

  V3DITKFileDialogElement( const char * label, V3DPluginCallback * callback )
    {
    QString objectName( label );
    QString labelObjectName = objectName+"Label";

    this->nameLabel = new QLabel();
    this->nameLabel->setObjectName( labelObjectName );
    this->nameLabel->setText(QApplication::translate("MainWindow", label, 0, QApplication::UnicodeUTF8));
    }

  ~V3DITKFileDialogElement()
    {
    delete this->nameLabel;
    }

public:
  QLabel * nameLabel;
};



class V3DITKImageSelectionDialog : public QDialog
{
  Q_OBJECT

public:
  V3DITKImageSelectionDialog();
  ~V3DITKImageSelectionDialog();

  explicit V3DITKImageSelectionDialog( QWidget* iParent = 0);

  V3DITKImageSelectionDialog( const char * windowTitle, V3DPluginCallback * pluginCallback );

  int exec();

public slots:


public:

  std::string   dialogTitle;

  QGridLayout * gridLayout;
  QPushButton * ok;
  QPushButton * cancel;

  typedef std::map< std::string, V3DITKFileDialogElement * >  ElementContainerType;

  ElementContainerType elementContainer;

private:

  void removeChildrenFromGridLayout();

  V3DPluginCallback * callback;

  Q_DISABLE_COPY(V3DITKImageSelectionDialog);

};

#endif
