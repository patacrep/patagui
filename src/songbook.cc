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

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSlider>
#include <QPushButton>
#include <QToolButton>
#include <QColor>
#include <QColorDialog>
#include <QFileDialog>
#include <QDir>
#include <QRadioButton>
#include <QCheckBox>
#include <QGroupBox>
#include <QButtonGroup>

#include <QScriptEngine>

#include <QDebug>

CSongbook::CSongbook()
  : QObject()
  , m_filename()
  , m_title()
  , m_subtitle()
  , m_author()
  , m_version()
  , m_mail()
  , m_picture()
  , m_pictureCopyright()
  , m_footer()
  , m_licence()
  , m_shadeColor()
  , m_fontSize()
  , m_tmpl()
  , m_bookType()
  , m_songs()
  , m_panel()
{}

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

QString CSongbook::title()
{
  return m_title;
}
void CSongbook::setTitle(const QString &title)
{
  if (m_title != title)
    {
      setModified(true);
      m_title = title;
    }
}

QString CSongbook::subtitle()
{
  return m_subtitle;
}
void CSongbook::setSubtitle(const QString &subtitle)
{
  if (m_subtitle != subtitle)
    {
      setModified(true);
      m_subtitle = subtitle;
    }
}

QString CSongbook::author()
{
  return m_author;
}
void CSongbook::setAuthor(const QString &author)
{
  if (m_author != author)
    {
      setModified(true);
      m_author = author;
    }
}

QString CSongbook::version()
{
  return m_version;
}
void CSongbook::setVersion(const QString &version)
{
  if (m_version != version)
    {
      setModified(true);
      m_version = version;
    }
}

QString CSongbook::mail()
{
  return m_mail;
}
void CSongbook::setMail(const QString &mail)
{
  if (m_mail != mail)
    {
      setModified(true);
      m_mail = mail;
    }
}

QString CSongbook::picture()
{
  return m_picture;
}
void CSongbook::setPicture(const QString &picture)
{
  if (m_picture != picture)
    {
      setModified(true);
      m_picture = picture;
    }
}

QString CSongbook::pictureCopyright()
{
  return m_pictureCopyright;
}
void CSongbook::setPictureCopyright(const QString &pictureCopyright)
{
  if (m_pictureCopyright != pictureCopyright)
    {
      setModified(true);
      m_pictureCopyright = pictureCopyright;
    }
}

QString CSongbook::footer()
{
  return m_footer;
}
void CSongbook::setFooter(const QString &footer)
{
  if (m_footer != footer)
    {
      setModified(true);
      m_footer = footer;
    }
}

QString CSongbook::licence()
{
  return m_licence;
}
void CSongbook::setLicence(const QString &licence)
{
  if (m_licence != licence)
    {
      setModified(true);
      m_licence = licence;
    }
}

QString CSongbook::shadeColor()
{
  return m_shadeColor;
}
void CSongbook::setShadeColor(const QString &shadeColor)
{
  if (m_shadeColor != shadeColor)
    {
      setModified(true);
      m_shadeColor = shadeColor;
    }
}

QString CSongbook::fontSize()
{
  return m_fontSize;
}
void CSongbook::setFontSize(const QString &fontSize)
{
  if (m_fontSize != fontSize)
    {
      setModified(true);
      m_fontSize = fontSize;
    }
}

QString CSongbook::tmpl()
{
  return m_tmpl;
}
void CSongbook::setTmpl(const QString &tmpl)
{
  if (m_tmpl != tmpl)
    {
      setModified(true);
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

      QStringList properties;
      QString property;
      QString value;
      properties << "title" << "author" << "subtitle" << "mail" 
                 << "version" << "picture" << "footer" << "licence"
                 << "pictureCopyright" << "shadeColor" << "fontSize";
      foreach (property, properties)
        {
          value = QObject::property(property.toStdString().c_str()).toString();
          if (!value.isEmpty())
            {
              out << "\"" << property.toLower() 
                  << "\" : \"" 
                  << value.replace('\\',"\\\\") << "\",\n";
            }
        }

      if (!bookType().empty())
        out << "\"booktype\" : [\n    \"" << (bookType().join("\",\n    \"")) << "\"\n  ],\n";

      out << "\"songs\" : [\n    \"" << (songs().join("\",\n    \"")) << "\"\n  ]\n}\n";
      file.close();
      setModified(false);
      setFilename(filename);
    }
  else
    {
      qWarning() << "unable to open file in write mode";
    }
}

void CSongbook::load(const QString & filename)
{
  QFile file(filename);
  if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      reset();

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
            setTmpl(sv.toString());

          // default property
          QStringList properties;
          QString property;
          QVariant variant;
          properties << "title" << "author" << "subtitle" << "mail" 
                     << "version" << "picture" << "footer" << "licence"
                     << "pictureCopyright" << "shadeColor" << "fontSize";
          foreach (property, properties)
            {
              sv = object.property(property.toLower());
              if (sv.isValid())
                {
                  variant = sv.toVariant();
                  setProperty(property.toStdString().c_str(), variant);
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
      setModified(false);
      setFilename(filename);
      update();
    }
  else
    {
      qWarning() << "unable to open file in read mode";
    }
}

QWidget * CSongbook::panel()
{
  if (!m_panel)
    {
      m_panel = new QWidget;

      m_titleEdit = new QLineEdit(title());
      m_subtitleEdit = new QLineEdit(subtitle());
      m_authorEdit = new QLineEdit(author());
      m_versionEdit = new QLineEdit(version());
      m_mailEdit = new QLineEdit(mail());
      m_pictureEdit = new QLineEdit(picture());
      m_pictureCopyrightEdit = new QLineEdit(pictureCopyright());
      m_pictureEdit->setReadOnly(true);
      m_footerEdit = new QLineEdit(footer());
      m_licenceEdit = new QLineEdit(licence());
    
      QToolButton *browsePictureButton = new QToolButton;
      browsePictureButton->setIcon(QIcon(":/icons/document-load.png"));
      connect(browsePictureButton, SIGNAL(clicked()),
	      this, SLOT(browsePicture()));
  
      m_shadeColorLabel = new QLabel;
      QColor shade(QString("#%1").arg(shadeColor()));
      m_shadeColorLabel->setText(shade.name());
      m_shadeColorLabel->setPalette(QPalette(shade));
      m_shadeColorLabel->setAutoFillBackground(true);
      
      QPushButton *pickShadeColorButton = new QPushButton(tr("Change"));
      connect(pickShadeColorButton, SIGNAL(clicked()),
	      this, SLOT(pickShadeColor()));  
      
      m_fontSizeSlider = new QSlider(Qt::Horizontal);
      m_fontSizeSlider->setRange(0,4);
      m_fontSizeSlider->setPageStep(1);
      m_fontSizeSlider->setSingleStep(1);
      m_fontSizeSlider->setTickPosition(QSlider::TicksBelow);
      m_fontSizeSlider->setValue(2);

      // BookType
      m_chordbookRadioButton = new QRadioButton(tr("Chordbook"));
      m_lyricbookRadioButton = new QRadioButton(tr("Lyricbook"));
      m_diagramCheckBox = new QCheckBox(tr("Chord Diagram"));
      m_lilypondCheckBox = new QCheckBox(tr("Lilypond"));
      m_tablatureCheckBox = new QCheckBox(tr("Tablature"));

      QButtonGroup *bookTypeGroup = new QButtonGroup();
      bookTypeGroup->addButton(m_chordbookRadioButton);
      bookTypeGroup->addButton(m_lyricbookRadioButton);

      m_chordbookRadioButton->setChecked(true);

      // connect modification signal
      connect(m_titleEdit, SIGNAL(textChanged(QString)),
              this, SLOT(setTitle(QString)));
      connect(m_subtitleEdit, SIGNAL(textChanged(QString)),
              this, SLOT(setSubtitle(QString)));
      connect(m_authorEdit, SIGNAL(textChanged(QString)),
              this, SLOT(setAuthor(QString)));
      connect(m_versionEdit, SIGNAL(textChanged(QString)),
              this, SLOT(setVersion(QString)));
      connect(m_mailEdit, SIGNAL(textChanged(QString)),
              this, SLOT(setMail(QString)));
      connect(m_pictureEdit, SIGNAL(textChanged(QString)),
              this, SLOT(setPicture(QString)));
      connect(m_pictureCopyrightEdit, SIGNAL(textChanged(QString)),
              this, SLOT(setPictureCopyright(QString)));
      connect(m_footerEdit, SIGNAL(textChanged(QString)),
              this, SLOT(setFooter(QString)));
      connect(m_licenceEdit, SIGNAL(textChanged(QString)),
              this, SLOT(setLicence(QString)));
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
      
      QGridLayout *layout = new QGridLayout;
      int line = -1;
      // title page
      layout->addWidget(new QLabel(tr("Title:")),++line,0,1,1);
      layout->addWidget(m_titleEdit,line,1,1,3);
      layout->addWidget(new QLabel(tr("Subtitle:")),++line,0,1,1);
      layout->addWidget(m_subtitleEdit,line,1,1,3);
      layout->addWidget(new QLabel(tr("Author:")),++line,0,1,1);
      layout->addWidget(m_authorEdit,line,1,1,3);
      layout->addWidget(new QLabel(tr("Version:")),++line,0,1,1);
      layout->addWidget(m_versionEdit,line,1,1,3);
      layout->addWidget(new QLabel(tr("Mail:")),++line,0,1,1);
      layout->addWidget(m_mailEdit,line,1,1,3);
      layout->addWidget(new QLabel(tr("Picture:")),++line,0,1,1);
      layout->addWidget(m_pictureEdit,line,1,1,3);
      layout->addWidget(browsePictureButton,line,3,1,1);
      layout->addWidget(new QLabel(tr("Copyright:")),++line,0,1,1);
      layout->addWidget(m_pictureCopyrightEdit,line,1,1,3);
      layout->addWidget(new QLabel(tr("Footer:")),++line,0,1,1);
      layout->addWidget(m_footerEdit,line,1,1,3);
      layout->addWidget(new QLabel(tr("Licence:")),++line,0,1,1);
      layout->addWidget(m_licenceEdit,line,1,1,3);
      // custom options
      layout->addWidget(new QLabel(tr("Shade Color:")),++line,0,1,1);
      layout->addWidget(m_shadeColorLabel,line,1,1,2);
      layout->addWidget(pickShadeColorButton,line,3,1,1);
      layout->addWidget(new QLabel(tr("Font Size:")),++line,0,1,1);
      layout->addWidget(new QLabel(tr("small")),line,1,1,1);
      layout->addWidget(m_fontSizeSlider,line,2,1,1);
      layout->addWidget(new QLabel(tr("large")),line,3,1,1);

      
      layout->addWidget(m_chordbookRadioButton,++line,0,1,2);
      layout->addWidget(m_lyricbookRadioButton,line,2,1,2);
      layout->addWidget(m_diagramCheckBox,++line,0,1,2);
      layout->addWidget(m_lilypondCheckBox,line,2,1,2);
      layout->addWidget(m_tablatureCheckBox,++line,0,1,2);

      m_panel->setLayout(layout);
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
      m_diagramCheckBox->setEnabled(true);
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
  QObject::setProperty("tmpl",QString());
  QStringList properties;
  QString property;
  properties << "filename" << "title" << "author" << "subtitle" << "mail" 
             << "version" << "picture" << "footer" << "licence"
             << "pictureCopyright" << "shadeColor" << "fontSize";
  foreach (property, properties)
    {
      QObject::setProperty(property.toStdString().c_str(),QString());
    }

  setBookType(QStringList()<<"chorded");

  setModified(false);
  update();
}

void CSongbook::update()
{
  m_titleEdit->setText(title());
  m_subtitleEdit->setText(subtitle());
  m_authorEdit->setText(author());
  m_versionEdit->setText(version());
  m_mailEdit->setText(mail());
  m_pictureEdit->setText(picture());
  m_pictureCopyrightEdit->setText(pictureCopyright());
  m_licenceEdit->setText(licence());
  m_footerEdit->setText(footer());

  qDebug() << m_bookType;

  if (m_bookType.contains("lyric"))
    {
      m_lyricbookRadioButton->setChecked(true);
    }
  else if (m_bookType.contains("chorded"))
    {
      m_chordbookRadioButton->setChecked(true);
      m_diagramCheckBox->setChecked(m_bookType.contains("diagram"));
      m_lilypondCheckBox->setChecked(m_bookType.contains("lilypond"));
      m_tablatureCheckBox->setChecked(m_bookType.contains("tabs"));
    }
}

void CSongbook::pickShadeColor()
{
  QColor color = QColorDialog::getColor(QColor(), m_panel);
  m_shadeColorLabel->setText(color.name());
  m_shadeColorLabel->setPalette(QPalette(color));
  setShadeColor(color.name());
}

void CSongbook::browsePicture()
{
  //todo: right now, only .jpg is supported since it's hardcoded in dockWidgets
  //problem is that in mybook.tex, there's just the basename so its extension 
  //should be guessed from somewhere else.
  QString filename = QFileDialog::getOpenFileName(m_panel, tr("Open Image File"),
						  QDir::home().path(),
						  tr("Images (*.jpg)"));
  if (!filename.isEmpty())
    {
      m_pictureEdit->setText(filename);
      setPicture(filename);
    }
}
