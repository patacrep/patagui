/****************************************************************************
 **
 ** Copyright (C) 2006 Trolltech ASA. All rights reserved.
 ** Copyright (C) 2012, Romain Goffe <romain.goffe@gmail.com>
 ** Copyright (C) 2012, Alexandre Dupas <alexandre.dupas@gmail.com>
 **
 ** This file is part of the documentation of Qt. It was originally
 ** published as part of Qt Quarterly.
 **
 ** This file may be used under the terms of the GNU General Public License
 ** version 2.0 as published by the Free Software Foundation or under the
 ** terms of the Qt Commercial License Agreement. The respective license
 ** texts for these are provided with the open source and commercial
 ** editions of Qt.
 **
 ** If you are unsure which license is appropriate for your use, please
 ** review the following information:
 ** http://www.trolltech.com/products/qt/licensing.html or contact the
 ** sales department at sales@trolltech.com.
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** Modified to serve personnal purposes.
 **
 ****************************************************************************/

#include "variant-factory.hh"
#include "variant-manager.hh"
#include "file-chooser.hh"
#include <QSpinBox>

VariantFactory::~VariantFactory()
{
  {
    QList<CFileChooser *> editors = theEditorToProperty.keys();
    QListIterator<CFileChooser *> it(editors);
    while (it.hasNext())
      delete it.next();
  }

  {
    QList<QSpinBox *> spinboxes = theSpinBoxToProperty.keys();
    QListIterator<QSpinBox *> it(spinboxes);
    while (it.hasNext())
      delete it.next();
  }
}

void VariantFactory::connectPropertyManager(QtVariantPropertyManager *manager)
{
  connect(manager, SIGNAL(valueChanged(QtProperty *, const QVariant &)),
	  this, SLOT(slotPropertyChanged(QtProperty *, const QVariant &)));
  connect(manager, SIGNAL(attributeChanged(QtProperty *, const QString &, const QVariant &)),
	  this, SLOT(slotPropertyAttributeChanged(QtProperty *, const QString &, const QVariant &)));
  QtVariantEditorFactory::connectPropertyManager(manager);
}

QWidget *VariantFactory::createEditor(QtVariantPropertyManager *manager,
				      QtProperty *property, QWidget *parent)
{
  if (manager->propertyType(property) == VariantManager::filePathTypeId())
    {
      CFileChooser *editor = new CFileChooser(parent);
      editor->setPath(manager->value(property).toString());
      editor->setFilter(manager->attributeValue(property, QLatin1String("filter")).toString());
      theCreatedEditors[property].append(editor);
      theEditorToProperty[editor] = property;

      connect(editor, SIGNAL(pathChanged(const QString &)),
	      this, SLOT(slotSetValue(const QString &)));
      connect(editor, SIGNAL(destroyed(QObject *)),
	      this, SLOT(slotEditorDestroyed(QObject *)));
      return editor;
    }
  else if (manager->propertyType(property) == VariantManager::unitTypeId())
    {
      QSpinBox *spinbox = new QSpinBox(parent);
      spinbox->setValue(manager->value(property).toInt());
      //spinbox->setSuffix(manager->attributeValue(property, QLatin1String("unit")).toString());
      //hardcoded as the solution above does not work and we only have one type of spinbox (for font size)
      spinbox->setSuffix("pt");
      spinbox->setRange(10, 12);
      theCreatedSpinBoxes[property].append(spinbox);
      theSpinBoxToProperty[spinbox] = property;

      connect(spinbox, SIGNAL(valueChanged(int)),
	      this, SLOT(slotSetIntValue(int)));
      connect(spinbox, SIGNAL(destroyed(QObject *)),
	      this, SLOT(slotEditorDestroyed(QObject *)));
      return spinbox;
    }
  return QtVariantEditorFactory::createEditor(manager, property, parent);
}

void VariantFactory::disconnectPropertyManager(QtVariantPropertyManager *manager)
{
  disconnect(manager, SIGNAL(valueChanged(QtProperty *, const QVariant &)),
	     this, SLOT(slotPropertyChanged(QtProperty *, const QVariant &)));
  disconnect(manager, SIGNAL(attributeChanged(QtProperty *, const QString &, const QVariant &)),
	     this, SLOT(slotPropertyAttributeChanged(QtProperty *, const QString &, const QVariant &)));
  QtVariantEditorFactory::disconnectPropertyManager(manager);
}

void VariantFactory::slotPropertyChanged(QtProperty *property,
					 const QVariant &value)
{
  if (theCreatedEditors.contains(property))
    {
      QList<CFileChooser *> editors = theCreatedEditors[property];
      QListIterator<CFileChooser *> itEditor(editors);
      while (itEditor.hasNext())
	itEditor.next()->setPath(value.toString());
    }
  else if (theCreatedSpinBoxes.contains(property))
    {
      QList<QSpinBox *> spinboxes = theCreatedSpinBoxes[property];
      QListIterator<QSpinBox *> it(spinboxes);
      while (it.hasNext())
	it.next()->setValue(value.toInt());
    }
}

void VariantFactory::slotPropertyAttributeChanged(QtProperty *property,
						  const QString &attribute, const QVariant &value)
{
  if (!theCreatedEditors.contains(property) && !theCreatedSpinBoxes.contains(property))
    return;

  if (attribute == QLatin1String("filter"))
    {
      QList<CFileChooser *> editors = theCreatedEditors[property];
      QListIterator<CFileChooser *> itEditor(editors);
      while (itEditor.hasNext())
	itEditor.next()->setFilter(value.toString());
    }
  else if (attribute == QLatin1String("unit"))
    {
      QList<QSpinBox *> spinboxes = theCreatedSpinBoxes[property];
      QListIterator<QSpinBox *> it(spinboxes);
      while (it.hasNext())
	it.next()->setSuffix(value.toString());
    }
}

void VariantFactory::slotSetValue(const QString &value)
{
  QObject *object = sender();
  QMap<CFileChooser *, QtProperty *>::ConstIterator itEditor =
    theEditorToProperty.constBegin();
  while (itEditor != theEditorToProperty.constEnd())
    {
      if (itEditor.key() == object)
	{
	  QtProperty *property = itEditor.value();
	  QtVariantPropertyManager *manager = propertyManager(property);
	  if (!manager)
	    return;
	  manager->setValue(property, value);
	  return;
	}
      itEditor++;
    }
}

void VariantFactory::slotSetIntValue(int value)
{
  QObject *object = sender();
  QMap<QSpinBox *, QtProperty *>::ConstIterator it =
    theSpinBoxToProperty.constBegin();
  while (it != theSpinBoxToProperty.constEnd())
    {
      if (it.key() == object)
	{
	  QtProperty *property = it.value();
	  QtVariantPropertyManager *manager = propertyManager(property);
	  if (!manager)
	    return;
	  manager->setValue(property, value);
	  return;
	}
      it++;
    }
}

void VariantFactory::slotEditorDestroyed(QObject *object)
{
  QMap<CFileChooser *, QtProperty *>::ConstIterator itEditor =
    theEditorToProperty.constBegin();
  while (itEditor != theEditorToProperty.constEnd())
    {
      if (itEditor.key() == object)
	{
	  CFileChooser *editor = itEditor.key();
	  QtProperty *property = itEditor.value();
	  theEditorToProperty.remove(editor);
	  theCreatedEditors[property].removeAll(editor);
	  if (theCreatedEditors[property].isEmpty())
	    theCreatedEditors.remove(property);
	  break;
	}
      itEditor++;
    }

  QMap<QSpinBox *, QtProperty *>::ConstIterator it =
    theSpinBoxToProperty.constBegin();
  while (it != theSpinBoxToProperty.constEnd())
    {
      if (it.key() == object)
	{
	  QSpinBox *editor = it.key();
	  QtProperty *property = it.value();
	  theSpinBoxToProperty.remove(editor);
	  theCreatedSpinBoxes[property].removeAll(editor);
	  if (theCreatedSpinBoxes[property].isEmpty())
	    theCreatedSpinBoxes.remove(property);
	  break;
	}
      it++;
    }
}

