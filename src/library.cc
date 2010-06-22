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

#include <QList>
#include <QStringList>
#include <QTextStream>
#include <QDir>
#include <QDirIterator>
#include <QRegExp>
#include <QPixmap>
#include <QIcon>
#include <QTableView>
#include <QSqlRecord>
#include <QSqlTableModel>
#include <QSqlField>
#include <QPixmapCache>
#include <QDebug>
//------------------------------------------------------------------------------
CLibrary::CLibrary()
  : QSqlTableModel()
  , m_pathToSongs()
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
}
//------------------------------------------------------------------------------
CLibrary::~CLibrary()
{}
//------------------------------------------------------------------------------
QString CLibrary::pathToSongs()
{
  return m_pathToSongs;
}
//------------------------------------------------------------------------------
void CLibrary::setPathToSongs(const QString path)
{
  m_pathToSongs = path;
}
//------------------------------------------------------------------------------
void CLibrary::retrieveSongs()
{
  QStringList filter;
  filter << "*.sg";

  QString path = QString("%1/songs/").arg(pathToSongs());
  QDirIterator it(path, filter, QDir::NoFilter, QDirIterator::Subdirectories);

  while(it.hasNext())
    addSongFromFile(it.next());

  submitAll();
}
//------------------------------------------------------------------------------
void CLibrary::addSongFromFile(const QString path)
{
  QFile file(path);
  if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      QTextStream stream (&file);
      QString fileStr = stream.readAll();
      file.close();

      QString res;
      QString artist;
      QString title;
      bool lilypond(false);
      QString album;
      QString coverName;

      //artist
      QRegExp rx1("by=([^[,|\\]]+)");
      rx1.indexIn(fileStr);
      res = rx1.cap(1);
      artist = CLibrary::latexToUtf8(res);

      //title
      QRegExp rx2("beginsong\\{([^[\\}]+)");
      rx2.indexIn(fileStr);
      res = rx2.cap(1);
      title = CLibrary::latexToUtf8(res);

      //album
      QRegExp rx3(",cov=([^[\\]]+)");
      rx3.indexIn(fileStr);
      res = rx3.cap(1);
      coverName = res;
      album = CLibrary::processString(res);

      //lilypond
      QRegExp rx4("\\\\lilypond");
      lilypond = QBool(rx4.indexIn(fileStr) > -1);

      //cover
      QString coverPath = path;
      coverPath.replace( QRegExp("\\/([^\\/]*).sg"), QString() ); //deletes file.sg and keep the path

      // Create the actual item
      QSqlRecord song;
      QSqlField f1("artist", QVariant::String);
      QSqlField f2("title", QVariant::String);
      QSqlField f3("lilypond", QVariant::Bool);
      QSqlField f4("path", QVariant::String);
      QSqlField f5("album", QVariant::String);
      QSqlField f6("cover", QVariant::String);

      f1.setValue(QVariant(artist));
      f2.setValue(QVariant(title));
      f3.setValue(QVariant(lilypond));
      f4.setValue(QVariant(path));
      f5.setValue(QVariant(album));
      f6.setValue(QVariant(QString("%1/%2.jpg").arg(coverPath).arg(coverName)));

      song.append(f1);
      song.append(f2);
      song.append(f3);
      song.append(f4);
      song.append(f5);
      song.append(f6);

      insertRecord(-1,song);
    }
}
//------------------------------------------------------------------------------
QString CLibrary::latexToUtf8(const QString str)
{
  QString AString(str);
  AString.replace(QString("\\'e"), QString("é"));
  AString.replace(QString("\\`e"), QString("è"));
  AString.replace(QString("\\^e"), QString("ê"));
  AString.replace(QString("\\¨e"), QString("ë"));
  AString.replace(QString("\\¨i"), QString("ï"));
  AString.replace(QString("\\^i"), QString("î"));
  AString.replace(QString("\\^o"), QString("ô"));
  AString.replace(QString("\\`u"), QString("ù"));
  AString.replace(QString("\\`a"), QString("à"));
  AString.replace(QString("\\^a"), QString("â"));
  AString.replace(QString("\\&"), QString("&"));
  AString.replace(QString("\\~"), QString("~"));
  AString.replace(QString("~"), QString(" "));
  AString.replace(QString("\\dots"), QString("..."));

  return AString;
}
//------------------------------------------------------------------------------
QString CLibrary::processString(const QString str)
{
  QString AString(str);
  if (AString.isEmpty())
    return AString;

  //Set the first letter to uppercase
  AString[0] = AString[0].toUpper();
  AString.replace(QString("_"), QString(" "));
  AString.replace(QString("-"), QString(" "));
  return AString;
}
//------------------------------------------------------------------------------
QVariant CLibrary::data(const QModelIndex &index, int role) const
{

  //Draws lilypondcheck
  if ( index.column() == 2 )
    {
      if ( Qt::DisplayRole == role )
	return QString();

      QPixmap pixmap;
      if(QSqlTableModel::data( index, Qt::DisplayRole ).toBool())
	pixmap = QIcon::fromTheme("audio-x-generic").pixmap(24,24);
      
      if ( role == Qt::DecorationRole )
	return pixmap;

      if (role == Qt::SizeHintRole)
	return pixmap.size();
    }

  //Draws the cover
  if ( index.column() == 5 )
    {
      QString imgFile = QSqlTableModel::data( index, Qt::DisplayRole ).toString();
      if ( Qt::DisplayRole == role )
	return QString();

      QPixmap pixmap = QIcon::fromTheme("image-missing").pixmap(24,24);;
#if QT_VERSION >= 0x040600
      if (!imgFile.isEmpty() && QFile::exists( imgFile ) && !QPixmapCache::find(imgFile, &pixmap))
#else
      if (!imgFile.isEmpty() && QFile::exists( imgFile ) && !QPixmapCache::find(imgFile, pixmap))
#endif
	{
	  pixmap = QPixmap::fromImage(QImage(imgFile).scaledToWidth(24));
	  QPixmapCache::insert(imgFile, pixmap);
	}
           
      if ( role == Qt::DecorationRole )
	return pixmap;

      if (role == Qt::SizeHintRole)
	return pixmap.size();
    }
  return QSqlTableModel::data( index, role );
}

