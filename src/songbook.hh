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

class CLibrary;

class QWidget;
class QComboBox;
class QtGroupBoxPropertyBrowser;
class QtGroupPropertyManager;
class CUnitPropertyManager;
class CFilePropertyManager;

/**
 * \file songbook.hh
 * \class CSongbook
 * \brief CSongbook is the model representing a songbook
 *
 * A songbook is a composed of a list of songs and a set of options
 * that defines its PDF output.
 *
 * A CSongbook class extracts the list of selected songs in CLibraryView,
 * manages all the songbook options through the QtGroupBoxPropertyBrowser framework,
 * and can be saved/loaded as a .sb file.
 *
 * LaTeX options than can be used within a songbook are defined in a template
 * file (.tmpl) that is located in the songbook application's template/ directory.
 *
 */
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

  void checkAll();
  void uncheckAll();
  void toggleAll();

  void setChecked(const QModelIndex &index, bool checked);
  void toggle(const QModelIndex &index);

public:
  CSongbook(QObject *parent);
  ~CSongbook();

  QString workingPath() const;

  void setLibrary(CLibrary *library);
  CLibrary * library() const;

  QString filename() const;
  QString tmpl() const;

  int selectedCount() const;
  void selectLanguages(const QStringList &languages);

  void songsFromSelection();
  void songsToSelection();

  QStringList songs();

  bool isModified();

  void initializeEditor(QtGroupBoxPropertyBrowser *editor);

  virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  virtual Qt::ItemFlags flags(const QModelIndex &index) const;
  virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

  bool isChecked(const QModelIndex &index);

signals:
  void wasModified(bool modified);
  void songsChanged();

private slots:
  void sourceModelAboutToBeReset();
  void sourceModelReset();

private:
  CLibrary *m_library;
  QString m_filename;
  QString m_tmpl;

  QList< bool > m_selectedSongs;
  QStringList m_songs;

  bool m_modified;

  QtVariantPropertyManager *m_propertyManager;
  CUnitPropertyManager *m_unitManager;
  CFilePropertyManager *m_fileManager;
  QtGroupPropertyManager *m_groupManager;

  QMap< QString, QtVariantProperty* > m_parameters;

  QList< QtProperty*> m_mandatoryParameters;
  QtProperty *m_advancedParameters;
};

#endif // __SONGBOOK_HH__
