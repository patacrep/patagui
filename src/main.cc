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
#include <QApplication>
#include <QTextCodec>
#include "mainwindow.hh"
//******************************************************************************
int main( int argc, char * argv[] )
{
  //mac os, need to instanciate aplpication fist to get it's path
  QApplication app(argc, argv);

  Q_INIT_RESOURCE(songbook);

  static const char * GENERIC_ICON_TO_CHECK = "document-open";
  static const char * FALLBACK_ICON_THEME = "tango";
  if (!QIcon::hasThemeIcon(GENERIC_ICON_TO_CHECK))
    {
      //If there is no default working icon theme then we should
      //use an icon theme that we provide via a .qrc file
      //This case happens under Windows and Mac OS X
      //This does not happen under GNOME or KDE
      QIcon::setThemeName(FALLBACK_ICON_THEME);
    }

  QString version = QString("0.5 (%1)")
    .arg(QDate::currentDate().toString(Qt::SystemLocaleLongDate));
  QCoreApplication::setOrganizationName("Patacrep");
  QCoreApplication::setOrganizationDomain("patacrep.com");
  QCoreApplication::setApplicationName("songbook-client");
  QCoreApplication::setApplicationVersion(version);
  
  // Localization
  QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8")) ;
  QString locale = QLocale::system().name().section('_', 0, 0);
  QString filename = QString("songbook_%1").arg(locale) + ".qm";
  QString dir;

#ifdef __APPLE__
  QDir cdir(app.applicationDirPath());
  cdir.cdUp();
  cdir.cd("Resources/lang");
  const QDir systemDir(cdir.absolutePath());
  const QDir userDir(cdir.absolutePath());
#else
  const QDir systemDir("/usr/share/songbook-client/translations", "*.qm");
  const QDir userDir("/usr/local/share/songbook-client/translations", "*.qm");
#endif

  if (systemDir.entryList(QDir::Files | QDir::Readable).contains(filename))
    dir = systemDir.absolutePath();
  else if (userDir.entryList(QDir::Files | QDir::Readable).contains(filename))
    dir = userDir.absolutePath();
  else
    dir = QString("%1%2lang").arg(QDir::currentPath()).arg(QDir::separator());

  QTranslator translator;
  translator.load(QString("songbook_%1").arg(locale), dir);

  // Main application
  // move app creation to beggining
  app.installTranslator(&translator);

  CMainWindow mainWindow;
  mainWindow.show();
  return app.exec();
}
//******************************************************************************
