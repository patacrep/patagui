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
#include <QStandardItemModel>
#include <QString>
#include <QSqlTableModel>
class QTableView;
class CLibrary : public QSqlTableModel
{
  Q_OBJECT

public:
  CLibrary();
  ~CLibrary();
  
  QString pathToSongs();
  void setPathToSongs(const QString path);

public slots:
  void retrieveSongs();

public:
  void addSongFromFile(const QString path);
  void removeSongFromFile(const QString path);
  
  QString m_pathToSongs;
  QString imgFile;

public:
  static QString latexToUtf8(const QString str);  
  static QString processString(const QString str);
  QVariant data(const QModelIndex &index, int role) const;
};
