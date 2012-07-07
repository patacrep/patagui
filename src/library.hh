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

#ifndef __LIBRARY_HH__
#define __LIBRARY_HH__

#include "song.hh"

#include <QAbstractTableModel>
#include <QString>
#include <QDir>
#include <QLocale>
#include <QMetaType>

class QAbstractListModel;
class QStringListModel;

class QPixmap;
class CMainWindow;

/*!
  \file library.hh
  \class CLibrary
  \brief CLibrary is the base model that corresponds to the list of songs

  A CLibrary is a list of Song objects (structure representing .sg
  files) that are fetched from a local directory.

  This model is used to build an intermediate model
  (CSongSortFilterProxyModel) that allows filtering options, and is
  then presented in the library tab (CTabWidget) of the main window
  (CMainWindow) through its associated view (CLibraryView).

*/

class CLibrary : public QAbstractTableModel
{
  Q_OBJECT
  Q_PROPERTY(QDir directory READ directory WRITE setDirectory)

public:
  /*!
    \enum Roles
    Each Song in the CLibrary has a set of data elements associated with it, each with its own role.
    The roles are used by the view to indicate to the model which type of data it needs.
  */
  enum Roles {
    TitleRole = Qt::UserRole + 1, /*!< the title of the song item.*/
    ArtistRole = Qt::UserRole + 2, /*!< the artist of the song item.*/
    AlbumRole = Qt::UserRole + 3, /*!< the album of the song item.*/
    CoverRole = Qt::UserRole + 4, /*!< the cover of the song item.*/
    LilypondRole = Qt::UserRole + 5, /*!< whether or not the song item contains lilypond music sheets.*/
    LanguageRole = Qt::UserRole + 6, /*!< the language of the song item.*/
    PathRole = Qt::UserRole + 7, /*!< the absolute path to the .sg file corresponding to the song item.*/
    CoverSmallRole = Qt::UserRole + 8, /*!< the thumbnail cover (22x22) of the song item.*/
    CoverFullRole = Qt::UserRole + 9, /*!< the full cover (128x128) of the song item.*/
    RelativePathRole = Qt::UserRole + 10, /*!< the relative path to the .sg file corresponding to the song item (from the base directory of the songbook).*/
    MaxRole = RelativePathRole
  };

  /// Constructor.
  CLibrary(CMainWindow* parent);
  /// Destructor.
  ~CLibrary();

  void writeSettings();

  bool checkSongbookPath(const QString & path);
  QString findSongbookPath();

  QDir directory() const;
  void setDirectory(const QString &directory);
  void setDirectory(const QDir &directory);

  QStringList templates() const;

  /*!
    Returns the completion model associated with the library.
    The completion model isi based on the list of words from
    title, artist and album columns.
    \sa artistCompletionModel, albumCompletionModel
  */
  QAbstractListModel * completionModel() const;

  /*!
    Returns the artist completion model associated with the library.
    The completion model is based on the list of words from the artist column.
    \sa completionModel, albumCompletionModel
  */
  QAbstractListModel * artistCompletionModel() const;

  /*!
    Returns the album completion model associated with the library.
    The completion model is based on the list of words from the album column.
    \sa completionModel, artistCompletionModel
  */
  QAbstractListModel * albumCompletionModel() const;

  /*!
    Reimplements QAbstractTableModel::headerData.
    \sa data
  */
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

  /*!
    Reimplements QAbstractTableModel::data.
    Returns the data at position \a index for the given \a role.
    \sa headerData
  */
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

  /*!
    Reimplements QAbstractTableModel::rowCount.
    Returns the number of Song items in the library.
    \sa columnCount
  */
  virtual int rowCount(const QModelIndex &index = QModelIndex()) const;

  /*!
    Reimplements QAbstractTableModel::columnCount.
    Returns \a 6 as the number of columns for a song item
    (title, artist, path, album, lilypond, language).
    \sa rowCount
  */
  virtual int columnCount(const QModelIndex &index = QModelIndex()) const;

  /*!
    Returns the absolute path of an .sg file from \a artist and \a title names.
    Following the songbook convention, the song "Hello world!" from artist "Unknown" would
    be located in \a /path/to/songbook/songs/unknown/hello_world.sg".
   */
  QString pathToSong(const QString &artist, const QString &title) const;

  /*!
    This is a convenience method that returns the absolute path of
    an .sg file from a Song object.
  */
  QString pathToSong(Song &song) const;

  //! Add a song to the library list
  void addSong(const Song &song, bool reset=false);

  //! Add a song to the library list
  void addSong(const QString &path);

  //! Add songs to the library list
  void addSongs(const QStringList &paths);

  //! Look if the song is already in the library list
  bool containsSong(const QString &path);

  //! Remove a song from the library list
  void removeSong(const QString &path);

  //! Get the song index from the library list
  int getSongIndex(const QString &path) const;

  //! Get a song from the library list
  Song getSong(const QString &path) const;

  //! Load a song from a file in the library
  void loadSong(const QString &path, Song *song);

  //! Create the artist directory for the current library (if required)
  void createArtistDirectory(Song &song);

  //! Save a song in the library (update the song path if required)
  void saveSong(Song &song);

  //! Save a cover in the library (update the cover path if required)
  void saveCover(Song &song, const QImage &cover);

  //! Destroy a song file from the library
  void deleteSong(const QString &path);

public slots:
  void readSettings();
  void update();

signals:
  void wasModified();
  void directoryChanged(const QDir &directory);

protected:

private:
  CMainWindow *m_parent;
  QDir m_directory;

  QStringListModel *m_completionModel;
  QStringListModel *m_artistCompletionModel;
  QStringListModel *m_albumCompletionModel;

  QStringList m_templates;
  QList< Song > m_songs;
};

Q_DECLARE_METATYPE(QLocale::Language)

#endif // __LIBRARY_HH__
