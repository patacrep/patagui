#include <QMap>
#include <QLineEdit>
#include <QLayout>
#include <QToolButton>
#include <QFileDialog>
#include <QtStringPropertyManager>

#include "qtpropertybrowser.h"
#include "qteditorfactory.h"

#include "file-property-manager.hh"
#include "file-factory.hh"

CFileFactory::CFileFactory(QObject *parent)
  : QtAbstractEditorFactory<CFilePropertyManager>(parent)
{
  originalFactory = new QtLineEditFactory(this);
}

CFileFactory::~CFileFactory()
{
  // no need to delete editors because they will be 
  // deleted by originalFactory in its destructor
}

void CFileFactory::connectPropertyManager(CFilePropertyManager *manager)
{
  originalFactory->addPropertyManager(manager);
  connect(manager, SIGNAL(filenameChanged(QtProperty *, const QString &)), 
	  this, SLOT(slotFilenameChanged(QtProperty *, const QString &)));
}

QWidget *CFileFactory::createEditor(CFilePropertyManager *manager, QtProperty *property,
				    QWidget *parent)
{
  QtAbstractEditorFactoryBase *base = originalFactory;
  QWidget *w = base->createEditor(property, parent);
  if (!w)
    return 0;

  QLineEdit *lineEdit = qobject_cast<QLineEdit *>(w);
  if (!lineEdit)
    return 0;

  QHBoxLayout* layout = new QHBoxLayout;
  layout->addWidget(lineEdit);

  QToolButton* button = new QToolButton;
  button->setText(tr("Browse"));
  m_manager = manager; 
  m_property = property; 
  connect(button, SIGNAL(clicked()), this, SLOT(browse()));

  layout->addWidget(button);
  QWidget* widget = new QWidget;
  widget->setLayout(layout);
  lineEdit->setText(manager->filename(property));
    
  createdEditors[property].append(lineEdit);
  editorToProperty[lineEdit] = property;

  return widget;
}

void CFileFactory::browse()
{

  QString filename = QFileDialog::getOpenFileName(NULL,
						  tr("Open"),
						  QDir::currentPath(),
						  tr("Images (*.jpg)"));
  if(!filename.isEmpty())
    m_manager->setFilename(m_property, filename);
}

void CFileFactory::disconnectPropertyManager(CFilePropertyManager *manager)
{
  originalFactory->removePropertyManager(manager);
  disconnect(manager, SIGNAL(filterChanged(QtProperty *, const QString &)), 
	     this, SLOT(slotFilterChanged(QtProperty *, const QString &)));
}

void CFileFactory::slotFilenameChanged(QtProperty *property, const QString &filename)
{
  if (!createdEditors.contains(property))
    return;

  CFilePropertyManager *manager = propertyManager(property);
  if (!manager)
    return;

  QList<QLineEdit *> editors = createdEditors[property];
  QListIterator<QLineEdit *> itEditor(editors);
  while (itEditor.hasNext()) {
    QLineEdit *editor = itEditor.next();
    editor->setText(filename);
  }
}

void CFileFactory::slotEditorDestroyed(QObject *object)
{
  QMap<QLineEdit *, QtProperty *>::ConstIterator itEditor =
    editorToProperty.constBegin();
  while (itEditor != editorToProperty.constEnd()) {
    if (itEditor.key() == object) {
      QLineEdit *editor = itEditor.key();
      QtProperty *property = itEditor.value();
      editorToProperty.remove(editor);
      createdEditors[property].removeAll(editor);
      if (createdEditors[property].isEmpty())
	createdEditors.remove(property);
      return;
    }
    itEditor++;
  }
}
    
