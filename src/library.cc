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
#include <QtGui>
#include <QtSql>

#include "library.hh"
#include "mainwindow.hh"
#include "utils/utils.hh"
using namespace SbUtils;
//------------------------------------------------------------------------------
CLibrary::CLibrary(CMainWindow* AParent)
  : QSqlTableModel()
{
  m_parent = AParent;
  m_workingPath = parent()->workingPath();
  connect(parent(), SIGNAL(workingPathChanged(QString)),
	  this, SLOT(setWorkingPath(QString)));
  
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
CMainWindow* CLibrary::parent()
{
  return m_parent ;
}
//------------------------------------------------------------------------------
void CLibrary::retrieveSongs()
{
  uint count = 0;
  QStringList filter = QStringList() << "*.sg";
  QString path = QString("%1/songs/").arg(workingPath());
  
  QDirIterator it(path, filter, QDir::NoFilter, QDirIterator::Subdirectories);
  while(it.hasNext())
    {
      parent()->statusBar()->showMessage(QString(tr("Found song : %1")).arg(it.fileInfo().fileName()));      
      parent()->progressBar()->setValue(++count);
      addSongFromFile(it.next());
    }
  submitAll();
}
//------------------------------------------------------------------------------
void CLibrary::addSongFromFile(const QString path)
{
  //do not insert if the song is already in the library
  QSqlQuery query;
  query.exec(QString("SELECT artist FROM songs WHERE path = '%1'").arg(path));
  if (query.next())
    return;

  QFile file(path);
  if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      QTextStream stream (&file);
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
      album = rx3.cap(1);

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

      insertRecord(-1,song);
    }
}
//------------------------------------------------------------------------------
QVariant CLibrary::data(const QModelIndex &index, int role) const
{
  //Draws lilypondcheck
  if ( index.column() == 2 )
    {
      if ( role == Qt::DisplayRole )
	return QString();

      if(QSqlTableModel::data( index, Qt::DisplayRole ).toBool())
	{
#if QT_VERSION >= 0x040600
	  *m_pixmap = QPixmap(QIcon::fromTheme("audio-x-generic").pixmap(24,24));
#endif
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

#if QT_VERSION >= 0x040600
      *m_pixmap = QIcon::fromTheme("image-missing").pixmap(24,24);;
#endif

#if QT_VERSION >= 0x040600
      if (!imgFile.isEmpty() && QFile::exists( imgFile ) && !QPixmapCache::find(imgFile, m_pixmap))
#else
      if (!imgFile.isEmpty() && QFile::exists( imgFile ) && !QPixmapCache::find(imgFile, *m_pixmap))
#endif
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

#if QT_VERSION >= 0x040600
      if(QPixmapCache::find(lang, m_pixmap))
#else
      if(QPixmapCache::find(lang, *m_pixmap))
#endif
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
QString CLibrary::workingPath() const
{
  return m_workingPath;
}
//------------------------------------------------------------------------------
void CLibrary::setWorkingPath(QString value)
{
  m_workingPath = value;
}
//------------------------------------------------------------------------------
