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
