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

/*!
 * \file main.cc
 * \mainpage Songbook-Client Documentation
 *
 * A songbook is a collection of lyrics/guitar chords for songs.
 * This application allows one to manage your own set of songs and produce
 * a beautiful pdf as a result.
 *
 * \image html application.png
 *
 *
 */

#include <QApplication>

#include <QTranslator>
#include <QTextCodec>
#include <QDate>
#include <QLocale>
#include <QDir>
#include <QTextStream>

#include "main-window.hh"
#include "config.hh"

#ifdef USE_SPARKLE
#include "../macos_specific/sparkle/src/CocoaInitializer.h"
#include "../macos_specific/sparkle/src/SparkleAutoUpdater.h"
#endif

/// Main routine of the application
int main(int argc, char *argv[])
{
  // MacOSX needs to instanciate the application first to get the path
  QApplication application(argc, argv);

  QApplication::setOrganizationName("Patacrep");
  QApplication::setOrganizationDomain("patacrep.com");
  QApplication::setApplicationName(SONGBOOK_CLIENT_APPLICATION_NAME);
  QApplication::setApplicationVersion(SONGBOOK_CLIENT_VERSION);

  // Load the application ressources (icons, ...)
  Q_INIT_RESOURCE(songbook);

  // Check for a standard theme icon. If it does not exist, for
  // instance on MacOSX or Windows, fallback to one of the theme
  // provided in the ressource file.
  if (!QIcon::hasThemeIcon("document-open"))
    {
#ifdef __APPLE__
      QIcon::setThemeName("macos");
#else // __APPLE__
      QIcon::setThemeName("tango");
#endif // __APPLE__
    }

  // Parse command line arguments
  QStringList arguments = QApplication::arguments();
  bool helpFlag = false;;
  bool versionFlag = false;
  if (arguments.contains("-h") || arguments.contains("--help"))
    helpFlag = true;
  else if (arguments.contains("--version"))
    versionFlag = true;

  // Localization
  QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8")) ;
  QDir translationDirectory;
  QString translationFilename = QString("songbook_%1.qm").arg(QLocale::system().name().split('_').first());
  QString directory;

#ifdef __APPLE__
  translationDirectory = application.applicationDirPath();
  translationDirectory.cd("../Resources");
#else
  translationDirectory = QDir(SONGBOOK_CLIENT_DATA_PATH);
#endif

  if (translationDirectory.exists())
    directory = translationDirectory.absoluteFilePath("lang");
  else
    directory = QDir::current().absoluteFilePath("lang");

  QTranslator translator;
  translator.load(translationFilename, directory);
  application.installTranslator(&translator);

#ifdef USE_SPARKLE
  // Check for updates on startup using MacOSX. The atom feed provides
  // the list of releases to get the update from.
  // TODO: add a check to ignore beta versions
  CocoaInitializer initializer;
  AutoUpdater *updater = new SparkleAutoUpdater("http://songbookclient.lmdb.eu/atom.xml");
  if (updater)
    updater->checkForUpdates();
#endif // USE_SPARKLE

  if (helpFlag)
    {
      QTextStream out(stdout);
      out << "Usage: " << QApplication::applicationName() << "[OPTION]" << endl
	  << "Options:" << endl
	  << "    " << "-h, --help"
	  << "    " << "--version"
	  << " " << QApplication::applicationVersion()
	  << endl;
      return 0;
    }
  else if (versionFlag)
    {
      QTextStream out(stdout);
      out << QApplication::applicationName()
	  << " " << QApplication::applicationVersion()
	  << endl;
      return 0;
    }

  CMainWindow mainWindow;
  mainWindow.show();
  return application.exec();
}
