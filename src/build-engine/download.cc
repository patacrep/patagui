// Copyright (C) 2009,2010 Romain Goffe, Alexandre Dupas
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
#include "download.hh"
#include "mainwindow.hh"
#include "file-chooser.hh"
#include <QLayout>

CDownload::CDownload(CMainWindow* AParent)
  : CBuildEngine(AParent)
  ,m_downloadPath(QDir::home().path())
{
  setWindowTitle(tr("Download library"));
  setStatusActionMessage(tr("Downloading the songbook library. Please wait ..."));
  setStatusSuccessMessage(tr("Download completed."));
  setStatusErrorMessage(tr("An error occured during the download."));

  m_gitLabel = new QLabel(tr("<a href=\"http://git-scm.com/\">git</a>: <font color=orange>%1</font>"));
  setGitRepoUrl(QString("http://lohrun.net/git/songbook.git"));

  setProcessName("git");
  setProcessOptions(QStringList() << "clone" << "--quiet" << "--depth" << "1" << gitRepoUrl());
}
//------------------------------------------------------------------------------
QWidget* CDownload::mainWidget()
{
  if(!checkGitDependency())
    return NULL;

  QLineEdit* gitRepoLineEdit = new QLineEdit(gitRepoUrl());
  connect(gitRepoLineEdit, SIGNAL(textChanged(QString)),
	  this, SLOT(setGitRepoUrl(QString)));

  CFileChooser *download = new CFileChooser();
  download->setType(CFileChooser::DirectoryChooser);
  download->setCaption(tr("Target directory"));
  download->setPath(downloadPath());
  connect(download, SIGNAL(pathChanged(QString)),
	  this, SLOT(setDownloadPath(QString)));

  QWidget* widget = new QWidget;
  QFormLayout* layout = new QFormLayout;
  layout->addRow(tr("Remote repository:"), gitRepoLineEdit);
  layout->addRow(tr("Target directory:"),  download);
  widget->setLayout(layout);
  return widget;
}
//------------------------------------------------------------------------------
void CDownload::processExit(int exitCode, QProcess::ExitStatus exitStatus)
{
  CBuildEngine::processExit(exitCode, exitStatus);
  parent()->setWorkingPath(QString("%1/songbook").arg(downloadPath()));
  QSettings settings;
  settings.setValue("workingPath", parent()->workingPath());
}
//------------------------------------------------------------------------------
void CDownload::action()
{
  QDir dir(downloadPath());
  if ( !dir.exists() )
    {
      parent()->statusBar()->showMessage(tr("The download directory does not exist."));
      return;
    }

  QString rmPath = QString("%1/songbook").arg(downloadPath());
  dir.setPath(rmPath);
  if ( dir.exists() )
    {
      if( QMessageBox::question(NULL, QString(tr("File conflict")),
				QString(tr("The download directory: \"%1\" \n "
					   "already contains a \"songbook\" subdirectory. \n"
					   "Would you like to replace all existing content ? \n"
					   "Note that this will remove all files in \"%2\".")).arg(downloadPath()).arg(rmPath),
				QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton) == QMessageBox::Yes )
	{
	  QProcess rmProcess;
	  rmProcess.start("rm", QStringList() << "-rf" << rmPath);
	  rmProcess.waitForFinished();
	}
      else
	return;
    }

  if (!QUrl(gitRepoUrl()).isValid())
    {
      QMessageBox msgBox;
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.setText(tr("The Git repository url is not valid."));
      msgBox.setStandardButtons(QMessageBox::Cancel);
      msgBox.setDefaultButton(QMessageBox::Cancel);
      msgBox.exec();
      return;
    }
  process()->setWorkingDirectory(downloadPath());
  CBuildEngine::action();
}
//------------------------------------------------------------------------------
bool CDownload::checkGitDependency()
{
  QProcess *process = new QProcess(m_gitLabel);
  process->start("git", QStringList() << "--version");

  if (process->waitForFinished())
    {
      QRegExp rx("git version ([^\n]+)");
      rx.indexIn(process->readAllStandardOutput().data());
      m_gitLabel->setText(m_gitLabel->text().replace("orange","green").arg(rx.cap(1)));
      return true;
    }

  parent()->statusBar()->showMessage(tr("Unable to find Git application."));
  return false;
}
//------------------------------------------------------------------------------
QString CDownload::downloadPath()
{
  return m_downloadPath;
}
//------------------------------------------------------------------------------
void CDownload::setDownloadPath(QString ADownloadPath )
{
  m_downloadPath = ADownloadPath;
}
//------------------------------------------------------------------------------
QString CDownload::gitRepoUrl()
{
  return m_gitRepoUrl;
}
//------------------------------------------------------------------------------
void CDownload::setGitRepoUrl(QString url)
{
  m_gitRepoUrl = url;
}
