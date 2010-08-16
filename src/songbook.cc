// Copyright (C) 2010 Romain Goffe, Alexandre Dupas
//
// Songbook Creator is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// Songbook Creator is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
// MA  02110-1301, USA.
//******************************************************************************
#include "songbook.hh"

#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QWidget>
#include <QLabel>
#include <QComboBox>

#include <QScriptEngine>
#include <QScriptValue>
#include <QScriptValueIterator>

#include <QDebug>

#include "qtpropertymanager.h"
#include "mainwindow.hh"

CSongbook::CSongbook()
  : QObject()
  , m_filename()
  , m_tmpl()
  , m_songs()
  , m_panel()
  , m_propertyManager(new QtVariantPropertyManager())
  , m_templates()
  , m_parameters()
  , m_groupManager()
  , m_advancedParameters()
{}

CSongbook::~CSongbook()
{
  if (m_panel)
    delete m_panel;

  delete m_propertyManager;
}

QString CSongbook::filename()
{
  return m_filename;
}

void CSongbook::setFilename(const QString &filename)
{
  m_filename = filename;
}

bool CSongbook::isModified()
{
  return m_modified;
}

void CSongbook::setModified(bool modified)
{
  m_modified = modified;
  emit(wasModified(modified));
}

QString CSongbook::tmpl()
{
  return m_tmpl;
}

void CSongbook::setTmpl(const QString &tmpl)
{
  int index =  m_templates.indexOf(tmpl);
  if (m_tmpl != tmpl && -1 != index)
    {
      m_tmpl = tmpl;
      setModified(true);
      m_templateComboBox->setCurrentIndex(index);
      changeTemplate(tmpl);
    }
}

QStringList CSongbook::songs()
{
  return m_songs;
}

void CSongbook::setSongs(QStringList songs)
{
  if (m_songs != songs)
    {
      setModified(true);
      m_songs = songs;
      emit(songsChanged());
    }
}

QWidget * CSongbook::panel()
{
  if (!m_panel)
    {
      m_panel = new QWidget;
      m_panel->setMinimumWidth(300);

      m_propertyEditor = new QtButtonPropertyBrowser();
      m_propertyEditor->setFactoryForManager(m_propertyManager,
                                             new QtVariantEditorFactory());

      QBoxLayout *templateLayout = new QHBoxLayout;
      m_templateComboBox = new QComboBox(m_panel);
      m_templateComboBox->addItems(m_templates);
      m_templateComboBox->setCurrentIndex(m_templates.indexOf("patacrep.tmpl"));
      connect(m_templateComboBox, SIGNAL(currentIndexChanged(const QString &)),
              this, SLOT(setTmpl(const QString &)));
      templateLayout->addWidget(new QLabel(tr("Template:")));
      templateLayout->addWidget(m_templateComboBox);
      templateLayout->setStretch(1,1);

      changeTemplate();

      QBoxLayout *mainLayout = new QVBoxLayout;
      mainLayout->addLayout(templateLayout);
      mainLayout->addWidget(m_propertyEditor,1);

      m_panel->setLayout(mainLayout);
    }
  return m_panel;
}

void CSongbook::reset()
{
  setFilename(QString());

  QMap< QString, QtVariantProperty* >::const_iterator it;
  for (it = m_parameters.constBegin(); it != m_parameters.constEnd(); ++it)
    {
      it.value()->setValue(QVariant(""));
    }
  setModified(false);
}

void CSongbook::changeTemplate(const QString & filename)
{
  QString templateFilename("patacrep.tmpl");
  if (!filename.isEmpty())
    templateFilename = filename;

  QString json;

  // reserved template parameters
  QStringList reservedParameters;
  reservedParameters << "name" << "template" << "songs" << "songslist";

  // read template file
  QFile file(QString("%1/templates/%2").arg(workingPath()).arg(templateFilename));
  if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      QTextStream in(&file);
      QRegExp jsonFilter("^%%:");
      QString line;
      json = "(";
      do {
        line = in.readLine();
        if (line.startsWith("%%:"))
	  json += line.remove(jsonFilter) + "\n";

      } while (!line.isNull());
      json += ")";
      file.close();
    }
  else
    {
      qWarning() << "unable to open file in read mode";
    }

  // Load json encoded songbook data
  QScriptEngine engine;

  // check syntax
  QScriptSyntaxCheckResult res = QScriptEngine::checkSyntax(json);
  if (res.state() != QScriptSyntaxCheckResult::Valid)
    {
      qDebug() << "Error line "<< res.errorLineNumber()
               << " column " << res.errorColumnNumber()
               << ":" << res.errorMessage();
      return;
    }

  // evaluate the json data
  QScriptValue parameters = engine.evaluate(json);

  // load parameters data
  if (parameters.isValid() && parameters.isArray())
    {
      QScriptValue svName;
      QScriptValue svDescription;
      QScriptValue svType;
      QScriptValue svDefault;
      QScriptValue svValues;
      QScriptValue svMandatory;

      int propertyType;

      QMap< QString, QVariant > oldValues;
      {
        QMap< QString, QtVariantProperty* >::const_iterator it = m_parameters.constBegin();
        while (it != m_parameters.constEnd())
          {
            oldValues.insert(it.key(),it.value()->value());
            it++;
          }
        m_parameters.clear();
        m_propertyManager->clear();
      }

      QtVariantProperty *item;
      QScriptValueIterator it(parameters);
      bool advancedParameters = false;

      if(m_groupManager)
	delete m_groupManager;

      m_groupManager = new QtGroupPropertyManager(this);
      m_advancedParameters = m_groupManager->addProperty(tr("Advanced Parameters"));

      while (it.hasNext())
        {
          it.next();
          svName = it.value().property("name");
          if (!reservedParameters.contains(svName.toString()))
            {
              QVariant oldValue;
              QStringList stringValues;

              svDescription = it.value().property("description");
              svDefault = it.value().property("default");
              svType = it.value().property("type");
              svValues = it.value().property("values");
              svMandatory = it.value().property("mandatory");

              // determine property type
              if (svType.toString() == QString("string"))
                propertyType = QVariant::String;
              else if (svType.toString() == QString("color"))
                propertyType = QVariant::Color;
              else if (svType.toString() == QString("enum"))
                propertyType = QtVariantPropertyManager::enumTypeId();
              else if (svType.toString() == QString("flag"))
                propertyType = QtVariantPropertyManager::flagTypeId();
              else
                propertyType = QVariant::String;

              // add new property
              item = m_propertyManager
                ->addProperty(propertyType, svDescription.toString());

              // retrieve existing or default value
              if (oldValues.contains(svName.toString()))
                {
                  oldValue = oldValues.value(svName.toString());
                }
              else if (svDefault.isValid())
                {
                  oldValue = svDefault.toVariant();
                }

              if (propertyType == QtVariantPropertyManager::enumTypeId())
                {
                  qScriptValueToSequence(svValues, stringValues);
                  m_propertyManager->setAttribute(item, "enumNames",
                                                  QVariant(stringValues));
                  // handle existing or default value in case of enum
                  if (oldValue.isValid() && oldValue.type() == QVariant::String)
                    {
                      oldValue = QVariant(stringValues.indexOf(oldValue.toString()));
                    }
                }
              else if (propertyType == QtVariantPropertyManager::flagTypeId())
                {
                  qScriptValueToSequence(svValues, stringValues);
                  m_propertyManager->setAttribute(item, "flagNames",
                                                  QVariant(stringValues));
                  // handle existing or default value in case of flag
                  if (oldValue.isValid() && oldValue.type() == QVariant::List)
                    {
                      QStringList activatedFlags;
                      qScriptValueToSequence(svDefault, activatedFlags);
                      int flags = 0;
                      int index = 1;
                      for (int i = 0; i < stringValues.size(); ++i)
                        {
                          if (activatedFlags.contains(stringValues.at(i)))
                            {
                              flags |= index;
                            }
                          index *= 2;
                        }
                      oldValue = QVariant(flags);
                    }
                }

              // set the existing or default value
              if (oldValue.isValid())
                {
                  item->setValue(oldValue);
                }

              // insert the property into the list of parameters
              m_parameters.insert(svName.toString(), item);

              // handle the mandatory boolean parameter
	      if (svMandatory.isValid() && svMandatory.toBool())
                {
		  m_propertyEditor->addProperty(item);
		}
	      else
		{
		  advancedParameters = true;
		  m_advancedParameters->addSubProperty(item);
		}
            }
        }
      if (advancedParameters)
	{
	  m_propertyEditor->addProperty(m_advancedParameters);
	}
    }
}

void CSongbook::save(const QString & filename)
{
  QFile file(filename);
  if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
      QTextStream out(&file);
      out << "{\n";

      if (!tmpl().isEmpty())
        out << "\"template\" : \"" << tmpl() << "\",\n";

      QMap< QString, QtVariantProperty* >::const_iterator it = m_parameters.constBegin();
      QtProperty *property;
      int type;
      QVariant value;
      QString string_value;
      QColor color_value;
      QVariant stringValues;
      while (it != m_parameters.constEnd())
        {
          property = it.value();
          type = m_propertyManager->propertyType(property);
          value = m_propertyManager->value(property);
          if (type == QVariant::String)
            {
              string_value = value.toString();
              if (!string_value.isEmpty())
                {
                  out << "\"" << it.key() << "\" : \""
                      << string_value.replace('\\',"\\\\") << "\",\n";
                }
            }
          else if (type == QVariant::Color)
            {
              color_value = value.value< QColor >();
              string_value = color_value.name();
              string_value.remove(0,1);
              if (!string_value.isEmpty())
                {
                  out << "\"" << it.key() << "\" : \"#"
                      << string_value.toUpper() << "\",\n";
                }
            }
          else if (type == QtVariantPropertyManager::enumTypeId())
            {
              stringValues = m_propertyManager->attributeValue(property,
                                                               "enumNames");
              string_value = stringValues.toStringList()[value.toInt()];
              if (!string_value.isEmpty())
                {
                  out << "\"" << it.key() << "\" : \""
                      << string_value.replace('\\',"\\\\") << "\",\n";
                }
            }
          else if (type == QtVariantPropertyManager::flagTypeId())
            {
              stringValues = m_propertyManager->attributeValue(property,
                                                               "flagNames");
              QStringList flagValues = stringValues.toStringList();
              QStringList activatedFlags;
              int index = 1;
              int flags = value.toInt();
              for (int i = 0; i < flagValues.size(); ++i)
                {
                  if (flags & index)
                    {
                      activatedFlags << flagValues.at(i);
                    }
                  index *= 2;
                }

              if (!activatedFlags.isEmpty())
                {
                  out << "\"" << it.key() << "\" : [\n    \""
                      << (activatedFlags.join("\",\n    \""))
                      << "\"\n  ],\n";
                }
            }
          ++it;
        }

      out << "\"songs\" : [\n    \"" << (songs().join("\",\n    \"")) << "\"\n  ]\n}\n";
      file.close();
      setModified(false);
      setFilename(filename);
    }
}

void CSongbook::load(const QString & filename)
{
  QFile file(filename);
  if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      QTextStream in(&file);
      QString json = QString("(%1)").arg(in.readAll());
      file.close();

      // Load json encoded songbook data
      QScriptValue object;
      QScriptEngine engine;

      // check syntax
      QScriptSyntaxCheckResult res = QScriptEngine::checkSyntax(json);
      if (res.state() != QScriptSyntaxCheckResult::Valid)
        {
          qDebug() << "Error line "<< res.errorLineNumber()
                   << " column " << res.errorColumnNumber()
                   << ":" << res.errorMessage();
        }
      // evaluate the json data
      object = engine.evaluate(json);

      // load data into this object
      if (object.isObject())
        {
          QScriptValue sv;
          // template property
          sv = object.property("template");
          if (sv.isValid())
            {
              setTmpl(sv.toString());
            }

          // template specific properties
          QtProperty *property;
          int type;
          QVariant value;
          QMap< QString, QtVariantProperty* >::const_iterator it;
          for (it = m_parameters.constBegin(); it != m_parameters.constEnd(); ++it)
            {
              sv = object.property(it.key());
              if (sv.isValid())
                {
                  property = it.value();
                  type = m_propertyManager->propertyType(property);
                  value = sv.toVariant();
                  QVariant stringValues;
                  if (type == QtVariantPropertyManager::enumTypeId())
                    {
                      stringValues = m_propertyManager->attributeValue(property, "enumNames");
                      value = QVariant(stringValues.toStringList().indexOf(value.toString()));
                    }
                  else if (type == QtVariantPropertyManager::flagTypeId())
                    {
                      stringValues = m_propertyManager->attributeValue(property, "flagNames");
                      QStringList flagValues = stringValues.toStringList();
                      QStringList activatedFlags = value.toStringList();
                      int flags = 0;
                      int index = 1;
                      for (int i = 0; i < flagValues.size(); ++i)
                        {
                          if (activatedFlags.contains(flagValues.at(i)))
                            {
                              flags |= index;
                            }
                          index *= 2;
                        }
                      value = QVariant(flags);
                    }
                  m_propertyManager->setValue(property, value);
                }
            }

          // songs property (if not an array, the value can be "all")
          sv = object.property("songs");
          if (sv.isValid())
            {
              QStringList items;
              if (!sv.isArray())
                {
                  qDebug() << "not implemented yet";
                }
              else
                {
                  qScriptValueToSequence(sv, items);
                }
              setSongs(items);
            }
        }
      setModified(false);
      setFilename(filename);
    }
  else
    {
      qWarning() << "unable to open file in read mode";
    }
}

QString CSongbook::workingPath() const
{
  return m_workingPath;
}

void CSongbook::setWorkingPath(QString path)
{
  if (m_workingPath != path)
    {
      m_workingPath = path;
      QDir templatesDirectory(QString("%1/templates").arg(workingPath()));
      m_templates = templatesDirectory.entryList(QStringList() << "*.tmpl");
      if (m_panel)
	{
	  m_templateComboBox->clear();
	  m_templateComboBox->addItems(m_templates);
	  m_templateComboBox->setCurrentIndex(m_templates.indexOf("patacrep.tmpl"));    }
    }
}
