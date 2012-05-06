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

#ifndef __LIBRARY_DOWNLOAD_HH__
#define __LIBRARY_DOWNLOAD_HH__

#include <QDialog>
#include <QDir>
#include <QTime>

class QNetworkAccessManager;
class QNetworkReply;
class QComboBox;

class CFileChooser;
class CMainWindow;

/**
 * \file library-download.hh
 * \class CLibraryDownload
 * \brief CLibraryDownload is a dialog to download a songbook from a remote url.
 *
 * \image html download.png
 *
 * The remote url can be a git repository or a tar.gz archive.
 *
 */
class CLibraryDownload : public QDialog
{
  Q_OBJECT

public:
  /// Constructor.
  CLibraryDownload(CMainWindow *parent);

  /// Destructor.
  ~CLibraryDownload();

  /// Save some data as a new file.
  /// @param filename : filename of the new file on disk
  /// @param data : the data to be saved
  /// @return true if the operation succeeded, false otherwise
  bool saveToDisk(const QString &filename, QIODevice *data);

  /// Decompress an archive depending on libarchive library
  /// (http://github.com/libarchive/libarchive).
  /// @param filename : filename of the compressed archive
  /// @param directory : the directory resulting from the decompression
  /// @return true if the operation succeeded, false otherwise
  bool decompress(const QString &filename, QDir &directory);

public slots:
  /// Handles common errors and dialog at the end of the downloading operation
  /// such as conflicts with filenames or failed download.
  void downloadFinished();

  /// Network initialisation before download.
  void downloadStart();

  void downloadProgress(qint64 bytesRead, qint64 totalBytes);

private:
  CMainWindow * parent();
  QString bytesToString(double bytes);
  QString findFileName(QNetworkReply *reply);

  QNetworkAccessManager *m_manager;
  QComboBox *m_url;
  CFileChooser *m_path;
  QTime m_downloadTime;
};

#endif  // __LIBRARY_DOWNLOAD_HH_
