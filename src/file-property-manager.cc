#include <QtCore/QMap>
#include <QtGui/QLineEdit>
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
