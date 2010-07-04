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

#include <QFile>
#include <QTextStream>
#include <QSettings>

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QDir>
#include <QRadioButton>
#include <QCheckBox>
#include <QGroupBox>
#include <QButtonGroup>

#include <QScriptEngine>
#include <QScriptValue>
#include <QScriptValueIterator>

#include <QDebug>

CSongbook::CSongbook()
  : QObject()
  , m_filename()
  , m_tmpl()
  , m_bookType()
  , m_songs()
  , m_panel()
  , m_templates()
  , m_parameters()
{
  QSettings settings;
  QString workingPath = settings.value("workingPath", QString("%1/").arg(QDir::currentPath())).toString();

  QDir templatesDirectory(QString("%1/templates").arg(workingPath));
  m_templates = templatesDirectory.entryList(QStringList() << "*.tmpl");
}

CSongbook::~CSongbook()
{
  if (m_panel)
    delete m_panel;
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
  int index;
  if (m_tmpl != tmpl && (-1 != (index = m_templates.indexOf(tmpl))))
    {
      setModified(true);
      m_templateComboBox->setCurrentIndex(index);
      changeTemplate(tmpl);
      m_tmpl = tmpl;
    }
}

QStringList CSongbook::bookType()
{
  return m_bookType;
}

void CSongbook::setBookType(QStringList bookType)
{
  if (m_bookType != bookType)
    {
      setModified(true);
      m_bookType = bookType;
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

      m_propertyManager = new QtVariantPropertyManager();
      m_propertyEditor = new QtButtonPropertyBrowser();
      m_propertyEditor->setFactoryForManager(m_propertyManager,
                                             new QtVariantEditorFactory());
      m_propertyEditor->setMinimumSize(200,250);


      QBoxLayout *templateLayout = new QHBoxLayout;
      m_templateComboBox = new QComboBox(m_panel);
      m_templateComboBox->addItems(m_templates);
      connect(m_templateComboBox, SIGNAL(currentIndexChanged(const QString &)),
              this, SLOT(setTmpl(const QString &)));
      templateLayout->addWidget(new QLabel(tr("Template:")));
      templateLayout->addWidget(m_templateComboBox);
      templateLayout->setStretch(1,1);

      changeTemplate();

      QBoxLayout *mainLayout = new QVBoxLayout;
      mainLayout->addLayout(templateLayout);
      mainLayout->addWidget(m_propertyEditor,1);

      // BookType
      m_chordbookRadioButton = new QRadioButton(tr("Chordbook"));
      m_lyricbookRadioButton = new QRadioButton(tr("Lyricbook"));
      m_diagramCheckBox = new QCheckBox(tr("Chord Diagram"));
      m_lilypondCheckBox = new QCheckBox(tr("Lilypond"));
      m_tablatureCheckBox = new QCheckBox(tr("Tablature"));

      // diagram option cannot (yet) be disabled
      m_diagramCheckBox->setChecked(true);
      m_diagramCheckBox->setEnabled(false);

      QButtonGroup *bookTypeGroup = new QButtonGroup();
      bookTypeGroup->addButton(m_chordbookRadioButton);
      bookTypeGroup->addButton(m_lyricbookRadioButton);

      m_chordbookRadioButton->setChecked(true);

      // connect modification signal
      connect(m_chordbookRadioButton, SIGNAL(toggled(bool)),
              this, SLOT(updateBooktype(bool)));
      connect(m_lyricbookRadioButton, SIGNAL(toggled(bool)),
              this, SLOT(updateBooktype(bool)));
      connect(m_diagramCheckBox, SIGNAL(toggled(bool)),
              this, SLOT(updateBooktype(bool)));
      connect(m_lilypondCheckBox, SIGNAL(toggled(bool)),
              this, SLOT(updateBooktype(bool)));
      connect(m_tablatureCheckBox, SIGNAL(toggled(bool)),
              this, SLOT(updateBooktype(bool)));

      QGroupBox* bookTypeGroupBox = new QGroupBox(tr("Book type"));
      QVBoxLayout* bookTypeLayout = new QVBoxLayout;
      bookTypeLayout->addWidget(m_chordbookRadioButton);
      bookTypeLayout->addWidget(m_lyricbookRadioButton);
      bookTypeGroupBox->setLayout(bookTypeLayout);

      QGroupBox* optionsGroupBox = new QGroupBox(tr("Book options"));
      QVBoxLayout* optionsLayout = new QVBoxLayout;
      optionsLayout->addWidget(m_diagramCheckBox);
      optionsLayout->addWidget(m_lilypondCheckBox);
      optionsLayout->addWidget(m_tablatureCheckBox);
      optionsGroupBox->setLayout(optionsLayout);

      QHBoxLayout *layout = new QHBoxLayout;
      layout->addWidget(bookTypeGroupBox);
      layout->addWidget(optionsGroupBox);

      mainLayout->addLayout(layout);

      m_panel->setLayout(mainLayout);
    }
  return m_panel;
}

void CSongbook::updateBooktype(bool)
{
  if (m_lyricbookRadioButton->isChecked())
    {
      m_diagramCheckBox->setEnabled(false);
      m_lilypondCheckBox->setEnabled(false);
      m_tablatureCheckBox->setEnabled(false);
      m_bookType = QStringList() << "lyric";
    }
  else
    {
      m_bookType = QStringList() << "chorded";
      // diagram option cannot (yet) be disabled
      //m_diagramCheckBox->setEnabled(true);
      m_lilypondCheckBox->setEnabled(true);
      m_tablatureCheckBox->setEnabled(true);
      if (m_diagramCheckBox->isChecked())
        ; // m_bookType << "diagram"; // currently unsupported
      if (m_lilypondCheckBox->isChecked())
        m_bookType << "lilypond";
      if (m_tablatureCheckBox->isChecked())
        m_bookType << "tabs";
    }
}

void CSongbook::reset()
{
  setBookType(QStringList()<<"chorded");

  QMap< QString, QtVariantProperty* >::const_iterator it;
  for (it = m_parameters.constBegin(); it != m_parameters.constEnd(); ++it)
    {
      it.value()->setValue(QVariant(""));
    }
  setModified(false);
  update();
}

void CSongbook::update()
{
  if (m_bookType.contains("lyric"))
    {
      m_lyricbookRadioButton->setChecked(true);
    }
  else if (m_bookType.contains("chorded"))
    {
      m_chordbookRadioButton->setChecked(true);
      // diagram option cannot (yet) be disabled
      //m_diagramCheckBox->setChecked(m_bookType.contains("diagram"));
      m_lilypondCheckBox->setChecked(m_bookType.contains("lilypond"));
      m_tablatureCheckBox->setChecked(m_bookType.contains("tabs"));
    }
}

void CSongbook::changeTemplate(const QString & filename)
{
  QString templateFilename("songbook.tmpl");
  if (!filename.isEmpty())
    templateFilename = filename;

  QString json;

  // reserved template parameters
  QStringList reservedParameters;
  reservedParameters << "name" << "booktype" << "songs" << "songslist"
                     << "template";

  // "shadeColor" << "fontSize";
  //"{\"name\":\"boxshade\", \"description\":\"Box Shade\", \"type\":\"color\"}"

  // read template file
  QSettings settings;
  QString workingPath = settings.value("workingPath", QString("%1/").arg(QDir::currentPath())).toString();
  QFile file(QString("%1/templates/%2").arg(workingPath).arg(templateFilename));
  if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      QTextStream in(&file);
      QRegExp jsonFilter("^%%:");
      QString line;
      json = "(";
      do {
        line = in.readLine();
        if (line.startsWith("%%:"))
          {
            json += line.remove(jsonFilter) + "\n";
          }
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
      QVariant oldValue;
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
      while (it.hasNext())
        {
          it.next();
          svName = it.value().property("name");
          if (!reservedParameters.contains(svName.toString()))
            {
              svDescription = it.value().property("description");
              svDefault = it.value().property("default");
              svType = it.value().property("type");
              if (svType.toString() == QString("string"))
                propertyType = QVariant::String;
              else if (svType.toString() == QString("color"))
                propertyType = QVariant::Color;
              else
                propertyType = QVariant::String;

              item = m_propertyManager
                ->addProperty(propertyType, svDescription.toString());
              if (oldValues.contains(svName.toString()))
                {
                  item->setValue(oldValues.value(svName.toString()));
                }
              else if (svDefault.isValid())
                {
                  item->setValue(svDefault.toVariant());
                }
              m_parameters.insert(svName.toString(), item);
              m_propertyEditor->addProperty(item);
            }
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
      QVariant value;
      QString string_value;
      while (it != m_parameters.constEnd())
        {
          value = it.value()->value();
          switch (value.type())
            {
            case QVariant::String:
              string_value = value.toString();
              if (!string_value.isEmpty())
                {
                  out << "\"" << it.key() << "\" : \""
                      << string_value.replace('\\',"\\\\") << "\",\n";
                }
              break;
            default:
              break;
            }
          ++it;
        }

      if (!bookType().empty())
        out << "\"booktype\" : [\n    \"" << (bookType().join("\",\n    \"")) << "\"\n  ],\n";

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
          QMap< QString, QtVariantProperty* >::const_iterator it;
          for (it = m_parameters.constBegin(); it != m_parameters.constEnd(); ++it)
            {
              sv = object.property(it.key());
              if (sv.isValid())
                {
                  it.value()->setValue(sv.toVariant());
                }
            }

          // booktype property (always an array)
          sv = object.property("booktype");
          if (sv.isValid() && sv.isArray())
            {
              QStringList items;
              qScriptValueToSequence(sv, items);
              setBookType(items);
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
      update();
      setModified(false);
      setFilename(filename);
    }
  else
    {
      qWarning() << "unable to open file in read mode";
    }
}
