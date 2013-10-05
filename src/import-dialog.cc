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

#include "import-dialog.hh"

#include "file-chooser.hh"
#include "main-window.hh"
#include "library.hh"
#include "progress-bar.hh"

#include <QDir>
#include <QLabel>
#include <QGroupBox>
#include <QFormLayout>
#include <QBoxLayout>
#include <QSettings>
#include <QButtonGroup>
#include <QRadioButton>
#include <QPushButton>
#include <QGridLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QStatusBar>
#include <QDesktopServices>

#include <QElapsedTimer>
#include <QDebug>

#include "config.hh"

#ifdef ENABLE_LIBRARY_DOWNLOAD
#include <archive.h>
#include <archive_entry.h>

#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QNetworkReply>
#include <QNetworkRequest>
#endif //ENABLE_LIBRARY_DOWNLOAD

CImportDialog::CImportDialog(QWidget *parent)
  : QDialog(parent)
  , m_libraryPath(new CFileChooser(this))
  , m_libraryPathValid(new QLabel(this))
  , m_songsToBeImported(QStringList())
{
  setWindowTitle(tr("Import songs"));
  setParent(static_cast<CMainWindow*>(parent));

  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(buttonBox, SIGNAL(accepted()), SLOT(acceptDialog()));
  connect(buttonBox, SIGNAL(rejected()), SLOT(close()));

  m_libraryPath->setMinimumWidth(300);
  m_libraryPath->setOptions(QFileDialog::ShowDirsOnly);
  m_libraryPath->setCaption(tr("Library path"));
  connect(m_libraryPath, SIGNAL(pathChanged(const QString&)),
          this, SLOT(checkLibraryPath(const QString&)));
  checkLibraryPath(m_libraryPath->path());

  QGroupBox *pathGroupBox = new QGroupBox(tr("Library"));

  QFormLayout *pathLayout = new QFormLayout;
  pathLayout->addRow(tr("Path:"), m_libraryPath);
  pathLayout->addRow(m_libraryPathValid);
  pathGroupBox->setLayout(pathLayout);

  //Import Section
  QGroupBox *importGroupBox = new QGroupBox(tr("Import new Songs"));

  QButtonGroup *buttonGroup = new QButtonGroup;
  m_fromLocalButton = new QRadioButton(tr("From local files"));
  buttonGroup->addButton(m_fromLocalButton);
  m_fromNetworkButton = new QRadioButton(tr("From network"));
  buttonGroup->addButton(m_fromNetworkButton);
  connect(buttonGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(onRadioButtonClicked(QAbstractButton*)));

  QLabel *fromLocalLabel = new QLabel;
  fromLocalLabel->setPixmap(QIcon::fromTheme("document-open", QIcon(":/icons/tango/48x48/actions/document-open.png")).pixmap(48, 48));
  QLabel *fromNetworkLabel = new QLabel;
  fromNetworkLabel->setPixmap(QIcon(":/icons/songbook/48x48/applications-internet.png").pixmap(48, 48));

  //from local files
  m_fileList = new QListWidget;
  m_fileList->setSelectionMode(QAbstractItemView::MultiSelection);
  connect(m_fileList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(openItem(QListWidgetItem*)));

  m_addButton = new QPushButton(QIcon::fromTheme("list-add", QIcon(":/icons/tango/32x32/actions/list-add.png")), tr("Add files"));
  connect(m_addButton, SIGNAL(clicked()), this, SLOT(addFiles()));
  m_removeButton = new QPushButton(QIcon::fromTheme("list-remove", QIcon(":/icons/tango/32x32/actions/list-remove.png")), tr("Remove files"));
  connect(m_removeButton, SIGNAL(clicked()), this, SLOT(removeFiles()));

  QBoxLayout *fileActionsLayout = new QVBoxLayout;
  fileActionsLayout->addStretch();
  fileActionsLayout->addWidget(m_addButton);
  fileActionsLayout->addWidget(m_removeButton);

  //from network
  QButtonGroup *networkGroup = new QButtonGroup;
  m_patacrepButton = new QRadioButton(tr("Latest stable version"));
  networkGroup->addButton(m_patacrepButton);
  m_gitButton = new QRadioButton(tr("Latest development version"));
  networkGroup->addButton(m_gitButton);
  m_urlButton = new QRadioButton(tr("Custom URL"));
  networkGroup->addButton(m_urlButton);
  connect(networkGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(onRadioButtonClicked(QAbstractButton*)));
  
  m_patacrepLabel = new QLabel;
  m_patacrepLabel->setPixmap(QIcon(":/icons/songbook/32x32/songbook-client.png").pixmap(32, 32));
  m_gitLabel = new QLabel;
  m_gitLabel->setPixmap(QIcon(":/icons/songbook/32x32/git.png").pixmap(32, 32));
  m_urlLabel = new QLabel;
  m_urlLabel->setPixmap(QIcon(":/icons/songbook/32x32/applications-internet.png").pixmap(32, 32));

  m_urlLineEdit = new QLineEdit;
  connect(m_urlLineEdit, SIGNAL(textChanged(const QString &)), SLOT(onUrlChanged(const QString &)));

#ifndef ENABLE_LIBRARY_DOWNLOAD
  m_fromNetworkButton->setEnabled(false);
  m_patacrepButton->setEnabled(false);
  m_gitButton->setEnabled(false);
  m_urlButton->setEnabled(false);
#endif //ENABLE_LIBRARY_DOWNLOAD

  QGridLayout *importLayout = new QGridLayout;
  importLayout->addWidget(fromLocalLabel, 0, 0, 1, 1);
  importLayout->addWidget(m_fromLocalButton, 0, 1, 1, 4);
  importLayout->addWidget(m_fileList, 1, 2, 1, 1);
  importLayout->addLayout(fileActionsLayout, 1, 4, 1, 1);
  importLayout->addWidget(fromNetworkLabel, 2, 0, 1, 1);
  importLayout->addWidget(m_fromNetworkButton, 2, 1, 1, 4);
  importLayout->addWidget(m_patacrepLabel, 3, 1, 1, 1);
  importLayout->addWidget(m_patacrepButton, 3, 2, 1, 3);
  importLayout->addWidget(m_gitLabel, 4, 1, 1, 1);
  importLayout->addWidget(m_gitButton, 4, 2, 1, 3);
  importLayout->addWidget(m_urlLabel, 5, 1, 1, 1);
  importLayout->addWidget(m_urlButton, 5, 2, 1, 2);
  importLayout->addWidget(m_urlLineEdit, 5, 4, 1, 1);

  importGroupBox->setLayout(importLayout);

  setLocalSubWidgetsVisible(false);
  setNetworkSubWidgetsVisible(false);

  // main layout
  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(pathGroupBox);
  mainLayout->addWidget(importGroupBox);
  mainLayout->addStretch(1);
  mainLayout->addWidget(buttonBox);
  setLayout(mainLayout);

  setMinimumWidth(600);

  readSettings();
}

CImportDialog::~CImportDialog()
{
  delete m_libraryPath;
  delete m_libraryPathValid;

  delete m_fromLocalButton;
  delete m_fromNetworkButton;

  delete m_addButton;
  delete m_removeButton;

  delete m_patacrepLabel;
  delete m_gitLabel;
  delete m_urlLabel;

  delete m_patacrepButton;
  delete m_gitButton;
  delete m_urlButton;
  delete m_urlLineEdit;
  
  delete m_fileList;

  delete m_manager;
}

void CImportDialog::readSettings()
{
  QSettings settings;
  settings.beginGroup("general");
  m_libraryPath->setPath(settings.value("libraryPath", QDir::homePath()).toString());
  settings.endGroup();

  settings.beginGroup("import");
  m_urlLineEdit->setText(settings.value("customUrl", "http://").toString());
  settings.endGroup();
  m_patacrepButton->setChecked(true);
}

void CImportDialog::writeSettings()
{
  QSettings settings;
  settings.beginGroup("general");
  settings.setValue("libraryPath", m_libraryPath->path());
  settings.endGroup();

  settings.beginGroup("import");
  settings.setValue("customUrl", m_urlLineEdit->text());
  settings.endGroup();
}


CMainWindow* CImportDialog::parent() const
{
  return m_parent;
}

void CImportDialog::setParent(CMainWindow* parent)
{
  m_parent = parent;
}

void CImportDialog::checkLibraryPath(const QString & path)
{
  m_libraryPathValid->setText(CLibrary::checkPath(path));
}

void CImportDialog::setLocalSubWidgetsVisible(const bool value)
{
  m_addButton->setVisible(value);
  m_removeButton->setVisible(value);
  m_fileList->setVisible(value);
}

void CImportDialog::setNetworkSubWidgetsVisible(const bool value)
{
  m_patacrepLabel->setVisible(value);
  m_gitLabel->setVisible(value);
  m_urlLabel->setVisible(value);

  m_patacrepButton->setVisible(value);
  m_gitButton->setVisible(value);
  m_urlButton->setVisible(value);
  m_urlLineEdit->setVisible(value);
}

void CImportDialog::onRadioButtonClicked(QAbstractButton* button)
{
  if (button == m_fromLocalButton)
    {
      setLocalSubWidgetsVisible(true);
      setNetworkSubWidgetsVisible(false);      
    }
  else if (button == m_fromNetworkButton)
    {
      setLocalSubWidgetsVisible(false);
      setNetworkSubWidgetsVisible(true);      
    }
}

void CImportDialog::onUrlChanged(const QString & text)
{
  Q_UNUSED(text);
  m_urlButton->setChecked(true);
}

void CImportDialog::openItem(QListWidgetItem* item)
{
  if (!QDesktopServices::openUrl(QUrl::fromLocalFile(item->data(Qt::ToolTipRole).toString())) &&
      !QDesktopServices::openUrl(QUrl::fromLocalFile(item->data(Qt::DisplayRole).toString())) &&
      !QDesktopServices::openUrl(QUrl::fromLocalFile(item->data(Qt::EditRole).toString())) &&
      !QDesktopServices::openUrl(QUrl::fromLocalFile(item->data(Qt::WhatsThisRole).toString())) )
    {
      parent()->statusBar()->showMessage
	(tr("Can't open: %1")
	 .arg(item->data(Qt::DisplayRole).toString()));
    }
}

void CImportDialog::addFiles()
{
  QStringList paths = QFileDialog::getOpenFileNames(this, tr("Import songs"),
						    QDir::homePath(),
						    tr("Songs (*.sg)"));
  foreach (const QString & path, paths)
    {
      if (!m_songsToBeImported.contains(path))
	{
	  QFileInfo fileInfo(path);
	  //todo: set associated cover as icon
	  QListWidgetItem *item = new QListWidgetItem;
	  item->setIcon(QIcon(":/icons/songbook/48x48/song.png"));
	  item->setData(Qt::DisplayRole, fileInfo.fileName());
	  item->setData(Qt::WhatsThisRole, fileInfo.absoluteFilePath());
	  item->setData(Qt::ToolTipRole, fileInfo.absoluteFilePath());
	  m_fileList->addItem(item);
	  m_songsToBeImported << path;
	}
    }
}

void CImportDialog::removeFiles()
{
  QList<QListWidgetItem *> items = m_fileList->selectedItems();
  foreach (QListWidgetItem *item, items)
    {
      m_songsToBeImported.removeOne(item->data(Qt::WhatsThisRole).toString());
      m_fileList->removeItemWidget(item);
      delete item;
    }
}

bool CImportDialog::acceptDialog()
{
#ifdef ENABLE_LIBRARY_DOWNLOAD
  if (m_fromNetworkButton->isChecked())
    {
      if (m_patacrepButton->isChecked())
	{
	  m_url = QUrl("http://www.patacrep.com/data/documents/songbook.tar.gz");
	}
      else if (m_gitButton->isChecked())
	{
	  m_url = QUrl("http://git.lohrun.net/?p=songbook.git;a=snapshot;h=HEAD;sf=tgz");
	}
      else if (m_urlButton->isChecked() && QUrl(m_urlLineEdit->text()).isValid())
	{
	  m_url = QUrl(m_urlLineEdit->text());
	}
      else
	{
	  return false;
	}

      initDownload();
      downloadStart();
    }
#endif //ENABLE_LIBRARY_DOWNLOAD

  if (m_fromLocalButton->isChecked())
    {
      emit(songsReadyToBeImported(m_songsToBeImported));
    }

  CLibrary::instance()->setDirectory(m_libraryPath->path());
  writeSettings();
  accept();
  return true;
}

CProgressBar* CImportDialog::progressBar() const
{
  return parent()->progressBar();
}

void CImportDialog::showMessage(const QString & message)
{
  parent()->statusBar()->showMessage(message);
}

#ifdef ENABLE_LIBRARY_DOWNLOAD
void CImportDialog::initDownload()
{
  delete m_manager;
  m_manager = new QNetworkAccessManager(this);

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
  
  connect(progressBar(), SIGNAL(canceled()),
          this, SLOT(cancelDownload()));
}

bool CImportDialog::saveToDisk(const QString &filename, QIODevice *data)
{
  QFile file(filename);
  if (!file.open(QIODevice::WriteOnly))
    {
      showMessage(tr("Could not open %1 in write mode: %2")
		  .arg(qPrintable(filename))
		  .arg(qPrintable(file.errorString())));
      return false;
    }
  file.write(data->readAll());
  file.close();
  return true;
}

void CImportDialog::downloadStart()
{
  if (m_url.isValid())
    {
      // check if there already is a songbook directory in the specified path
      QDir dir(CMainWindow::_cachePath);
      QNetworkRequest request;
      request.setUrl(m_url);
      request.setRawHeader("User-Agent", "songbook-client a1");
      m_reply = m_manager->get(request);
      connect(m_reply, SIGNAL(finished()),
              this, SLOT(downloadFinished()));
      connect(m_reply, SIGNAL(sslErrors(QList<QSslError>)),
	      this, SLOT(sslErrors(QList<QSslError>)));
      connect(m_reply, SIGNAL(downloadProgress(qint64, qint64)),
	      this, SLOT(downloadProgress(qint64, qint64)));
      m_downloadTime.start();
    }
  else
    {
      qWarning() << tr("CImportDialog::downloadStart the following url is invalid: ") << m_url;
    }
}

void CImportDialog::downloadFinished()
{
  bool abort = false;

  if (m_reply->error())
    {
      showMessage(tr("Download of %1 failed: %2")
		  .arg(m_reply->url().toEncoded().constData())
		  .arg(qPrintable(m_reply->errorString())));
      abort = true;
    }

  QString filename = findFileName();

  if (!abort)
    {
      QDir dir(CMainWindow::_cachePath);
      QDir oldCurrent = QDir::currentPath();
      QString filepath = dir.filePath(filename);
      if (saveToDisk(filepath, m_reply))
	{
	  QDir::setCurrent(dir.absolutePath());
	  if (decompress(filepath))
	    {
	      CLibrary::recursiveFindFiles(dir.absolutePath(), QStringList() << "*.sg", m_songsToBeImported);
	      showMessage(tr("Download completed"));
	      emit(songsReadyToBeImported(m_songsToBeImported));
	    }
	  QDir::setCurrent(oldCurrent.absolutePath());
	}
      // remove the downloaded archive after decompressing
      dir.remove(filepath);
    }

  progressBar()->hide();
  disconnect(progressBar(), SIGNAL(canceled()), this, SLOT(cancelDownload()));
  m_reply->deleteLater();
}

void CImportDialog::sslErrors(const QList<QSslError> &sslErrors)
{
#ifndef QT_NO_OPENSSL
  foreach (const QSslError &error, sslErrors)
    qWarning() << "CImportDialog::sslErrors : " << error.errorString();
#endif
}

void CImportDialog::cancelDownload()
{
  if (m_reply)
    m_reply->abort();
  downloadFinished();
}

// Uses the code sample proposed in the libarchive documentation
// http://code.google.com/p/libarchive/wiki/Examples#A_Complete_Extractor
bool CImportDialog::decompress(const QString &filename)
{
  struct archive *archive;
  struct archive *ext;
  struct archive_entry *entry;
  int flags;
  int r;

  /* Select which attributes we want to restore. */
  flags = ARCHIVE_EXTRACT_TIME;
  flags |= ARCHIVE_EXTRACT_PERM;
  flags |= ARCHIVE_EXTRACT_ACL;
  flags |= ARCHIVE_EXTRACT_FFLAGS;

  archive = archive_read_new();
  archive_read_support_format_all(archive);
  archive_read_support_filter_all(archive);
  ext = archive_write_disk_new();
  archive_write_disk_set_options(ext, flags);
  archive_write_disk_set_standard_lookup(ext);

  if ((r = archive_read_open_filename(archive, filename.toStdString().c_str(), 10240)))
    {
      showMessage(tr("Unable to open the archive: %1").arg(filename));
      return false;
    }

  do
    {
      r = archive_read_next_header(archive, &entry);
      if (r == ARCHIVE_EOF)
	break;
      if (r != ARCHIVE_OK)
	showMessage(tr("Error: %1").arg(archive_error_string(archive)));
      if (r < ARCHIVE_WARN)
	return false;

      r = archive_write_header(ext, entry);
      if (r != ARCHIVE_OK)
	showMessage(tr("Error: %1").arg(archive_error_string(ext)));
      else if (archive_entry_size(entry) > 0)
	{
	  copy_data(archive, ext);
	  if (r != ARCHIVE_OK)
	    showMessage(tr("Error: %1").arg(archive_error_string(ext)));
	  if (r < ARCHIVE_WARN)
	    return false;
	}

      r = archive_write_finish_entry(ext);
      if (r != ARCHIVE_OK)
	{
	  showMessage(tr("Error: %1").arg(archive_error_string(ext)));
	}
      if (r < ARCHIVE_WARN)
	return false;
    }
  while (true);

  archive_read_close(archive);
  archive_read_free(archive);
  archive_write_close(ext);
  archive_write_free(ext);
  return true;
}

int CImportDialog::copy_data(struct archive *ar, struct archive *aw)
{
  const void *buff;
  size_t size;
  off_t offset;

  do
    {
      int r = archive_read_data_block(ar, &buff, &size, &offset);
      if (r == ARCHIVE_EOF)
	return (ARCHIVE_OK);
      if (r != ARCHIVE_OK)
	return (r);

      r = archive_write_data_block(aw, buff, size, offset);
      if (r != ARCHIVE_OK)
	{
	  showMessage(tr("Error: %1").arg(archive_error_string(aw)));
	  return (r);
	}
    }
  while (true);
}

QString CImportDialog::findFileName()
{
  if (!m_reply)
    {
      qWarning() << tr("CImportDialog::findFileName : invalid network reply");
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

QString CImportDialog::bytesToString(double value)
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
  return tr("%1 %2").arg(value, 2, 'f', 1).arg(unit);
}

void CImportDialog::downloadProgress(qint64 bytesRead, qint64 totalBytes)
{
  QString message = tr("Downloading %1").arg(findFileName());
  // download transfer
  message.append(tr(" - %1").arg(bytesToString(bytesRead)));

  if (totalBytes > -1)
    {
      // download size
      message.append(tr(" of %1").arg(bytesToString(totalBytes)));

      //update the progress bar
      progressBar()->setRange(0, totalBytes);
      progressBar()->setValue(bytesRead);
    }

  // download speed
  message.append(tr(" (%2/s)").arg(bytesToString(bytesRead * 1000.0 / m_downloadTime.elapsed())));

  showMessage(message);
  progressBar()->show();
}
#endif //ENABLE_LIBRARY_DOWNLOAD
