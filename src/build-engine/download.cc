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
#include <QLayout>

CDownload::CDownload(CMainWindow* AParent)
  : CBuildEngine(AParent)
{
  setWindowTitle(tr("Download library"));
  setStatusActionMessage(tr("Downloading the songbook library. Please wait ..."));
  setStatusSuccessMessage(tr("Download completed."));
  setStatusErrorMessage(tr("An error occured during the download."));

  m_gitLabel = new QLabel(tr("<a href=\"http://git-scm.com/\">git</a>: <font color=orange>%1</font>"));

  setGitRepoUrl(QString("http://lohrun.net/git/songbook.git"));
  setDownloadPath(QDir::homePath()); // also initialise process working dir

  m_gitRepoLineEdit = new QLineEdit(gitRepoUrl());
  connect(m_gitRepoLineEdit, SIGNAL(textChanged(QString)), this, SLOT(setGitRepoUrl(QString)));

  m_downloadLineEdit = new QLineEdit(downloadPath());
  connect(m_downloadLineEdit, SIGNAL(textChanged(QString)), this, SLOT(setDownloadPath(QString)));

  setFileName("git");
  setProcessOptions(QStringList() << "clone" << "--quiet" << "--depth" << "1" << gitRepoUrl());
}
//------------------------------------------------------------------------------
QWidget* CDownload::mainWidget()
{
  if(!checkGitDependency())
    return NULL;

  QWidget* widget = new QWidget;
  QPushButton* button = new QPushButton(tr("Browse"));
  connect(button, SIGNAL(clicked()), this, SLOT(browse()) );

  QLayout* layout = new QGridLayout;
  static_cast<QGridLayout*>(layout)->addWidget(new QLabel(tr("Remote repository:")), 0,0,1,1);
  static_cast<QGridLayout*>(layout)->addWidget(m_gitRepoLineEdit, 0,1,1,2);
  static_cast<QGridLayout*>(layout)->addWidget(new QLabel(tr("Target directory:")), 1,0,1,1);
  static_cast<QGridLayout*>(layout)->addWidget(m_downloadLineEdit, 1,1,1,1);
  static_cast<QGridLayout*>(layout)->addWidget(button, 1,2,1,1);
  static_cast<QGridLayout*>(layout)->setRowStretch(2,1);
  static_cast<QGridLayout*>(layout)->setColumnStretch(1,1);
  widget->setLayout(layout);
  return widget;
}
//------------------------------------------------------------------------------
void CDownload::processExit(int exitCode, QProcess::ExitStatus exitStatus)
{
  CBuildEngine::processExit(exitCode, exitStatus);
  parent()->setWorkingPath(QString("%1/songbook").arg(downloadPath()));
}
//------------------------------------------------------------------------------
void CDownload::action()
{
  QDir dir(downloadPath());
  if ( !dir.exists() )
    {
      QMessageBox msgBox;
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.setText( tr("The download directory does not exist.") );
      msgBox.setStandardButtons(QMessageBox::Cancel);
      msgBox.setDefaultButton(QMessageBox::Cancel);
      msgBox.exec();
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

  QMessageBox msgBox;
  msgBox.setIcon(QMessageBox::Warning);
  msgBox.setText(tr("Unable to find <a href=\"http://git-scm.com\">Git</a> application. \n"
		    "Please install or use manual download from <a href=\"http::www.patacrep.com\">www.patacrep.com</a>."));
  msgBox.setStandardButtons(QMessageBox::Cancel);
  msgBox.setDefaultButton(QMessageBox::Cancel);
  msgBox.exec();
  return false;
}
//------------------------------------------------------------------------------
void CDownload::browse()
{
  QString directory = QFileDialog::getExistingDirectory(this, tr("Target directory"),
                                                        downloadPath());
  if (!directory.isEmpty())
    m_downloadLineEdit->setText(directory);
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
  process()->setWorkingDirectory(m_downloadPath);
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
