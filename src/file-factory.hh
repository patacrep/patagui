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
#ifndef __FILE_FACTORY_HH__
#define __FILE_FACTORY_HH__

#include <QObject>
#include <QString>
#include <QStringList>

#include "file-property-manager.hh"

#include "qtpropertybrowser.h"
#include "qteditorfactory.h"


class QLineEdit;
class QtLineEditFactory;



class CFileFactory : public QtAbstractEditorFactory<CFilePropertyManager>
{
  Q_OBJECT
  public:
  CFileFactory(QObject *parent = 0);
  ~CFileFactory();
  
protected:
  void connectPropertyManager(CFilePropertyManager *manager);
  QWidget *createEditor(CFilePropertyManager *manager, QtProperty *property,
			QWidget *parent);
    void disconnectPropertyManager(CFilePropertyManager *manager);
private slots:
  void browse();
  void slotFilenameChanged(QtProperty *property, const QString &filename);
  void slotEditorDestroyed(QObject *object);
private:
  QtLineEditFactory *originalFactory;
  CFilePropertyManager* m_manager;
  QtProperty* m_property;
  QMap<QtProperty *, QList<QLineEdit *> > createdEditors;
  QMap<QLineEdit *, QtProperty *> editorToProperty;
};

#endif // __FILE_FACTORY_HH__
