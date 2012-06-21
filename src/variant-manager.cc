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

#include "variant-manager.hh"

class FilePathPropertyType
{};
Q_DECLARE_METATYPE(FilePathPropertyType)

class UnitPropertyType
{};
Q_DECLARE_METATYPE(UnitPropertyType)

int VariantManager::filePathTypeId()
{
  return qMetaTypeId<FilePathPropertyType>();
}

int VariantManager::unitTypeId()
{
  return qMetaTypeId<UnitPropertyType>();
}

bool VariantManager::isPropertyTypeSupported(int propertyType) const
{
  if (propertyType == filePathTypeId() || propertyType == unitTypeId())
    return true;
  return QtVariantPropertyManager::isPropertyTypeSupported(propertyType);
}

int VariantManager::valueType(int propertyType) const
{
  if (propertyType == filePathTypeId() || propertyType == unitTypeId())
    return QVariant::String;
  return QtVariantPropertyManager::valueType(propertyType);
}

QVariant VariantManager::value(const QtProperty *property) const
{
  if (theValues.contains(property))
    return theValues[property].value;
  return QtVariantPropertyManager::value(property);
}

QStringList VariantManager::attributes(int propertyType) const
{
  if (propertyType == filePathTypeId())
    {
      QStringList attr;
      attr << QLatin1String("filter");
      return attr;
    }
  else if (propertyType == unitTypeId())
    {
      QStringList attr;
      attr << QLatin1String("unit");
      return attr;
    }
  return QtVariantPropertyManager::attributes(propertyType);
}

int VariantManager::attributeType(int propertyType, const QString &attribute) const
{
  if (propertyType == filePathTypeId()) {
    if (attribute == QLatin1String("filter"))
      return QVariant::String;
    return 0;
  }
  else if (propertyType == unitTypeId())
    {
      if (attribute == QLatin1String("unit"))
	return QVariant::String;
      return 0;
    }
  return QtVariantPropertyManager::attributeType(propertyType, attribute);
}

QVariant VariantManager::attributeValue(const QtProperty *property, const QString &attribute)
{
  if (theValues.contains(property))
    {
      if (attribute == QLatin1String("filter"))
	return theValues[property].filter;
      else if (attribute == QLatin1String("unit"))
	return theValues[property].filter;
      return QVariant();
    }
  return QtVariantPropertyManager::attributeValue(property, attribute);
}

QString VariantManager::valueText(const QtProperty *property) const
{
  if (theValues.contains(property))
    return theValues[property].value;
  return QtVariantPropertyManager::valueText(property);
}

void VariantManager::setValue(QtProperty *property, const QVariant &val)
{
  if (theValues.contains(property))
    {
      if (val.type() != QVariant::String && !val.canConvert(QVariant::String))
	return;
      QString str = qVariantValue<QString>(val);
      Data d = theValues[property];
      if (d.value == str)
	return;
      d.value = str;
      theValues[property] = d;
      emit propertyChanged(property);
      emit valueChanged(property, str);
      return;
    }
  QtVariantPropertyManager::setValue(property, val);
}

void VariantManager::setAttribute(QtProperty *property,
				  const QString &attribute, const QVariant &val)
{
  if (theValues.contains(property))
    {
      if (attribute == QLatin1String("filter") || attribute == QLatin1String("unit"))
	{
	  if (val.type() != QVariant::String && !val.canConvert(QVariant::String))
	    return;
	  QString str = qVariantValue<QString>(val);
	  Data d = theValues[property];
	  if (d.filter == str)
	    return;

	  d.filter = str;
	  theValues[property] = d;
	  emit attributeChanged(property, attribute, str);
	}
      return;
    }
  QtVariantPropertyManager::setAttribute(property, attribute, val);
}

void VariantManager::initializeProperty(QtProperty *property)
{
  if (propertyType(property) == filePathTypeId() || propertyType(property) == unitTypeId())
    theValues[property] = Data();
  QtVariantPropertyManager::initializeProperty(property);
}

void VariantManager::uninitializeProperty(QtProperty *property)
{
  theValues.remove(property);
  QtVariantPropertyManager::uninitializeProperty(property);
}

