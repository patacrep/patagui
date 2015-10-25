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

class Library;
class QWidget;
class QComboBox;
class QtGroupBoxPropertyBrowser;
class QtGroupPropertyManager;

/*!
  \file songbook.hh
  \class CSongbook
  \brief CSongbook is the model representing a songbook

  A songbook is a composed of a list of songs and a set of options
  that defines its PDF output.

  A CSongbook class extracts the list of selected songs in LibraryView,
  manages all the songbook options through the QtGroupBoxPropertyBrowser framework,
  and can be saved/loaded as a .sb file.

  LaTeX options than can be used within a songbook are defined in a template
  file (.tex) that is located in the songbook application's template/ directory.
*/
class CSongbook : public IdentityProxyModel
{
    Q_OBJECT
    Q_PROPERTY(bool modified READ isModified WRITE setModified NOTIFY wasModified)
    Q_PROPERTY(QString filename READ filename WRITE setFilename)
    Q_PROPERTY(QString tmpl READ tmpl WRITE setTmpl)
    Q_PROPERTY(QStringList songs READ songs WRITE setSongs)
    Q_PROPERTY(QStringList datadirs READ datadirs WRITE setDatadirs)

public slots:
    /*!
    Sets \a filename the filename of the songbook.
    \sa filename
  */
    void setFilename(const QString &filename);

    /*!
    Sets \a tmpl as the template filename of the songbook.
    \sa tmpl
  */
    void setTmpl(const QString &tmpl);

    /*!
    Sets \a songs as the list of songs of the songbook.
    \sa songs
  */
    void setSongs(QStringList songs);

    /*!
    Sets \a datadirs ad the list of datadirs in the songbook
  */
    void setDatadirs(QStringList datadirs);

    /*!
    Resets the model.
  */
    void reset();

    /*!
    Saves this songbook (.sb file) in the file \a filename.
    \sa load
  */
    void save(const QString &filename);

    /*!
    Loads a songbook from the file \a filename (.sb file).
    \sa save
  */
    void load(const QString &filename);

    /*!
    Sets the songbook as modified according to \a value.
    \sa isModified
  */
    void setModified(bool value);

    /*!
    Changes the current template (.tmpl file) of the songbook.
    \sa tmpl, setTmpl
  */
    void changeTemplate(const QString &filename = QString());

    /*!
    Check all songs.
    \sa uncheckAll, toggleAll
  */
    void checkAll();

    /*!
    Uncheck all songs.
    \sa checkAll, toggleAll
  */
    void uncheckAll();

    /*!
    Toggle selection for all songs.
    \sa checkAll, uncheckAll
  */
    void toggleAll();

    /*!
    Sets the song at position \a index as checked according to \a value.
    \sa isChecked, checkAll, uncheckAll, toggleAll
  */
    void setChecked(const QModelIndex &index, bool value);

    /*!
    Toggles the selection of the song at position \a index.
    \sa isChecked, setChecked, checkAll, uncheckAll, toggleAll
  */
    void toggle(const QModelIndex &index);

public:
    /// Constructor.
    CSongbook(QObject *parent);

    /// Destructor.
    ~CSongbook();

    /*!
    Returns the absolute path to the songbook directory.
  */
    QString workingPath() const;

    /*!
    Returns the library.
    \sa setLibrary
  */
    Library * library() const;

    /*!
    Returns the filename of the songbook file (.sb).
    \sa setFilename
  */
    QString filename() const;

    /*!
    Returns the current template for this songbook (.tmpl).
    \sa setTmpl
  */
    QString tmpl() const;

    /*!
    Returns the current datadirs for this songbook.
    \sa setDatadirs
  */
    QStringList datadirs();

    /*!
    Returns the number of selected songs for this songbook.
  */
    int selectedCount() const;
    void selectLanguages(const QStringList &languages);

    /*!
    Updates the list of songs of the songbook from current selection.
    \sa songsToSelection
  */
    void songsFromSelection();

    /*!
    Updates the selection from the list of songs of the songbook.
    \sa songsToSelection
  */
    void songsToSelection();

    /*!
    Returns the list of songs of this songbook.
    \sa songsToSelection
  */
    QStringList songs();

    /*!
    Returns \a true if this songbook has unsaved modifications; \a false otherwise.
    \sa setModified
  */
    bool isModified();

    /*!
    Initialize the template editor.
  */
    void initializeEditor(QtGroupBoxPropertyBrowser *editor);

    /*!
    Returns true if the song at position \a index is checked; \a false otherwise.
  */
    bool isChecked(const QModelIndex &index);

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

signals:
    void wasModified(bool modified);
    void songsChanged();

private slots:
    void sourceModelAboutToBeReset();
    void sourceModelReset();

private:
    QString m_filename;
    QString m_tmpl;
    QStringList m_datadirs;

    QList< bool > m_selectedSongs;
    QStringList m_songs;

    bool m_modified;

    QtVariantPropertyManager *m_propertyManager;
    QtGroupPropertyManager *m_groupManager;

    QMap< QString, QtVariantProperty* > m_parameters;

    QList< QtProperty*> m_mandatoryParameters;
    QtProperty *m_advancedParameters;
};

#endif // __SONGBOOK_HH__
