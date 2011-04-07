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
#include "library.hh"

#include <QPixmap>
#include <QFileSystemWatcher>

#include <QSqlRecord>
#include <QSqlField>
#include <QSqlQuery>

#include "mainwindow.hh"
#include "utils/utils.hh"
using namespace SbUtils;
//------------------------------------------------------------------------------
CLibrary::CLibrary(CMainWindow *parent)
  : QSqlTableModel()
  , m_parent(parent)
{
  setTable("songs");
  setEditStrategy(QSqlTableModel::OnManualSubmit);
  select();
  setHeaderData(0, Qt::Horizontal, tr("Artist"));
  setHeaderData(1, Qt::Horizontal, tr("Title"));
  setHeaderData(2, Qt::Horizontal, tr("Lilypond"));
  setHeaderData(3, Qt::Horizontal, tr("Path"));
  setHeaderData(4, Qt::Horizontal, tr("Album"));
  setHeaderData(5, Qt::Horizontal, tr("Cover"));
  setHeaderData(6, Qt::Horizontal, tr("Language"));

  m_watcher = new QFileSystemWatcher;
  connect(m_watcher, SIGNAL(fileChanged(const QString &)),
	  this, SLOT(updateSong(const QString &)));

  m_pixmap = new QPixmap;
  m_pixmap->load(":/icons/fr.png");
  QPixmapCache::insert("french", *m_pixmap);
  m_pixmap->load(":/icons/en.png");
  QPixmapCache::insert("english", *m_pixmap);
  m_pixmap->load(":/icons/es.png");
  QPixmapCache::insert("spanish", *m_pixmap);
}
//------------------------------------------------------------------------------
CLibrary::~CLibrary()
{
  delete m_pixmap;
}
//------------------------------------------------------------------------------
CMainWindow* CLibrary::parent() const
{
  return m_parent;
}
//------------------------------------------------------------------------------
void CLibrary::retrieveSongs()
{
  QSqlDatabase db = QSqlDatabase::database();

  uint count = 0;
  QStringList filter = QStringList() << "*.sg";
  QString path = QString("%1/songs/").arg(workingPath());
  QStringList paths;
  if(!m_watcher->files().isEmpty())
    m_watcher->removePaths(m_watcher->files());

  QDirIterator it(path, filter, QDir::NoFilter, QDirIterator::Subdirectories);
  db.transaction();
  while(it.hasNext())
    {
      parent()->statusBar()->showMessage(QString(tr("Inserting song : %1")).arg(it.fileInfo().fileName()));
      QString filePath = it.fileInfo().absoluteFilePath();
      if(!filePath.isEmpty())
	paths << filePath;
      parent()->progressBar()->setValue(++count);
      addSong(it.next());
    }
  submitAll();
  db.commit();

#ifndef __APPLE__
  m_watcher->addPaths(paths);
#endif

  emit(wasModified());
}
//------------------------------------------------------------------------------
void CLibrary::addSong(const QString & path)
{
  //do not insert if the song is already in the library
  if(containsSong(path))
    return;

  //qDebug() << "CLibrary::addSong " << path;
  QFile file(path);
  if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      QTextStream stream (&file);
      stream.setCodec("UTF-8");
      QString fileStr = stream.readAll();
      file.close();

      QString artist;
      QString title;
      bool lilypond(false);
      QString album;
      QString coverName;
      QString lang;
 
      //artist
      QRegExp rx1("by=([^[,|\\]]+)");
      rx1.indexIn(fileStr);
      artist = latexToUtf8(rx1.cap(1));

      //title
      QRegExp rx2("beginsong\\{([^[\\}]+)");
      rx2.indexIn(fileStr);
      title = latexToUtf8(rx2.cap(1));

      //album
      QRegExp rx3(",album=([^[\\]]+)");
      rx3.indexIn(fileStr);
      album = latexToUtf8(rx3.cap(1));

      //lilypond
      QRegExp rx4("\\\\lilypond");
      lilypond = QBool(rx4.indexIn(fileStr) > -1);

      //lang
      QRegExp rx5("selectlanguage\\{([^[\\}]+)");
      rx5.indexIn(fileStr);
      lang = rx5.cap(1);
      
      //cover
      QRegExp rx6(",cov=([^[,]+)");
      rx6.indexIn(fileStr);
      coverName = rx6.cap(1);
      QString coverPath = path;
      //deletes "filename.sg" from string and keep the path
      coverPath.replace( QRegExp("\\/([^\\/]*).sg"), QString() ); 

      // Create the actual item
      QSqlRecord song;
      QSqlField f1("artist", QVariant::String);
      QSqlField f2("title", QVariant::String);
      QSqlField f3("lilypond", QVariant::Bool);
      QSqlField f4("path", QVariant::String);
      QSqlField f5("album", QVariant::String);
      QSqlField f6("cover", QVariant::String);
      QSqlField f7("lang", QVariant::String);
   
      f1.setValue(QVariant(artist));
      f2.setValue(QVariant(title));
      f3.setValue(QVariant(lilypond));
      f4.setValue(QVariant(path));
      f5.setValue(QVariant(album));
      f6.setValue(QVariant(QString("%1/%2.jpg").arg(coverPath).arg(coverName)));
      f7.setValue(QVariant(lang));
      
      song.append(f1);
      song.append(f2);
      song.append(f3);
      song.append(f4);
      song.append(f5);
      song.append(f6);
      song.append(f7);

      if(!insertRecord(-1,song))
	{
	  qDebug() << "\n artiste = " << artist;
	  qDebug() << "title = " << title;
	  qDebug() << "lilypond = " << lilypond;
	  qDebug() << "path = " << path;
	  qDebug() << "album = " << album;
	  qDebug() << "cover = " << QString("%1/%2.jpg").arg(coverPath).arg(coverName);
	  qDebug() << "lang = " << lang;
	  qWarning() << "CLibrary::addSongFromFile : unable to insert song " << path;
	}
    }
}
//------------------------------------------------------------------------------
void CLibrary::removeSong(const QString & path)
{
  //qDebug() << "CLibrary::removeSong " << path;
  QModelIndex index = createIndex(0,3); //column 3 stores the path
  QModelIndexList list = match(index, Qt::DisplayRole, path, -1, Qt::MatchExactly);

  foreach(index, list)
    removeRows(index.row(), 1);

  submitAll();
}
//------------------------------------------------------------------------------
void CLibrary::updateSong(const QString & path)
{
  //qDebug() << "CLibrary::updateSong " << path;
  removeSong(path);
  addSong(path);
  emit(wasModified());
}
//------------------------------------------------------------------------------
bool CLibrary::containsSong(const QString & path)
{
  //qDebug() << "CLibrary::containsSong " << path;
  //QModelIndex index = createIndex(0,3); //column 3 stores the path
  //QModelIndexList list = match(index, Qt::DisplayRole, path, -1, Qt::MatchExactly);
  //return !list.isEmpty();
  QSqlQuery query;
  query.exec(QString("SELECT artist FROM songs WHERE path = '%1'").arg(path));
  return query.next();
}
//------------------------------------------------------------------------------
QVariant CLibrary::data(const QModelIndex &index, int role) const
{
  QModelIndex parentIndex = index.parent();
  switch (role)
    {
    case TitleRole:
      return QSqlTableModel::data(sibling(index.row(), 1, parentIndex));
    case ArtistRole:
      return QSqlTableModel::data(sibling(index.row(), 0, parentIndex));
    case AlbumRole:
      return QSqlTableModel::data(sibling(index.row(), 4, parentIndex));
    case CoverRole:
      return QSqlTableModel::data(sibling(index.row(), 5, parentIndex));
    case LilypondRole:
      return QSqlTableModel::data(sibling(index.row(), 2, parentIndex));
    case LanguageRole:
      return QSqlTableModel::data(sibling(index.row(), 6, parentIndex));
    case PathRole:
      return QSqlTableModel::data(sibling(index.row(), 3, parentIndex));
    };

  //Draws lilypondcheck
  if ( index.column() == 2 )
    {
      if ( role == Qt::DisplayRole )
	return QString();

      if(QSqlTableModel::data( index, Qt::DisplayRole ).toBool())
	{
	  *m_pixmap = QPixmap(QIcon::fromTheme("audio-x-generic").pixmap(24,24));
	  if ( role == Qt::DecorationRole )
	    return *m_pixmap;
	  
	  if( m_pixmap->isNull() )
	    return true;

	  if ( role == Qt::SizeHintRole )
	    return m_pixmap->size();
	}
      return QString();
    }

  //Draws the cover
  if ( index.column() == 5 )
    {
      QString imgFile = QSqlTableModel::data( index, Qt::DisplayRole ).toString();
      if ( Qt::DisplayRole == role )
	return QString();

      *m_pixmap = QIcon::fromTheme("image-missing").pixmap(24,24);;
      if (!imgFile.isEmpty() && QFile::exists( imgFile ) && !QPixmapCache::find(imgFile, m_pixmap))
	{
	  *m_pixmap = QPixmap::fromImage(QImage(imgFile).scaledToWidth(24));
	  QPixmapCache::insert(imgFile, *m_pixmap);
	}

      if ( role == Qt::DecorationRole )
	return *m_pixmap;

      if ( role == Qt::SizeHintRole )
	return m_pixmap->size();
    }

  //Draws language flag
  if ( index.column() == 6 )
    {
      if ( role == Qt::DisplayRole )
      	return QString();

      QString lang = QSqlTableModel::data( index, Qt::DisplayRole ).toString();

      if ( role == Qt::ToolTipRole )
      	return lang;

      if(QPixmapCache::find(lang, m_pixmap))
	{
	  if ( role == Qt::DecorationRole )
	    return *m_pixmap;

	  if ( role == Qt::SizeHintRole )
	    return m_pixmap->size();
	}
    }
  return QSqlTableModel::data( index, role );
}
//------------------------------------------------------------------------------
void CLibrary::update()
{
  // recreate the database
  QSqlQuery query;
  query.exec("DROP TABLE songs");
  query.exec("CREATE TABLE songs ("
  	     "artist text,"
  	     "title text,"
  	     "lilypond bool,"
  	     "path text,"
  	     "album text,"
  	     "cover text,"
  	     "lang text"
  	     ")");

  QStringList filter = QStringList() << "*.sg";
  QString path = QString("%1/songs/").arg(workingPath());
  
  QDirIterator i(path, filter, QDir::NoFilter, QDirIterator::Subdirectories);
  uint count = 0;
  while(i.hasNext())
    {
      ++count;
      i.next();
    }
  parent()->progressBar()->show();
  parent()->progressBar()->setTextVisible(true);
  parent()->progressBar()->setRange(0, count);

  retrieveSongs();

  parent()->progressBar()->setTextVisible(false);
  parent()->progressBar()->hide();
  parent()->statusBar()->showMessage(tr("Song database updated."));
}

QDir CLibrary::directory() const
{
  return m_directory;
}

void CLibrary::setDirectory(const QString &directory)
{
  m_directory.setPath(directory);
}

void CLibrary::setDirectory(const QDir &directory)
{
  m_directory = directory;
}
