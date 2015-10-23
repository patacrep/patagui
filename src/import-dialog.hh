// Copyright (C) 2009-2013, Romain Goffe <romain.goffe@gmail.com>
// Copyright (C) 2009-2013, Alexandre Dupas <alexandre.dupas@gmail.com>
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

#ifndef __IMPORT_DIALOG_HH
#define __IMPORT_DIALOG_HH

#include <QDialog>
#include <QString>
#include <QUrl>
#include <QDir>

#include "config.hh"

#ifdef ENABLE_LIBRARY_DOWNLOAD
#include <QSslError>
#include <QTime>
#endif //ENABLE_LIBRARY_DOWNLOAD

class QLabel;
class QRadioButton;
class QPushButton;
class QAbstractButton;
class QListWidget;
class QListWidgetItem;
class QLineEdit;

class QNetworkAccessManager;
class QNetworkReply;

class ProgressBar;
class CFileChooser;
class CMainWindow;

/*!
  \file import-dialog.hh
  \class CImportDialog
  \brief CImportDialog is a dialog to import songs in the library.

  Songs may be imported from local (.sg) files or from a remote url.

  \image html import-dialog01.png
  \image html import-dialog02.png
*/
class CImportDialog : public QDialog
{
    Q_OBJECT
public:
    /// Constructor.
    CImportDialog(QWidget *parent = 0);

    /// Destructor.
    virtual ~CImportDialog();

    void readSettings();
    void writeSettings();

    ProgressBar* progressBar() const;
    void showMessage(const QString & message);

    CMainWindow* parent() const;
    void setParent(CMainWindow* parent);

#ifdef ENABLE_LIBRARY_DOWNLOAD
    void initDownload();

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
    bool decompress(const QString &filename);

    /// (http://github.com/libarchive/libarchive).
    int copy_data(struct archive *ar, struct archive *aw);

public slots:
    /// Handles common errors and dialog at the end of the downloading operation
    /// such as conflicts with filenames or failed download.
    void downloadFinished();

    void sslErrors(const QList<QSslError> &errors);

    /// Network initialisation before download.
    void downloadStart();

    void downloadProgress(qint64 bytesRead, qint64 totalBytes);

    void cancelDownload();
#endif //ENABLE_LIBRARY_DOWNLOAD

private slots:
    bool acceptDialog();
    void checkLibraryPath(const QString & path);
    void onRadioButtonClicked(QAbstractButton*);
    void onUrlChanged(const QString &);
    void openItem(QListWidgetItem*);
    void addFiles();
    void removeFiles();

signals:
    void songsReadyToBeImported(const QStringList &);

private:
    void setLocalSubWidgetsVisible(const bool value);
    void setNetworkSubWidgetsVisible(const bool value);

    CMainWindow *m_parent;
    CFileChooser *m_libraryPath;
    QLabel *m_libraryPathValid;

    QRadioButton *m_fromLocalButton;
    QRadioButton *m_fromNetworkButton;

    QPushButton *m_addButton;
    QPushButton *m_removeButton;

    QLabel *m_patacrepLabel;
    QLabel *m_gitLabel;
    QLabel *m_urlLabel;

    QRadioButton *m_patacrepButton;
    QRadioButton *m_gitButton;
    QRadioButton *m_urlButton;
    QLineEdit *m_urlLineEdit;
    QUrl m_url;

    QListWidget *m_fileList;
    QStringList m_songsToBeImported;

#ifdef ENABLE_LIBRARY_DOWNLOAD
    QString bytesToString(double bytes);
    QString findFileName();

    QNetworkAccessManager *m_manager;
    QNetworkReply *m_reply;
    QTime m_downloadTime;
#endif //ENABLE_LIBRARY_DOWNLOAD
};

#endif // __IMPORT_DIALOG_HH
