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

#include <QtGui>

#include "main-window.hh"
#include "utils/utils.hh"

#include <QDebug>

CLibrary::CLibrary(CMainWindow *parent)
  : QAbstractTableModel()
  , m_parent(parent)
  , m_directory()
  , m_completionModel(new QStringListModel(this))
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
  setDirectory(settings.value("workingPath", QDir::homePath()).toString());
  settings.endGroup();
}

void CLibrary::writeSettings()
{
  QSettings settings;
  settings.beginGroup("library");
  settings.setValue("workingPath", directory().canonicalPath());
  settings.endGroup();
}

QDir CLibrary::directory() const
{
  return m_directory;
}

void CLibrary::setDirectory(const QString &directory)
{
  if(!directory.isEmpty())
    setDirectory(QDir(directory));
}

void CLibrary::setDirectory(const QDir &directory)
{
  if (m_directory != directory)
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

QAbstractListModel * CLibrary::completionModel()
{
  return m_completionModel;
}


CMainWindow* CLibrary::parent() const
{
  return m_parent;
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
      return qVariantFromValue(m_songs[index.row()].language);
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
  while(it.hasNext())
    paths.append(it.next());

  parent()->progressBar()->show();
  parent()->progressBar()->setTextVisible(true);
  parent()->progressBar()->setRange(0, paths.size());

  addSongs(paths);

  QStringList wordList;
  for (int i = 0; i < rowCount(); ++i)
    {
      wordList << data(index(i,0),CLibrary::TitleRole).toString()
	       << data(index(i,0),CLibrary::ArtistRole).toString()
	       << data(index(i,0),CLibrary::PathRole).toString();
    }
  wordList.removeDuplicates();
  m_completionModel->setStringList(wordList);

  parent()->progressBar()->setTextVisible(false);
  parent()->progressBar()->hide();
  parent()->statusBar()->showMessage(tr("Song database updated."));
  emit(wasModified());
}

void CLibrary::addSongs(const QStringList &paths)
{
  // run through the library songs files
  uint count = 0;
  QStringListIterator filepath(paths);
  while (filepath.hasNext())
    {
      parent()->progressBar()->setValue(++count);
      addSong(filepath.next());
    }
  reset();
  emit(wasModified());
}

QRegExp CLibrary::reSong("begin\\{?song\\}?\\{([^[\\}]+)\\}[^[]*\\[([^]]*)\\]");
QRegExp CLibrary::reArtist("by=([^,]+)");
QRegExp CLibrary::reAlbum("album=([^,]+)");
QRegExp CLibrary::reCoverName("cov=([^,]+)");
QRegExp CLibrary::reLilypond("\\\\lilypond");
QRegExp CLibrary::reLanguage("selectlanguage\\{([^\\}]+)");


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

  reSong.indexIn(fileStr);
  song.title = SbUtils::latexToUtf8(reSong.cap(1));

  reArtist.indexIn(reSong.cap(2));
  song.artist = SbUtils::latexToUtf8(reArtist.cap(1));

  reAlbum.indexIn(reSong.cap(2));
  song.album = SbUtils::latexToUtf8(reAlbum.cap(1));

  song.isLilypond = QBool(reLilypond.indexIn(fileStr) > -1);

  reCoverName.indexIn(reSong.cap(2));
  song.coverName = reCoverName.cap(1);

  song.coverPath = QFileInfo(path).absolutePath();

  reLanguage.indexIn(fileStr);
  song.language = languageFromString(reLanguage.cap(1));

  return true;
}

void CLibrary::addSong(const QString &path)
{
  Song song;
  parseSong(path, song);
  m_songs << song;
}

void CLibrary::removeSong(const QString &path)
{
  for (int i = 0; i < m_songs.size(); ++i)
    {
      if (m_songs[i].path == path)
	m_songs.removeAt(i);
    }
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
  for (int i = 0; i < m_songs.size(); ++i)
    {
      if (m_songs[i].path == path)
	return true;
    }
  return false;
}

int CLibrary::rowCount(const QModelIndex &) const
{
  return m_songs.size();
}

int CLibrary::columnCount(const QModelIndex &) const
{
  return 6;
}

QLocale::Language CLibrary::languageFromString(const QString &languageName)
{
  if (languageName == "french")
    return QLocale::French;
  else if (languageName == "english")
    return QLocale::English;
  else if (languageName == "spanish")
    return QLocale::Spanish;

  return QLocale::system().language();
}
