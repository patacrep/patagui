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
#include <iostream>
#include <QApplication>
#include <QTextCodec>
#include <QtSql>
#include "mainwindow.hh"
using namespace std;
//******************************************************************************
int main( int argc, char * argv[] )
{
  Q_INIT_RESOURCE(application);

  QCoreApplication::setOrganizationName("Patacrep");
  QCoreApplication::setOrganizationDomain("patacrep.com");
  QCoreApplication::setApplicationName("songbook");

  QApplication app(argc, argv);

  //Localization
  QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8")) ;
  QString locale = QLocale::system().name().section('_', 0, 0);
  QTranslator translator;
  translator.load(QString("songbook_%1").arg(locale), "./translations");
  app.installTranslator(&translator);
  
  CMainWindow mainWindow;
  mainWindow.show();
  int res = app.exec();

  //close db connection
  QSqlDatabase db = QSqlDatabase::database();
  db.close();
  QSqlDatabase::removeDatabase(db.connectionName());
  
  return res;
}
//******************************************************************************
