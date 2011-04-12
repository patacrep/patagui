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
#include <QMap>
#include <QLineEdit>
#include <QtStringPropertyManager>
#include "qtpropertybrowser.h"
#include "qteditorfactory.h"

#include "file-property-manager.hh"

CFilePropertyManager::CFilePropertyManager(QObject *parent)
  : QtStringPropertyManager(parent)
{
}

CFilePropertyManager::~CFilePropertyManager()
{
}

QString CFilePropertyManager::filename(const QtProperty *property) const
{
  if (!propertyToData.contains(property))
    return QString();
  return propertyToData[property].filename;
}

void CFilePropertyManager::setFilename(QtProperty *property, const QString &filename)
{
  if (!propertyToData.contains(property))
    return;

  CFilePropertyManager::Data data = propertyToData[property];
  if (data.filename == filename)
    return;

  data.filename = filename;
  propertyToData[property] = data;
    
  emit propertyChanged(property);
  emit filenameChanged(property, filename);
}

QString CFilePropertyManager::value(const QtProperty *property) const
{
  QString text = QtStringPropertyManager::value(property);
  if (!propertyToData.contains(property))
    return text;

  CFilePropertyManager::Data data = propertyToData[property];
  text = data.filename;

  return text;
}

void CFilePropertyManager::initializeProperty(QtProperty *property)
{
  propertyToData[property] = CFilePropertyManager::Data();
  QtStringPropertyManager::initializeProperty(property);
}

void CFilePropertyManager::uninitializeProperty(QtProperty *property)
{
  propertyToData.remove(property);
  QtStringPropertyManager::uninitializeProperty(property);
}
