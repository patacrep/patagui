// Copyright (C) 2009 Romain Goffe, Alexandre Dupas
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

/**
 * \file library.hh
 *
 * Class for representing the songlibrary.
 *
 */
#ifndef __LIBRARY_HH__
#define __LIBRARY_HH__

#include <QSqlTableModel>
#include <QString>
#include <QDir>

class QPixmap;

#ifndef __APPLE__
class QFileSystemWatcher;
#endif // __APPLE__

class CMainWindow;

class CLibrary : public QSqlTableModel
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
    MaxRole = PathRole
  };

  CLibrary(CMainWindow* parent);
  ~CLibrary();

  QDir directory() const;
  void setDirectory(const QString &directory);
  void setDirectory(const QDir &directory);

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

  void addSong(const QString & path);
  void addSongs(const QStringList &paths);
  void removeSong(const QString & path);
  bool containsSong(const QString & path);

public slots:
  void update();
  void updateSong(const QString & path);

signals:
  void wasModified();
  void directoryChanged(const QDir &directory);

private:
  CMainWindow *parent() const;

  CMainWindow *m_parent;
  QDir m_directory;
  QPixmap* m_pixmap;

#ifndef __APPLE__
  QFileSystemWatcher* m_watcher;
#endif // __APPLE__
};

#endif // __LIBRARY_HH__
