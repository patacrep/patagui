// Copyright (C) 2009-2011, Romain Goffe <romain.goffe@gmail.com>
// Copyright (C) 2009-2011, Alexandre Dupas <alexandre.dupas@gmail.com>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301, USA.
#ifndef __UNIT_FACTORY_HH__
#define __UNIT_FACTORY_HH__

#include <QObject>
#include <QString>
#include <QStringList>

#include "unit-property-manager.hh"

#include "qtpropertybrowser.h"
#include "qteditorfactory.h"


class QSpinBox;
class QtSpinBoxFactory;



class CUnitFactory : public QtAbstractEditorFactory<CUnitPropertyManager>
{
  Q_OBJECT
  public:
  CUnitFactory(QObject *parent = 0);
  ~CUnitFactory();
  
protected:
  void connectPropertyManager(CUnitPropertyManager *manager);
  QWidget *createEditor(CUnitPropertyManager *manager, QtProperty *property,
			QWidget *parent);
  void disconnectPropertyManager(CUnitPropertyManager *manager);
private slots:
  
  void slotSuffixChanged(QtProperty *property, const QString &suffix);
  void slotEditorDestroyed(QObject *object);
private:
  QtSpinBoxFactory *originalFactory;
  QMap<QtProperty *, QList<QSpinBox *> > createdEditors;
  QMap<QSpinBox *, QtProperty *> editorToProperty;
};

#endif // __UNIT_FACTORY_HH__
