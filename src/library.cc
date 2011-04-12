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
#include "library.hh"

#include <QPixmap>
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlQuery>

#ifndef __APPLE__
#include <QFileSystemWatcher>
#endif // __APPLE__

#include "mainwindow.hh"
#include "utils/utils.hh"

CLibrary::CLibrary(CMainWindow *parent)
  : QSqlTableModel()
  , m_parent(parent)
  , m_directory()
  , m_pixmap()
  , m_songRecord()
#ifndef __APPLE__
  , m_watcher()
#endif // __APPLE__
{
  setEditStrategy(QSqlTableModel::OnManualSubmit);

  m_pixmap = new QPixmap;
  m_pixmap->load(":/icons/fr.png");
  QPixmapCache::insert("french", *m_pixmap);
  m_pixmap->load(":/icons/en.png");
  QPixmapCache::insert("english", *m_pixmap);
  m_pixmap->load(":/icons/es.png");
  QPixmapCache::insert("spanish", *m_pixmap);

  m_songRecord.append(QSqlField("artist", QVariant::String));
  m_songRecord.append(QSqlField("title", QVariant::String));
  m_songRecord.append(QSqlField("lilypond", QVariant::Bool));
  m_songRecord.append(QSqlField("path", QVariant::String));
  m_songRecord.append(QSqlField("album", QVariant::String));
  m_songRecord.append(QSqlField("cover", QVariant::String));
  m_songRecord.append(QSqlField("lang", QVariant::String));

  connect(this, SIGNAL(directoryChanged(const QDir&)), SLOT(update()));

#ifndef __APPLE__
  m_watcher = new QFileSystemWatcher;
  connect(m_watcher, SIGNAL(fileChanged(const QString &)),
	  this, SLOT(updateSong(const QString &)));
#endif // __APPLE__
}

CLibrary::~CLibrary()
{
  delete m_pixmap;

#ifndef __APPLE__
  delete m_watcher;
#endif // __APPLE__
}

QDir CLibrary::directory() const
{
  return m_directory;
}

void CLibrary::setDirectory(const QString &directory)
{
  setDirectory(QDir(directory));
}

void CLibrary::setDirectory(const QDir &directory)
{
  if (m_directory != directory)
    {
      m_directory = directory;
      emit(directoryChanged(m_directory));
    }
}

CMainWindow* CLibrary::parent() const
{
  return m_parent;
}

QVariant CLibrary::data(const QModelIndex &index, int role) const
{
  switch (role)
    {
    case TitleRole:
      return QSqlTableModel::data(sibling(index.row(), 1, index.parent()));
    case ArtistRole:
      return QSqlTableModel::data(sibling(index.row(), 0, index.parent()));
    case AlbumRole:
      return QSqlTableModel::data(sibling(index.row(), 4, index.parent()));
    case CoverRole:
      return QSqlTableModel::data(sibling(index.row(), 5, index.parent()));
    case LilypondRole:
      return QSqlTableModel::data(sibling(index.row(), 2, index.parent()));
    case LanguageRole:
      return QSqlTableModel::data(sibling(index.row(), 6, index.parent()));
    case PathRole:
      return QSqlTableModel::data(sibling(index.row(), 3, index.parent()));
    };

  //Draws lilypondcheck
  if (index.column() == 2)
    {
      if (role == Qt::DisplayRole)
	return QString();

      if (QSqlTableModel::data(index, Qt::DisplayRole).toBool())
	{
	  *m_pixmap = QPixmap(QIcon::fromTheme("audio-x-generic").pixmap(24,24));
	  if (role == Qt::DecorationRole)
	    return *m_pixmap;

	  if (m_pixmap->isNull())
	    return true;

	  if (role == Qt::SizeHintRole)
	    return m_pixmap->size();
	}
      return QString();
    }
  else if (index.column() == 4)
    {
      if (role == Qt::DecorationRole)
	{
	  return data(sibling(index.row(), 5, index.parent()), role);
	}
    }
  else if (index.column() == 5)
    {
      QString imgFile = QSqlTableModel::data(index, Qt::DisplayRole).toString();
      if (Qt::DisplayRole == role)
	return QString();

      *m_pixmap = QIcon::fromTheme("image-missing").pixmap(24,24);;
      if (!imgFile.isEmpty() && QFile::exists(imgFile) && !QPixmapCache::find(imgFile, m_pixmap))
	{
	  *m_pixmap = QPixmap::fromImage(QImage(imgFile).scaledToWidth(24));
	  QPixmapCache::insert(imgFile, *m_pixmap);
	}

      if (role == Qt::DecorationRole)
	return *m_pixmap;

      if (role == Qt::SizeHintRole)
	return m_pixmap->size();
    }
  else if (index.column() == 6)
    {
      if (role == Qt::DisplayRole)
      	return QString();

      QString lang = QSqlTableModel::data(index, Qt::DisplayRole).toString();

      if (role == Qt::ToolTipRole)
      	return lang;

      if (QPixmapCache::find(lang, m_pixmap))
	{
	  if (role == Qt::DecorationRole)
	    return *m_pixmap;

	  if (role == Qt::SizeHintRole)
	    return m_pixmap->size();
	}
    }
  return QSqlTableModel::data(index, role);
}

void CLibrary::update()
{
  // recreate the database
  QSqlQuery query;
  query.exec("DROP TABLE songs");
  query.exec("CREATE TABLE songs ("
  	     "artist text NOT NULL,"
  	     "title text NOT NULL,"
  	     "lilypond bool,"
  	     "path text PRIMARY KEY,"
  	     "album text,"
  	     "cover text,"
  	     "lang text"
  	     ")");

  setTable("songs");
  select();

  // define column titles
  setHeaderData(0, Qt::Horizontal, tr("Artist"));
  setHeaderData(1, Qt::Horizontal, tr("Title"));
  setHeaderData(2, Qt::Horizontal, tr("Lilypond"));
  setHeaderData(3, Qt::Horizontal, tr("Path"));
  setHeaderData(4, Qt::Horizontal, tr("Album"));
  setHeaderData(5, Qt::Horizontal, tr("Cover"));
  setHeaderData(6, Qt::Horizontal, tr("Language"));

  // get the path of each song in the library
  QStringList filter = QStringList() << "*.sg";
  QString path = directory().absoluteFilePath("songs/");
  QStringList paths;

  QDirIterator it(path, filter, QDir::NoFilter, QDirIterator::Subdirectories);
  while(it.hasNext())
    paths.append(it.next());

  parent()->progressBar()->show();
  parent()->progressBar()->setTextVisible(true);
  parent()->progressBar()->setRange(0, paths.size());

  addSongs(paths);

  parent()->progressBar()->setTextVisible(false);
  parent()->progressBar()->hide();
  parent()->statusBar()->showMessage(tr("Song database updated."));
}

void CLibrary::addSongs(const QStringList &paths)
{
  QSqlDatabase db = QSqlDatabase::database();
  db.transaction();

  // run through the library songs files
  uint count = 0;
  QStringListIterator filepath(paths);
  while (filepath.hasNext())
    {
      parent()->progressBar()->setValue(++count);
      addSong(filepath.next());
    }
  submitAll();
  db.commit();

#ifndef __APPLE__
  if (!m_watcher->files().isEmpty())
    m_watcher->removePaths(m_watcher->files());
  m_watcher->addPaths(paths);
#endif // __APPLE__

  emit(wasModified());
}

QRegExp CLibrary::reTitle = QRegExp("beginsong\\{([^[\\}]+)");
QRegExp CLibrary::reArtist("by=([^[,|\\]]+)");
QRegExp CLibrary::reAlbum(",album=([^[\\]]+)");
QRegExp CLibrary::reLilypond("\\\\lilypond");
QRegExp CLibrary::reLanguage("selectlanguage\\{([^[\\}]+)");
QRegExp CLibrary::reCoverName(",cov=([^[,]+)");

bool CLibrary::parseSong(const QString &path, Song &song)
{
  QFile file(path);

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      qWarning() << "CLibrary::parseSong: unable to open " << path;
      return false;
    }

  QTextStream stream (&file);
  stream.setCodec("UTF-8");
  QString fileStr = stream.readAll();
  file.close();

  song.path = path;

  reArtist.indexIn(fileStr);
  song.artist = SbUtils::latexToUtf8(reArtist.cap(1));

  reTitle.indexIn(fileStr);
  song.title = SbUtils::latexToUtf8(reTitle.cap(1));
  
  reAlbum.indexIn(fileStr);
  song.album = SbUtils::latexToUtf8(reAlbum.cap(1));

  song.isLilypond = QBool(reLilypond.indexIn(fileStr) > -1);

  reCoverName.indexIn(fileStr);
  song.coverName = reCoverName.cap(1);

  song.coverPath = QFileInfo(path).absolutePath();

  reLanguage.indexIn(fileStr);
  song.language = reLanguage.cap(1);
    
  return true;
}

void CLibrary::addSong(const QString &path)
{
  Song song;
  parseSong(path, song);

  m_songRecord.setValue(0, song.artist);
  m_songRecord.setValue(1, song.title);
  m_songRecord.setValue(2, song.isLilypond);
  m_songRecord.setValue(3, song.path);
  m_songRecord.setValue(4, song.album);
  m_songRecord.setValue(5, QString("%1/%2.jpg")
				    .arg(song.coverPath)
				    .arg(song.coverName));
  m_songRecord.setValue(6, song.language);

  if (!insertRecord(-1, m_songRecord))
    {
      qWarning() << "CLibrary::addSongFromFile: unable to insert song " << path;
    }
  m_songRecord.clearValues();
}

void CLibrary::removeSong(const QString &path)
{
  QModelIndex index = createIndex(0,3); //column 3 stores the path
  QModelIndexList list = match(index, Qt::DisplayRole, path, -1, Qt::MatchExactly);

  foreach(index, list)
    removeRows(index.row(), 1);

  submitAll();
  emit(wasModified());
}

void CLibrary::updateSong(const QString &path)
{
  removeSong(path);
  addSong(path);
  emit(wasModified());
}

bool CLibrary::containsSong(const QString &path)
{
  QModelIndex index = createIndex(0,3);
  QModelIndexList list = match(index, PathRole, path, -1, Qt::MatchExactly);
  return !list.isEmpty();
}
