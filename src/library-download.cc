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

#include <QBoxLayout>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QStatusBar>

#include "file-chooser.hh"
#include "main-window.hh"
#include "library.hh"
#include "progress-bar.hh"

#include <QDebug>

CLibraryDownload::CLibraryDownload(CMainWindow *p)
  : QDialog(p)
  , m_manager()
  , m_reply(0)
  , m_url()
  , m_path()
{
  setWindowTitle(tr("Download"));

  m_manager = new QNetworkAccessManager(this);

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

  m_url = new QComboBox(this);
  m_url->setEditable(true);
  // set the default download URL to the songbook repository HEAD
  m_url->addItem("http://git.lohrun.net/?p=songbook.git;a=snapshot;h=HEAD;sf=tgz");
  m_url->addItem("http://www.patacrep.com/data/documents/songbook.tar.gz");

  m_path = new CFileChooser(this);
  m_path->setOptions(QFileDialog::ShowDirsOnly);
  m_path->setCaption(tr("Install directory"));

  QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
  buttonBox->addButton(tr("Download"),QDialogButtonBox::AcceptRole);
  buttonBox->addButton(QDialogButtonBox::Cancel);

  connect(buttonBox, SIGNAL(accepted()), this, SLOT(downloadStart()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));

  QVBoxLayout *vlayout = new QVBoxLayout;
  QFormLayout *layout = new QFormLayout;
  layout->addRow(tr("URL:"), m_url);
  layout->addRow(tr("Directory:"), m_path);
  vlayout->addLayout(layout);
  vlayout->addWidget(buttonBox);
  setLayout(vlayout);

  connect(parent()->progressBar(), SIGNAL(canceled()),
	  this, SLOT(cancelDownload()));
}

CLibraryDownload::~CLibraryDownload()
{
}

bool CLibraryDownload::saveToDisk(const QString &filename, QIODevice *data)
{
  QFile file(filename);
  if (!file.open(QIODevice::WriteOnly))
    {
      parent()->statusBar()->showMessage(tr("Could not open %1 to write: %2").arg(qPrintable(filename)).arg(qPrintable(file.errorString())));
      return false;
    }
  file.write(data->readAll());
  file.close();

  return true;
}

void CLibraryDownload::downloadStart()
{
  if (!m_url->currentText().isEmpty() && QUrl(m_url->currentText()).isValid())
    {
      // check if there already is a songbook directory in the specified path
      QDir dir = m_path->directory();
      QUrl url(m_url->currentText());
      QNetworkRequest request;
      request.setUrl(url);
      request.setRawHeader("User-Agent", "songbook-client a1");
      m_reply = m_manager->get(request);
      connect(m_reply, SIGNAL(finished()),
              this, SLOT(downloadFinished()));
      connect(m_reply, SIGNAL(sslErrors(QList<QSslError>)),
	      this, SLOT(sslErrors(QList<QSslError>)));
      connect(m_reply, SIGNAL(downloadProgress(qint64,qint64)),
	      this, SLOT(downloadProgress(qint64,qint64)));
      m_downloadTime.start();
      QDialog::accept();
    }
  else
    {
      qWarning() << tr("CLibraryDownload::downloadStart the following url is invalid: ") << m_url;
    }
}

void CLibraryDownload::downloadFinished()
{
  bool abort = false;

  if (m_reply->error())
    {
      parent()->statusBar()->showMessage(tr("Download of %1 failed: %2")
					 .arg(m_reply->url().toEncoded().constData())
					 .arg(qPrintable(m_reply->errorString())));
      abort = true;
    }

  QString filename = findFileName();

  if (QDir().exists(filename))
    {
      QMessageBox *warning
	= new QMessageBox(QMessageBox::Warning, tr("Existing file"),
			  tr("The following archive file already exists:\n %1 \n"
			     "Do you want to proceed anyway?").arg(filename),
			  QMessageBox::Yes | QMessageBox::No);
      if (warning->exec() == QMessageBox::No)
	{
	  parent()->statusBar()->showMessage(tr("Operation aborted"));
	  abort = true;
	}
    }

  if (!abort)
    {
      QDir dir = m_path->directory();
      QDir oldCurrent = QDir::currentPath();
      QString filepath = dir.filePath(filename);
      if (saveToDisk(filepath, m_reply))
	{
	  QDir::setCurrent(dir.absolutePath());
	  if (decompress(filepath, dir))
	    parent()->library()->setDirectory(dir);

	  QDir::setCurrent(oldCurrent.absolutePath());
	}
      // remove the downloaded archive after decompressing
      dir.remove(filepath);
      parent()->statusBar()->showMessage(tr("Download completed"));
    }

  parent()->progressBar()->hide();
  m_reply->deleteLater();
}

void CLibraryDownload::sslErrors(const QList<QSslError> &sslErrors)
{
#ifndef QT_NO_OPENSSL
  foreach (const QSslError &error, sslErrors)
    qWarning() << "CLibraryDownload::sslErrors : " << error.errorString();
#endif
}

void CLibraryDownload::cancelDownload()
{
  if (m_reply)
    m_reply->abort();
  downloadFinished();
}

// Uses the code sample proposed in the libarchive documentation
// http://code.google.com/p/libarchive/wiki/Examples#A_Complete_Extractor
bool CLibraryDownload::decompress(const QString &filename, QDir &directory)
{
  QDir dir;
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

QString CLibraryDownload::findFileName()
{
  if (!m_reply)
    {
      qWarning() << tr("CLibraryDownload::findFileName : invalid network reply");
      return QString();
    }

  QString filename = QFileInfo(m_reply->url().path()).fileName();
  if (filename.isEmpty())
    {
      // try to find a filename in the reply header
      QByteArray raw = m_reply->rawHeader(QByteArray("Content-Disposition"));
      QString rawHeader(raw);
      QRegExp re("filename=\"(.*)\"");
      re.indexIn(rawHeader);
      filename = re.cap(1);

      // fallback
      if (filename.isEmpty())
	filename = "songbook.tar.gz";
    }
  return filename;
}

QString CLibraryDownload::bytesToString(double value)
{
  QString unit;
  if (value < 1024)
    {
      unit = tr("bytes");
    }
  else if (value < 1024*1024)
    {
      value /= 1024;
      unit = tr("kB");
    }
  else
    {
      value /= 1024*1024;
      unit = tr("MB");
    }
  return tr("%1 %2").arg(value, 3, 'f', 1).arg(unit);
}

void CLibraryDownload::downloadProgress(qint64 bytesRead, qint64 totalBytes)
{
  QString message = tr("Downloading %1").arg(findFileName());
  // download transfer
  message.append(tr(" - %1").arg(bytesToString(bytesRead)));

  if (totalBytes > -1)
    {
      // download size
      message.append(tr(" of %1").arg(bytesToString(totalBytes)));

      //update the progress bar
      parent()->progressBar()->setRange(0, totalBytes);
      parent()->progressBar()->setValue(bytesRead);
    }

  // download speed
  message.append(tr(" (%2/s)").arg(bytesToString(bytesRead * 1000.0 / m_downloadTime.elapsed())));

  parent()->statusBar()->showMessage(message);
  parent()->progressBar()->show();
}
