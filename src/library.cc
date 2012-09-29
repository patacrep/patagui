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

#include "main-window.hh"
#include "progress-bar.hh"

#include <QStringListModel>
#include <QDirIterator>
#include <QPixmapCache>
#include <QStatusBar>
#include <QDesktopServices>
#include <QSettings>
#include <QMessageBox>

#include <QDebug>

namespace // anonymous namespace
{
  QString stringToFilename(const QString & string, const QString & separator)
  {
    QString result(string.toLower());

    // replace whitespaces with separator
    result.replace(QRegExp("\\W+"), separator);
    result.remove(QRegExp(QString("%1+$").arg(separator)));

    // replace utf8 characters
    result.replace(QRegExp("[àâ]"), "a");
    result.replace(QRegExp("[ïî]"), "i");
    result.replace(QRegExp("[óô]"), "o");
    result.replace(QRegExp("[ùúû]"), "u");
    result.replace(QRegExp("[éèêë]"), "e");
    result.replace(QString("ñ"), "n");
    result.replace(QString("ç"), "c");

    return result;
  }
}

CLibrary::CLibrary(CMainWindow *parent)
  : QAbstractTableModel()
  , m_parent(parent)
  , m_directory()
  , m_completionModel(new QStringListModel(this))
  , m_artistCompletionModel(new QStringListModel(this))
  , m_albumCompletionModel(new QStringListModel(this))
  , m_templates()
  , m_songs()
{
  connect(this, SIGNAL(directoryChanged(const QDir&)), SLOT(update()));
}

CLibrary::~CLibrary()
{
  m_songs.clear();
}

void CLibrary::readSettings()
{
  QSettings settings;
  settings.beginGroup("library");
  setDirectory(settings.value("workingPath", findSongbookPath()).toString());
  settings.endGroup();
}

void CLibrary::writeSettings()
{
  QSettings settings;
  settings.beginGroup("library");
  settings.setValue("workingPath", directory().absolutePath());
  settings.endGroup();
}

bool CLibrary::checkSongbookPath(const QString &path)
{
  QDir directory(path);
  return directory.exists()
    && directory.exists("makefile")
    && directory.exists("songbook.py")
    && directory.exists("songs");
}

QString CLibrary::findSongbookPath()
{
  QStringList paths;
  paths << QString("%1/songbook").arg(QDir::homePath())
	<< QString("%1/songbook").arg(QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation));

  QString path;
  foreach(path, paths)
    {
      if (checkSongbookPath(path))
        return path;
    }

  return QDir::homePath();
}

QDir CLibrary::directory() const
{
  return m_directory;
}

void CLibrary::setDirectory(const QString &directory)
{
  if (!directory.isEmpty())
    setDirectory(QDir(directory));
}

void CLibrary::setDirectory(const QDir &directory)
{
  if (directory != m_directory)
    {
      m_directory = directory;
      QDir templatesDirectory(QString("%1/templates").arg(directory.canonicalPath()));
      m_templates = templatesDirectory.entryList(QStringList() << "*.tmpl");
      writeSettings();
      emit(directoryChanged(m_directory));
    }
}

QStringList CLibrary::templates() const
{
  return m_templates;
}

QAbstractListModel * CLibrary::completionModel() const
{
  return m_completionModel;
}

QAbstractListModel * CLibrary::artistCompletionModel() const
{
  return m_artistCompletionModel;
}

QAbstractListModel * CLibrary::albumCompletionModel() const
{
  return m_albumCompletionModel;
}

QVariant CLibrary::headerData (int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
      switch (section)
	{
	case 0:
	  return tr("Title");
	case 1:
	  return tr("Artist");
	case 2:
	  return tr("Lilypond");
	case 3:
	  return tr("Path");
	case 4:
	  return tr("Album");
	case 5:
	  return tr("Language");
	}
    }
  return QVariant();
}

QVariant CLibrary::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  switch (role)
    {
    case Qt::DisplayRole:
      switch (index.column())
	{
	case 0:
	  return data(index, TitleRole);
	case 1:
	  return data(index, ArtistRole);
	case 2:
	  return data(index, LilypondRole);
	case 3:
	  return data(index, PathRole);
	case 4:
	  return data(index, AlbumRole);
        case 5:
          return QLocale::languageToString(qVariantValue< QLocale::Language >(data(index, LanguageRole)));
	}
      break;
    case Qt::ToolTipRole:
      switch (index.column())
        {
        case 2:
          return tr("Lilypond music sheet");
        case 5:
          return QLocale::languageToString(qVariantValue< QLocale::Language >(data(index, LanguageRole)));
        default:
          return QVariant();
        }
      break;
    case TitleRole:
      return m_songs[index.row()].title;
    case ArtistRole:
      return m_songs[index.row()].artist;
    case AlbumRole:
      return m_songs[index.row()].album;
    case CoverRole:
      return QString("%1/%2.jpg")
	.arg(m_songs[index.row()].coverPath)
	.arg(m_songs[index.row()].coverName);
    case LilypondRole:
      return m_songs[index.row()].isLilypond;
    case LanguageRole:
      return qVariantFromValue(m_songs[index.row()].locale.language());
    case PathRole:
      return m_songs[index.row()].path;
    case RelativePathRole:
      return QDir(QString("%1/songs").arg(directory().canonicalPath())).relativeFilePath(m_songs[index.row()].path);
    case CoverSmallRole:
      {
        QPixmap pixmap;
	QFileInfo file = QFileInfo(data(index, CoverRole).toString());
	if (file.exists())
          {
            if (!QPixmapCache::find(file.baseName()+"-small", &pixmap))
              {
                pixmap = QPixmap::fromImage(QImage(file.filePath()).scaledToWidth(24));
                QPixmapCache::insert(file.baseName()+"-small", pixmap);
              }
            return pixmap;
          }
      }
      return QVariant();
    case CoverFullRole:
      {
	QPixmap pixmap;
	QFileInfo file = QFileInfo(data(index, CoverRole).toString());
	if (file.exists())
          {
            if (!QPixmapCache::find(file.baseName()+"-full", &pixmap))
              {
                pixmap = QPixmap::fromImage(QImage(file.filePath()).scaled(128,128));
                QPixmapCache::insert(file.baseName()+"-full", pixmap);
              }
            return pixmap;
          }
      }
      return QVariant();
    }
  return QVariant();
}

void CLibrary::update()
{
  m_songs.clear();

  // get the path of each song in the library
  QStringList filter = QStringList() << "*.sg";
  QString path = directory().absoluteFilePath("songs/");
  QStringList paths;

  QDirIterator it(path, filter, QDir::NoFilter, QDirIterator::Subdirectories);
  while (it.hasNext())
    paths.append(it.next());

  m_parent->statusBar()->showMessage(tr("Updating the library..."));
  m_parent->progressBar()->setCancelable(false);
  m_parent->progressBar()->setTextVisible(true);
  m_parent->progressBar()->setRange(0, paths.size());
  m_parent->progressBar()->show();

  addSongs(paths);

  QStringList wordList;
  QStringList artistList;
  QStringList albumList;
  for (int i = 0; i < rowCount(); ++i)
    {
      wordList << data(index(i,0),CLibrary::TitleRole).toString()
	       << data(index(i,0),CLibrary::ArtistRole).toString()
	       << data(index(i,0),CLibrary::PathRole).toString();

      artistList << data(index(i,0),CLibrary::ArtistRole).toString();

      albumList << data(index(i,0),CLibrary::AlbumRole).toString();
    }
  wordList.removeDuplicates();
  artistList.removeDuplicates();
  albumList.removeDuplicates();
  m_completionModel->setStringList(wordList);
  m_artistCompletionModel->setStringList(artistList);
  m_albumCompletionModel->setStringList(albumList);

  m_parent->progressBar()->setCancelable(true);
  m_parent->progressBar()->setTextVisible(false);
  m_parent->progressBar()->setRange(0, 0);
  m_parent->progressBar()->hide();
  m_parent->statusBar()->showMessage(tr("Library updated."));
  emit(wasModified());
}

int CLibrary::rowCount(const QModelIndex &) const
{
  return m_songs.size();
}

int CLibrary::columnCount(const QModelIndex &) const
{
  return 6;
}

void CLibrary::addSong(const Song &song, bool resetModel)
{
  m_songs << song;

  if (resetModel)
    {
      reset();
      emit(wasModified());
    }
}

void CLibrary::addSongs(const QStringList &paths)
{
  Song song;
  int songCount = 0;
  // run through the library songs files
  QStringListIterator filepath(paths);
  while (filepath.hasNext())
    {
      m_parent->progressBar()->setValue(++songCount);
      loadSong(filepath.next(), &song);
      addSong(song);
    }
  reset();
  emit(wasModified());
}

void CLibrary::addSong(const QString &path)
{
  m_songs << Song::fromFile(path);
}


void CLibrary::removeSong(const QString &path)
{
  for (int i = 0; i < m_songs.size(); ++i)
    {
      if (m_songs[i].path == path)
        {
          m_songs.removeAt(i);
          break;
        }
    }
  reset();
  emit(wasModified());
}

Song CLibrary::getSong(const QString &path) const
{
  for (int i = 0; i < m_songs.size(); ++i)
    {
      if (m_songs[i].path == path)
        return m_songs[i];
    }
  return Song();
}

int CLibrary::getSongIndex(const QString &path) const
{
  for (int i = 0; i < m_songs.size(); ++i)
    {
      if (m_songs[i].path == path)
        return i;
    }
  return -1;
}

void CLibrary::loadSong(const QString &path, Song *song)
{
  if (song == 0)
    return;
  (*song) = Song::fromFile(path);
}

void CLibrary::saveSong(Song &song)
{
  // write the song file
  QFile file(song.path);
  if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
      QTextStream stream (&file);
      stream.setCodec("UTF-8");
      stream << Song::toString(song);
      file.close();
    }
  //update the song in the library
  int index = getSongIndex(song.path);
  if (index != -1)
    m_songs[index] = song;
  else //new song
    addSong(song, true);
}

void CLibrary::saveCover(Song &song, const QImage &cover)
{
  QFileInfo fileInfo(song.path);
  QDir artistDirectory = fileInfo.absoluteDir();

  // update song cover information
  song.coverPath = artistDirectory.absolutePath();
  song.coverName = (!song.album.isEmpty()) ?
    stringToFilename(song.album,  "-") :
    stringToFilename(song.artist, "-"); //fallback on artist name

  // guess the cover filename
  QString coverFilename = QString("%1/%2.jpg").arg(song.coverPath).arg(song.coverName);

  // ask before overwriting
  if (QFile(coverFilename).exists())
    {
      int ret = QMessageBox::warning(0, tr("Songbook-Client"),
				     tr("This file will be overwritten:\n%1\n"
					"Are you sure?").arg(coverFilename),
				     QMessageBox::Cancel | QMessageBox::Ok,
				     QMessageBox::Cancel);
      // actually write the image
      if (ret == QMessageBox::Ok)
	cover.save(coverFilename);
    }
  else
    {
      cover.save(coverFilename);
    }
}

void CLibrary::createArtistDirectory(Song &song)
{
  // if the song is new or comes from an other library, update the
  // song file path
  if (song.path.isEmpty()
      || !song.path.startsWith(directory().path()))
    {
      song.path = pathToSong(song);
    }

  // create the artist directory (if it does not exists)
  QFileInfo fileInfo(song.path);
  QDir artistDirectory = fileInfo.absoluteDir();
  if (!artistDirectory.exists())
    directory().mkpath(artistDirectory.absolutePath());
}

void CLibrary::deleteSong(const QString &path)
{
  // make sure the song is not in the library list anymore
  removeSong(path);

  // remove the file
  QFile file(path);
  if (!file.remove())
    {
      qWarning() << "Unable to remove the song file: " << path;
    }
}

QString CLibrary::pathToSong(const QString &artist, const QString &title) const
{
  QString artistInPath = stringToFilename(artist, "_");
  QString titleInPath = stringToFilename(title, "_");

  return QString("%1/songs/%2/%3.sg")
    .arg(directory().canonicalPath())
    .arg(artistInPath)
    .arg(titleInPath);
}

QString CLibrary::pathToSong(Song &song) const
{
  return pathToSong(song.artist, song.title);
}
