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
#ifndef __SONGBOOK_HH__
#define __SONGBOOK_HH__

#include <QObject>
#include <QString>
#include <QStringList>

#include <QtVariantProperty>
#include <QtGroupBoxPropertyBrowser>

class QWidget;
class QComboBox;
class QtGroupPropertyManager;
class CMainWindow;
class CUnitPropertyManager;
class CFilePropertyManager;

class CSongbook : public QObject
{
  Q_OBJECT
  Q_PROPERTY(bool modified READ isModified WRITE setModified NOTIFY wasModified)
  Q_PROPERTY(QString workingPath READ workingPath WRITE setWorkingPath)
  Q_PROPERTY(QString filename READ filename WRITE setFilename)
  Q_PROPERTY(QString tmpl READ tmpl WRITE setTmpl)
  Q_PROPERTY(QStringList songs READ songs WRITE setSongs)
    
    
public slots:
  void setWorkingPath(QString path);
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
  QString filename() const;
  QString tmpl() const;
  QString title() const;
  QString authors() const;
  QString style() const;
  
  QStringList songs();

  bool isModified();

  QWidget *panel();

signals:
  void wasModified(bool modified);
  void songsChanged();

private slots:
  void changeTemplate(const QString &filename = QString());

private:
  QString m_workingPath;
  QString m_filename;
  QString m_tmpl;

  QStringList m_songs;

  bool m_modified;

  // panel widgets
  QWidget *m_panel;

  QComboBox *m_templateComboBox;

  QtVariantPropertyManager *m_propertyManager;
  CUnitPropertyManager *m_unitManager;
  CFilePropertyManager *m_fileManager;
  QtGroupBoxPropertyBrowser *m_propertyEditor;

  QStringList m_templates;
  QMap< QString, QtVariantProperty* > m_parameters;

  QtGroupPropertyManager *m_groupManager;
  QtProperty *m_advancedParameters;
};

#endif // __SONGBOOK_HH__
