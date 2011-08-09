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

#include <QTranslator>
#include <QTextCodec>
#include <QDate>
#include <QLocale>
#include <QDir>

#include "main-window.hh"
#include "config.hh"

#ifdef USE_SPARKLE
#include "../macos_specific/sparkle/src/CocoaInitializer.h"
#include "../macos_specific/sparkle/src/SparkleAutoUpdater.h"
#endif

int main(int argc, char *argv[])
{
  //mac os, need to instanciate application fist to get it's path
  QApplication application(argc, argv);

  Q_INIT_RESOURCE(songbook);
  
  // this is the code needed to check for update on startup on mac os
  // we might plan to move it and also check for beta.
  #ifdef USE_SPARKLE
    AutoUpdater* updater;
    CocoaInitializer initializer;
    updater = new SparkleAutoUpdater("http://songbookclient.lmdb.eu/atom.xml");
    if (updater) {
        updater->checkForUpdates();
    }
  #endif

  static const char * GENERIC_ICON_TO_CHECK = "document-open";
  #ifdef __APPLE__
    static const char * FALLBACK_ICON_THEME = "macos";
  #else
    static const char * FALLBACK_ICON_THEME = "tango";
  #endif
  if (!QIcon::hasThemeIcon(GENERIC_ICON_TO_CHECK))
    {
      //If there is no default working icon theme then we should
      //use an icon theme that we provide via a .qrc file
      //This case happens under Windows and Mac OS X
      //This does not happen under GNOME or KDE
      QIcon::setThemeName(FALLBACK_ICON_THEME);
    }

  QCoreApplication::setOrganizationName("Patacrep");
  QCoreApplication::setOrganizationDomain("patacrep.com");
  QCoreApplication::setApplicationName(SONGBOOK_CLIENT_APPLICATION_NAME);
  QCoreApplication::setApplicationVersion(SONGBOOK_CLIENT_VERSION);
  
  // Localization
  QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8")) ;
  QDir translationDirectory;
  QString translationFilename = QString("songbook_%1.qm").arg(QLocale::system().name());
  QString directory;

#ifdef __APPLE__
  translationDirectory = application.applicationDirPath();
  translationDirectory.cd("../Resources/lang");
#else
  translationDirectory = QDir(SONGBOOK_CLIENT_DATA_PATH);
  translationDirectory.cd("translations");
#endif

  if (translationDirectory.exists())
    directory = translationDirectory.absolutePath();
  else
    directory = QDir::current().absoluteFilePath("lang");

  QTranslator translator;
  translator.load(translationFilename, directory);
  application.installTranslator(&translator);

  CMainWindow mainWindow;
  mainWindow.show();
  return application.exec();
}
