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

/**
 * \file library-download.hh
 */

#ifndef __LIBRARY_DOWNLOAD_HH__
#define __LIBRARY_DOWNLOAD_HH__

#include <QDialog>
#include <QDir>

class QNetworkAccessManager;
class QNetworkReply;
class QLineEdit;

class CFileChooser;
class CMainWindow;

/** \class CLibraryDownload "library-download.hh"
 * \brief CLibraryDownload is a class.
 */

class CLibraryDownload : public QDialog
{
  Q_OBJECT

public:
  CLibraryDownload(CMainWindow *parent);
  ~CLibraryDownload();

  bool saveToDisk(const QString &filename, QIODevice *data);
<<<<<<< .merge_file_ydZmZa
  bool decompress(const QString &filename, QDir &directory);
=======
  QDir decompress(const QString &filename);
>>>>>>> .merge_file_IKy5q7

public slots:
  void downloadFinished();
  void downloadStart();

private:
  CMainWindow * parent();

  QNetworkAccessManager *m_manager;
  QLineEdit *m_url;
  CFileChooser *m_path;
};

#endif  // __LIBRARY_DOWNLOAD_HH_
