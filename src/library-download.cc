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

#include "library-download.hh"

#include <archive.h>
#include <archive_entry.h>

#include <QNetworkProxy>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFileInfo>
#include <QFile>
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QProgressBar>

#include "file-chooser.hh"
#include "mainwindow.hh"

#include <QDebug>
#include <QString>

CLibraryDownload::CLibraryDownload(CMainWindow *parent)
  : QDialog(parent)
  , m_manager()
  , m_url()
  , m_path()
{
  setWindowTitle(tr("Download songs library"));

  m_manager = new QNetworkAccessManager;

  {
    QSettings settings;
    settings.beginGroup("proxy");
    QString hostname = settings.value("hostname", QString()).toString();
    QString port = settings.value("port", QString()).toString();
    QString user = settings.value("user", QString()).toString();
    QString password = settings.value("password", QString()).toString();
    settings.endGroup();

    QNetworkProxy proxy;
    if (hostname.isEmpty())
      {
	proxy.setType(QNetworkProxy::NoProxy);
      }
    else
      {
	proxy.setType(QNetworkProxy::HttpProxy);
	proxy.setHostName(hostname);
	proxy.setPort(port.toInt());
	proxy.setUser(user);
	proxy.setPassword(password);
      }
    QNetworkProxy::setApplicationProxy(proxy);
  }

  m_url = new QLineEdit();
  // set the default download URL to the songbook repository HEAD
  m_url->setText("http://git.lohrun.net/?p=songbook.git;a=snapshot;h=HEAD;sf=tgz");

  m_path = new CFileChooser();
  m_path->setType(CFileChooser::DirectoryChooser);
  m_path->setCaption(tr("Install directroy"));
  m_path->setPath(QDir::homePath());

  QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
  buttonBox->addButton(tr("Download"),QDialogButtonBox::AcceptRole);
  buttonBox->addButton(QDialogButtonBox::Cancel);

  connect(buttonBox, SIGNAL(accepted()), this, SLOT(downloadStart()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));

  QVBoxLayout *vlayout = new QVBoxLayout();
  QFormLayout *layout = new QFormLayout();
  layout->addRow(tr("URL:"), m_url);
  layout->addRow(tr("Directory:"), m_path);
  vlayout->addLayout(layout);
  vlayout->addWidget(buttonBox);
  setLayout(vlayout);
}

CLibraryDownload::~CLibraryDownload()
{}

bool CLibraryDownload::saveToDisk(const QString &filename, QIODevice *data)
{
  QFile file(filename);
  if (!file.open(QIODevice::WriteOnly))
    {
      parent()->statusBar()->showMessage(tr("Could not open %1 for writting: %2").arg(qPrintable(filename)).arg(qPrintable(file.errorString())));
      return false;
    }
  file.write(data->readAll());
  file.close();

  return true;
}

void CLibraryDownload::downloadStart()
{
  if (!m_url->text().isEmpty())
    {
      // check if there already is a songbook directory in the specified path
      QDir dir = m_path->directory();
      QUrl url(m_url->text());
      QNetworkRequest request;
      request.setUrl(url);
      request.setRawHeader("User-Agent", "songbook-client a1");
      QNetworkReply *reply = m_manager->get(request);
      connect(reply, SIGNAL(finished()),
              this, SLOT(downloadFinished()));
      parent()->progressBar()->show();
      QDialog::accept();
    }
}

void CLibraryDownload::downloadFinished()
{
  bool abort = false;

  QNetworkReply *reply = qobject_cast< QNetworkReply* >(sender());
  QUrl url = reply->url();

  if (reply->error())
    {
      parent()->statusBar()->showMessage(tr("Download of %1 failed: %2").arg(url.toEncoded().constData()).arg(qPrintable(reply->errorString())));
      abort = true;
    }

  QString filename = QFileInfo(url.path()).fileName();
  if (filename.isEmpty())
    {
      // try to find a filename in the reply header
      QByteArray raw = reply->rawHeader(QByteArray("Content-Disposition"));
      QString rawHeader(raw);
      QRegExp re("filename=\"(.*)\"");
      re.indexIn(rawHeader);
      filename = re.cap(1);

      // falback
      if (filename.isEmpty())
	filename = QString("songbook.tar.gz");
    }

  if (QDir().exists(filename))
    {
      QMessageBox *warning
	= new QMessageBox(QMessageBox::Warning,
			  tr("Existing file"),
			  QString(tr("The following archive file already exists:\n %1 \n"
				     "Do you want to proceed anyway?")).arg(filename),
			  QMessageBox::Yes | QMessageBox::No);
      if (warning->exec() == QMessageBox::No)
	{
	  parent()->statusBar()->showMessage(tr("Operation aborted"));
	  abort = true;
	}
    }

  if (!abort)
    {
      QDir libraryDir;
      QDir dir = m_path->directory();
      QDir oldCurrent = QDir::currentPath();
      QString filepath = dir.filePath(filename);
      if (saveToDisk(filepath, reply))
	{
	  QDir::setCurrent(dir.absolutePath());
	  if (decompress(filepath, libraryDir))
	    parent()->setWorkingPath(libraryDir.absolutePath());
	  QDir::setCurrent(oldCurrent.absolutePath());
	}
      // remove the downloaded archive after decompressing
      dir.remove(filename);
    }

  parent()->progressBar()->hide();
  reply->deleteLater();
}

// Uses the code sample proposed in the libarchive documentation
// http://code.google.com/p/libarchive/wiki/Examples#A_Complete_Extractor
bool CLibraryDownload::decompress(const QString &filename, QDir &directory)
{
  struct archive *archive;
  struct archive_entry *entry;
  int flags;

  /* Select which attributes we want to restore. */
  flags = ARCHIVE_EXTRACT_TIME;
  flags |= ARCHIVE_EXTRACT_PERM;
  flags |= ARCHIVE_EXTRACT_ACL;
  flags |= ARCHIVE_EXTRACT_FFLAGS;

  archive = archive_read_new();
  archive_read_support_format_all(archive);
  archive_read_support_compression_all(archive);

  if (archive_read_open_filename(archive, qPrintable( filename ), 10240))
    {
      parent()->statusBar()->showMessage(tr("CLibraryDownload::decompress: unable to open the archive"));
      return false;
    }

  bool first = true;
  while (archive_read_next_header(archive, &entry) == ARCHIVE_OK)
    {
      // update the directory
      if (first)
	{
	  first = false;
	  // the first entry is supposed to be the main directory
	  directory = QDir().absoluteFilePath(archive_entry_pathname(entry));
	  if (directory.exists())
	    {
	      QMessageBox *warning = new QMessageBox(QMessageBox::Warning,
						     tr("Existing directory"),
						     tr("The song library directory from the archive already exists in the selected install directory. Do you want to proceed anyway?"),
						     QMessageBox::Yes | QMessageBox::No,
						     this);
	      if (warning->exec() == QMessageBox::No)
		{
		  parent()->statusBar()->showMessage(tr("Operation aborted"));
		  return false;
		}
	    }
	}
      archive_read_extract(archive, entry, flags);
    }
  archive_read_finish(archive);
  return true;
}

CMainWindow * CLibraryDownload::parent()
{
  return qobject_cast< CMainWindow* >(QDialog::parent());
}
