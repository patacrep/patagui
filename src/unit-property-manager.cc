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
#include <QtCore/QMap>
#include <QtGui/QSpinBox>
#include <QtIntPropertyManager>
#include "qtpropertybrowser.h"
#include "qteditorfactory.h"

#include "unit-property-manager.hh"

CUnitPropertyManager::CUnitPropertyManager(QObject *parent)
  : QtIntPropertyManager(parent)
{
}

CUnitPropertyManager::~CUnitPropertyManager()
{
}

QString CUnitPropertyManager::suffix(const QtProperty *property) const
{
  if (!propertyToData.contains(property))
    return QString();
  return propertyToData[property].suffix;
}

void CUnitPropertyManager::setSuffix(QtProperty *property, const QString &suffix)
{
  if (!propertyToData.contains(property))
    return;

  CUnitPropertyManager::Data data = propertyToData[property];
  if (data.suffix == suffix)
    return;

  data.suffix = suffix;
  propertyToData[property] = data;

  emit propertyChanged(property);
  emit suffixChanged(property, suffix);
}

QString CUnitPropertyManager::valueText(const QtProperty *property) const
{
  return QtIntPropertyManager::valueText(property);
}

void CUnitPropertyManager::initializeProperty(QtProperty *property)
{
  propertyToData[property] = CUnitPropertyManager::Data();
  QtIntPropertyManager::initializeProperty(property);
}

void CUnitPropertyManager::uninitializeProperty(QtProperty *property)
{
  propertyToData.remove(property);
  QtIntPropertyManager::uninitializeProperty(property);
}
