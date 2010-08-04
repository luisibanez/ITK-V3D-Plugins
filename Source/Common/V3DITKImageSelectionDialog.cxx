#include "V3DITKImageSelectionDialog.h"

V3DITKImageSelectionDialog::V3DITKImageSelectionDialog()
{
}

V3DITKImageSelectionDialog::V3DITKImageSelectionDialog( QWidget * )
{
}


V3DITKImageSelectionDialog::~V3DITKImageSelectionDialog()
{
  delete this->ok;
  delete this->cancel;
  delete this->gridLayout;
}

V3DITKImageSelectionDialog::V3DITKImageSelectionDialog( const char * name, V3DPluginCallback * pluginCallback )
{
  this->dialogTitle = name;

  this->ok     = new QPushButton("OK");
  this->cancel = new QPushButton("Cancel");

  this->gridLayout = new QGridLayout();

  this->callback = pluginCallback;
}

void V3DITKImageSelectionDialog::removeChildrenFromGridLayout()
{
  QLayoutItem *child;
  while ( (child = this->gridLayout->takeAt(0)) != 0)
   {
   this->gridLayout->removeItem(child);
   delete child->widget();
   delete child;
   }
}

int V3DITKImageSelectionDialog::exec()
{
  this->removeChildrenFromGridLayout();

  this->setLayout(gridLayout);

  this->setWindowTitle( QObject::tr( this->dialogTitle.c_str() ) );

  this->connect( this->ok,     SIGNAL(clicked()), this, SLOT(accept()) );
  this->connect( this->cancel, SIGNAL(clicked()), this, SLOT(reject()) );

  return this->QDialog::exec();
}

