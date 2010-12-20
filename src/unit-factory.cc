#include <QMap>
#include <QSpinBox>
#include <QtIntPropertyManager>

#include "qtpropertybrowser.h"
#include "qteditorfactory.h"

#include "unit-property-manager.hh"
#include "unit-factory.hh"

CUnitFactory::CUnitFactory(QObject *parent)
  : QtAbstractEditorFactory<CUnitPropertyManager>(parent)
{
  originalFactory = new QtSpinBoxFactory(this);
}

CUnitFactory::~CUnitFactory()
{
  // no need to delete editors because they will be 
  // deleted by originalFactory in its destructor
}

void CUnitFactory::connectPropertyManager(CUnitPropertyManager *manager)
{
  originalFactory->addPropertyManager(manager);
  connect(manager, SIGNAL(suffixChanged(QtProperty *, const QString &)), 
	  this, SLOT(slotSuffixChanged(QtProperty *, const QString &)));
}

QWidget *CUnitFactory::createEditor(CUnitPropertyManager *manager, QtProperty *property,
				    QWidget *parent)
{
  QtAbstractEditorFactoryBase *base = originalFactory;
  QWidget *w = base->createEditor(property, parent);
  if (!w)
    return 0;

  QSpinBox *spinBox = qobject_cast<QSpinBox *>(w);
  if (!spinBox)
    return 0;

  spinBox->setSuffix(manager->suffix(property));

  createdEditors[property].append(spinBox);
  editorToProperty[spinBox] = property;

  return spinBox;
}

void CUnitFactory::disconnectPropertyManager(CUnitPropertyManager *manager)
{
  originalFactory->removePropertyManager(manager);
  disconnect(manager, SIGNAL(suffixChanged(QtProperty *, const QString &)), 
	     this, SLOT(slotSuffixChanged(QtProperty *, const QString &)));
}

void CUnitFactory::slotSuffixChanged(QtProperty *property, const QString &suffix)
{
  if (!createdEditors.contains(property))
    return;

  CUnitPropertyManager *manager = propertyManager(property);
  if (!manager)
    return;

  QList<QSpinBox *> editors = createdEditors[property];
  QListIterator<QSpinBox *> itEditor(editors);
  while (itEditor.hasNext()) {
    QSpinBox *editor = itEditor.next();
    editor->setSuffix(suffix);
  }
}

void CUnitFactory::slotEditorDestroyed(QObject *object)
{
  QMap<QSpinBox *, QtProperty *>::ConstIterator itEditor =
    editorToProperty.constBegin();
  while (itEditor != editorToProperty.constEnd()) {
    if (itEditor.key() == object) {
      QSpinBox *editor = itEditor.key();
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
