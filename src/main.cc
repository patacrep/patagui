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
#include <QApplication>
#include <QTextCodec>
#include "mainwindow.hh"
//******************************************************************************
int main( int argc, char * argv[] )
{
  Q_INIT_RESOURCE(songbook);

  QCoreApplication::setOrganizationName("Patacrep");
  QCoreApplication::setOrganizationDomain("patacrep.com");
  QCoreApplication::setApplicationName("songbook");

  // Localization
  QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8")) ;
  QString locale = QLocale::system().name().section('_', 0, 0);
  QString filename = QString("songbook_%1").arg(locale) + ".qm";
  QString dir;
  
  const QDir systemDir("/usr/share/songbook-client/translations", "*.qm");
  const QDir userDir("/usr/local/share/songbook-client/translations", "*.qm");
  
  if (systemDir.entryList(QDir::Files | QDir::Readable).contains(filename))
    dir = systemDir.absolutePath();
  else if (userDir.entryList(QDir::Files | QDir::Readable).contains(filename))
    dir = userDir.absolutePath();
  else
    dir = "./lang";

  QTranslator translator;
  translator.load(QString("songbook_%1").arg(locale), dir);
  
  // Main application
  QApplication app(argc, argv);
  app.installTranslator(&translator);

  CMainWindow mainWindow;
  mainWindow.show();
  return app.exec();  
}
//******************************************************************************
