// Copyright (C) 2010 Romain Goffe, Alexandre Dupas
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
#ifndef DATABASE_HH
#define DATABASE_HH

#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <iostream>

static bool createConnection()
{
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
  db.setDatabaseName("patacrep");
  if (!db.open()) 
    {
      QMessageBox::critical(0, qApp->tr("Cannot open database"),
			    qApp->tr("Unable to establish a database connection.\n"
				     "This application needs SQLite support. "
				     "Click Cancel to exit."), QMessageBox::Cancel);
      return false;
    }
  
  QSqlQuery query;
  query.exec("create table songs ( artist char(80), "
	     "title char(80), "
	     "lilypond bool, "
	     "path char(80), "
	     "album char(80), "
	     "cover char(80))");
  
  return true;
}

#endif
