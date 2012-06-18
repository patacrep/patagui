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

/**
 * \file library.hh
 * \class CLibrary
 * \brief CLibrary is the base model that corresponds to the list of songs
 *
 * A CLibrary is a list of Song (.sg files) that are fetched from a
 * local directory.
 * This model is used to build an intermediate model (CSongSortFilterProxyModel)
 * that allows filtering options, and is then presented in the library tab (CTabWidget)
 * of the main window (CMainWindow) through its associated view (CLibraryView).
 *
 */
class CLibrary : public QAbstractTableModel
{
  Q_OBJECT
  Q_PROPERTY(QDir directory READ directory WRITE setDirectory)

public:
  enum Roles {
    TitleRole = Qt::UserRole + 1,
    ArtistRole = Qt::UserRole + 2,
    AlbumRole = Qt::UserRole + 3,
    CoverRole = Qt::UserRole + 4,
    LilypondRole = Qt::UserRole + 5,
    LanguageRole = Qt::UserRole + 6,
    PathRole = Qt::UserRole + 7,
    CoverSmallRole = Qt::UserRole + 8,
    CoverFullRole = Qt::UserRole + 9,
    RelativePathRole = Qt::UserRole + 10,
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

  QAbstractListModel * completionModel() const;
  QAbstractListModel * artistCompletionModel() const;
  QAbstractListModel * albumCompletionModel() const;

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

  virtual int rowCount(const QModelIndex &index = QModelIndex()) const;
  virtual int columnCount(const QModelIndex &index = QModelIndex()) const;

  QString pathToSong(const QString &artist, const QString &title) const;
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
