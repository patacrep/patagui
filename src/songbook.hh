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

#include "identity-proxy-model.hh"

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

class CSongbook : public CIdentityProxyModel
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

  void changeTemplate(const QString &filename = QString());

  void selectAll();
  void unselectAll();
  void invertSelection();

public:
  CSongbook(QObject *parent);
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
  
  int selectedCount() const;
  QStringList selectedPaths() const;
  void selectLanguages(const QStringList &languages);
  bool selectPaths(QStringList &paths);
  void setSongsFromSelectedPaths();

  QStringList songs();

  bool isModified();

  QWidget *panel();

  virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  virtual Qt::ItemFlags flags(const QModelIndex &index) const;
  virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

signals:
  void wasModified(bool modified);
  void songsChanged();

protected:
  QList< bool > m_selectedSongs;
  QStringList m_selectedPaths;

private slots:
  void sourceModelAboutToBeReset();
  void sourceModelReset();

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
