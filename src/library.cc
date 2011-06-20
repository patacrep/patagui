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

#include "mainwindow.hh"
#include "utils/utils.hh"

CLibrary::CLibrary(CMainWindow *parent)
  : QSqlTableModel()
  , m_parent(parent)
  , m_directory()
  , m_songRecord()
{
  setEditStrategy(QSqlTableModel::OnManualSubmit);

  QPixmapCache::insert("cover-missing-small", QIcon::fromTheme("image-missing", QIcon(":/icons/tango/image-missing")).pixmap(24, 24));
  QPixmapCache::insert("cover-missing-full", QIcon::fromTheme("image-missing", QIcon(":/icons/tango/image-missing")).pixmap(128, 128));
  QPixmapCache::insert("lilypond-checked", QIcon::fromTheme("audio-x-generic", QIcon(":/icons/tango/audio-x-generic")).pixmap(24,24));
  QPixmapCache::insert("french", QIcon::fromTheme("flag-fr", QIcon(":/icons/tango/scalable/places/flag-fr.svg")).pixmap(24,24));
  QPixmapCache::insert("english", QIcon::fromTheme("flag-en", QIcon(":/icons/tango/scalable/places/flag-en.svg")).pixmap(24,24));
  QPixmapCache::insert("spanish", QIcon::fromTheme("flag-es", QIcon(":/icons/tango/scalable/places/flag-es.svg")).pixmap(24,24));

  m_songRecord.append(QSqlField("artist", QVariant::String));
  m_songRecord.append(QSqlField("title", QVariant::String));
  m_songRecord.append(QSqlField("lilypond", QVariant::Bool));
  m_songRecord.append(QSqlField("path", QVariant::String));
  m_songRecord.append(QSqlField("album", QVariant::String));
  m_songRecord.append(QSqlField("cover", QVariant::String));
  m_songRecord.append(QSqlField("lang", QVariant::String));

  connect(this, SIGNAL(directoryChanged(const QDir&)), SLOT(update()));
}

CLibrary::~CLibrary()
{}

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
    case CoverSmallRole:
      {
	QPixmap pixmap;
	QPixmapCache::find("cover-missing-small", &pixmap);
	QFileInfo file = QFileInfo(data(index, CoverRole).toString());
	if (file.exists()
	    && !QPixmapCache::find(file.baseName()+"-small", &pixmap))
	  {
	    pixmap = QPixmap::fromImage(QImage(file.filePath()).scaledToWidth(24));
	    QPixmapCache::insert(file.baseName()+"-small", pixmap);
	  }
	return pixmap;
      }
    case CoverFullRole:
      {
	QPixmap pixmap;
	QPixmapCache::find("cover-missing-full", &pixmap);
	QFileInfo file = QFileInfo(data(index, CoverRole).toString());
	if (file.exists()
	    && !QPixmapCache::find(file.baseName()+"-full", &pixmap))
	  {
	    pixmap = QPixmap::fromImage(QImage(file.filePath()).scaled(128,128));
	    QPixmapCache::insert(file.baseName()+"-full", pixmap);
	  }
	return pixmap;
      }
    }

  //Draws lilypondcheck
  if (index.column() == 2)
    {
      if (role == Qt::DisplayRole)
	return QString();

      if (QSqlTableModel::data(index).toBool())
	{
	  QPixmap pixmap;
	  QPixmapCache::find("lilypond-checked", &pixmap);

	  if (role == Qt::DecorationRole)
	    return pixmap;

	  if (role == Qt::SizeHintRole)
	    return pixmap.size();

	  if (role == Qt::ToolTipRole)
	    return tr("Lilypond music sheet");
	}
    }
  else if (index.column() == 4)
    {
      if (role == Qt::DecorationRole)
	{
	  return data(index, CoverSmallRole);
	}
    }
  else if (index.column() == 5)
    {
      if (Qt::DisplayRole == role)
	return QString();

      if (role == Qt::DecorationRole)
	{
	  return data(index, CoverSmallRole);
	}

      if (role == Qt::SizeHintRole)
	{
	  return data(index, CoverSmallRole).value< QPixmap >().size();
	}
    }
  else if (index.column() == 6)
    {
      QString language = data(index, LanguageRole).toString();
      QPixmap pixmap;
      if (QPixmapCache::find(language, &pixmap))
	{
	  if (role == Qt::DecorationRole)
	    return pixmap;

	  if (role == Qt::SizeHintRole)
	    return pixmap.size();

	  if (role == Qt::ToolTipRole)
	    return language;

	  if (role == Qt::DisplayRole)
	    return QString();
	}
      else
	{
	  if (role == Qt::DisplayRole)
	    return language;
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
