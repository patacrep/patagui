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
#ifndef __FILE_PROPERTY_MANAGER_HH__
#define __FILE_PROPERTY_MANAGER_HH__

#include <QObject>
#include <QString>
#include <QStringList>
#include <QtIntPropertyManager>
#include <QtVariantProperty>
#include <QtGroupBoxPropertyBrowser>

class CFilePropertyManager : public QtStringPropertyManager
{
  Q_OBJECT
  public:
  CFilePropertyManager(QObject *parent = 0);
  ~CFilePropertyManager();

  QString filename(const QtProperty *property) const;
  static int id(){return 126;};
  virtual QString value(const QtProperty *property) const;

public slots:
  void setFilename(QtProperty *property, const QString &filter);

signals:
  void filenameChanged(QtProperty *property, const QString &filter);

protected:
  virtual void initializeProperty(QtProperty *property);
  virtual void uninitializeProperty(QtProperty *property);

private:
  struct Data {
    QString filename;
  };
  QMap<const QtProperty *, Data> propertyToData;
};

#endif // __FILE_PROPERTY_MANAGER_HH__
