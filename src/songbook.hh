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
//******************************************************************************
#ifndef __SONGBOOK_HH__
#define __SONGBOOK_HH__

#include <QObject>

#include <QDir>
#include <QString>
#include <QStringList>

#include <QtVariantProperty>
#include <QtGroupBoxPropertyBrowser>

#include "utils/utils.hh"

class CLibrary;

class QWidget;
class QComboBox;
class QtGroupPropertyManager;
class CUnitPropertyManager;
class CFilePropertyManager;

class CSongbook : public QObject
{
  Q_OBJECT
  Q_PROPERTY(bool modified READ isModified WRITE setModified NOTIFY wasModified)
  Q_PROPERTY(QString filename READ filename WRITE setFilename)
  Q_PROPERTY(QString tmpl READ tmpl WRITE setTmpl)
  Q_PROPERTY(QStringList songs READ songs WRITE setSongs)

public slots:
  void setFilename(const QString &filename);
  void setTmpl(const QString &tmpl);
  void setSongs(QStringList songs);

  void reset();
  void save(const QString &filename);
  void load(const QString &filename);
  void setModified(bool modified);

public:
  CSongbook();
  ~CSongbook();

  QString workingPath() const;

  void setLibrary(CLibrary *library);
  CLibrary * library() const;

  QString filename() const;
  QString tmpl() const;
  QString title() const;
  QString authors() const;
  QString style() const;
  QPixmap* picture() const;
  QtGroupBoxPropertyBrowser * propertyEditor() const;
  
  QStringList songs();

  bool isModified();

  QWidget *panel();

signals:
  void wasModified(bool modified);
  void songsChanged();

public slots:
  void changeTemplate(const QString &filename = QString());

private:
  CLibrary *m_library;
  QString m_filename;
  QString m_tmpl;

  QStringList m_songs;

  bool m_modified;

  QtVariantPropertyManager *m_propertyManager;
  CUnitPropertyManager *m_unitManager;
  CFilePropertyManager *m_fileManager;
  QtGroupBoxPropertyBrowser *m_propertyEditor;

  QMap< QString, QtVariantProperty* > m_parameters;

  QtGroupPropertyManager *m_groupManager;
  QtProperty *m_advancedParameters;
};

#endif // __SONGBOOK_HH__
