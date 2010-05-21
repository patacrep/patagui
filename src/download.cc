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
#include <iostream>
#include <QStatusBar>
#include <QLayout>
#include <QStatusTipEvent>

CDownloadDialog::CDownloadDialog(CMainWindow* parent)
  : QDialog()
{
  QDir dir;
  m_parent = parent;
  m_downloadPath = dir.homePath();
  m_gitRepoUrl   = QString("git://git.lohrun.net/songbook.git");
  m_progress = NULL;
  setModal(true);
  checkGitDependency();
  m_gitCheck->waitForFinished();
  //Git available
  QHBoxLayout* horizontalLayout1 = new QHBoxLayout;
  //horizontalLayout1->addWidget( new QLabel(tr("Checking if Git is available:")) );
  if(m_isGit) 
    horizontalLayout1->addWidget( new QLabel(QString(tr("<font color=green>Found %1</font>")).arg(m_gitVersion)) );
  else    
    horizontalLayout1->addWidget( new QLabel(tr("<font color=red>Warning: Git not found !</font>")) );

  //Download location
  QHBoxLayout* horizontalLayout2 = new QHBoxLayout;
  horizontalLayout2->addWidget( new QLabel(tr("Download into:")) );
  m_downloadLineEdit = new QLineEdit(QString("%1").arg(downloadPath()));
  horizontalLayout2->addWidget(m_downloadLineEdit);
  QToolButton* browseDownloadButton = new QToolButton;
  browseDownloadButton->setIcon(QIcon(":/icons/document-load.png"));
  horizontalLayout2->addWidget(browseDownloadButton);

  //Checkbox options
  QVBoxLayout *verticalLayout = new QVBoxLayout;
  m_cb1 = new QCheckBox(tr("Set as Patacrep songbook"), this);
  m_cb1->setChecked(true);
  verticalLayout->addWidget(m_cb1);
  m_cb2 = new QCheckBox(tr("Synchronize after download"), this);
  m_cb2->setChecked(true);
  verticalLayout->addWidget(m_cb2);
  m_cb3 = new QCheckBox(tr("Remove existing directory"), this);
  m_cb3->setChecked(false);
  verticalLayout->addWidget(m_cb3);

  //Specify custom repo
  QHBoxLayout* horizontalLayout3 = new QHBoxLayout;
  horizontalLayout3->addWidget( new QLabel(tr("Specify Git repository:")) );
  m_gitRepoLineEdit = new QLineEdit(QString("%1").arg(gitRepoUrl()));
  horizontalLayout3->addWidget(m_gitRepoLineEdit);

  // StatusBar & ProgressBar
  m_statusBar = new QStatusBar;
  m_progressBar = new QProgressBar(m_statusBar);
  m_progressBar->setTextVisible(false);
  m_progressBar->setRange(0, 0);
  m_progressBar->hide();
  m_statusBar->addPermanentWidget(m_progressBar);

  // Action buttons
  QDialogButtonBox * buttonBox = new QDialogButtonBox;
  QPushButton * buttonDownload = new QPushButton(tr("Download"));
  QPushButton * buttonClose = new QPushButton(tr("Close"));
  buttonClose->setDefault(true);
  buttonBox->addButton(buttonDownload, QDialogButtonBox::ActionRole);
  buttonBox->addButton(buttonClose, QDialogButtonBox::DestructiveRole);

  //Connect buttons
  connect(buttonDownload, SIGNAL(clicked()),
	  this, SLOT(download()) );
  connect(buttonClose, SIGNAL(clicked()),
	  this, SLOT(close()) );
  connect(browseDownloadButton, SIGNAL(clicked()),
	  this, SLOT(browseDownloadPath()) );

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addLayout(horizontalLayout1);
  mainLayout->addLayout(horizontalLayout2);
  mainLayout->addLayout(verticalLayout);  
  mainLayout->addLayout(horizontalLayout3);
  mainLayout->addWidget(buttonBox);
  mainLayout->addWidget(m_statusBar);
  setLayout(mainLayout);

  setWindowTitle(tr("Download"));
  setMinimumWidth(450);
  show();
}
//------------------------------------------------------------------------------
QString CDownloadDialog::downloadPath() const
{
  return m_downloadPath;
}
//------------------------------------------------------------------------------
void CDownloadDialog::setDownloadPath(QString ADownloadPath )
{
  while(ADownloadPath.endsWith("/"))
    ADownloadPath.remove(ADownloadPath.lastIndexOf("/"),1);

  if(!ADownloadPath.isEmpty())
    m_downloadPath = ADownloadPath;
}
//------------------------------------------------------------------------------
QString CDownloadDialog::gitRepoUrl()
{
  return m_gitRepoUrl;
}
//------------------------------------------------------------------------------
void CDownloadDialog::setGitRepoUrl(const QString & url )
{
  m_gitRepoUrl = url;
}
//------------------------------------------------------------------------------
void CDownloadDialog::processExit(int exitCode, QProcess::ExitStatus exitStatus)
{
  if(exitStatus == QProcess::NormalExit && exitCode==0)
    {
      if(m_cb1->isChecked())
	  m_parent->setWorkingPath(QString("%1/songbook").arg(downloadPath()));
      
      if(m_cb2->isChecked())
	{
	  QString msg("Synchronizing database ...");
	  m_statusBar->showMessage(msg);
     	  m_parent->synchroniseWithLocalSongs();
	}
      
      QString msg("Success!");
      m_statusBar->showMessage(msg);
      m_progressBar->hide();
    }
}
//------------------------------------------------------------------------------
void CDownloadDialog::processError(QProcess::ProcessError error)
{
  QMessageBox msgBox;
  msgBox.setIcon(QMessageBox::Critical);
  msgBox.setText(tr("Sorry: an error occured during the download."));
  msgBox.setStandardButtons(QMessageBox::Cancel);
  msgBox.setDefaultButton(QMessageBox::Cancel);
  msgBox.exec();
}
//------------------------------------------------------------------------------
void CDownloadDialog::gitProcessExit(int exitCode, QProcess::ExitStatus exitStatus)
{
  if(exitStatus == QProcess::NormalExit && exitCode==0)
    m_isGit = true;
}
//------------------------------------------------------------------------------
void CDownloadDialog::gitProcessError(QProcess::ProcessError error)
{
  QMessageBox msgBox;
  msgBox.setIcon(QMessageBox::Warning);
  msgBox.setText(tr("Unable to find <a href=\"http://git-scm.com\">Git</a> application. \n Please install or use manual download from <a href=\"http::www.patacrep.com\">www.patacrep.com</a>."));
  msgBox.setStandardButtons(QMessageBox::Cancel);
  msgBox.setDefaultButton(QMessageBox::Cancel);
  msgBox.exec();
  m_isGit = false;
}
//------------------------------------------------------------------------------
void CDownloadDialog::readProcessOut()
{
  m_gitVersion = m_gitCheck->readAllStandardOutput().data();
}

//------------------------------------------------------------------------------
void CDownloadDialog::download()
{
  if(m_isGit && applyDialogOptions())
    {
      m_process = new QProcess(this);
      m_process->setWorkingDirectory(downloadPath());
      connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)), 
	      this, SLOT(processExit(int,QProcess::ExitStatus)));
      connect(m_process, SIGNAL(error(QProcess::ProcessError)), 
	      this, SLOT(processError(QProcess::ProcessError)));
      
      QStringList args;
      args << "clone" << gitRepoUrl();
      QString msg = QString(tr("Downloading from %1")).arg(gitRepoUrl());
      m_statusBar->showMessage(msg);
      m_progressBar->show();
      m_process->start("git", args);
    }
}
//------------------------------------------------------------------------------
void CDownloadDialog::browseDownloadPath()
{
  QString directory = QFileDialog::getExistingDirectory(this,
                                                        tr("Download into"), 
                                                        m_downloadLineEdit->text());
  if (!directory.isEmpty())
    m_downloadLineEdit->setText(directory);
 
}
//------------------------------------------------------------------------------
void CDownloadDialog::checkGitDependency()
{
  m_gitCheck = new QProcess(this);
  connect(m_gitCheck, SIGNAL(finished(int,QProcess::ExitStatus)), 
	  this, SLOT(gitProcessExit(int,QProcess::ExitStatus)));
  connect(m_gitCheck, SIGNAL(error(QProcess::ProcessError)), 
	  this, SLOT(gitProcessError(QProcess::ProcessError)));
  connect(m_gitCheck, SIGNAL(readyReadStandardOutput()), 
	  this, SLOT(readProcessOut()));

  QStringList args;
  args << "--version";
  m_gitCheck->start("git", args);
}
//------------------------------------------------------------------------------
bool CDownloadDialog::applyDialogOptions()
{
  //Retrieve download path
  QString path = m_downloadLineEdit->text();
  QDir dir(path);
  if( !dir.exists() )
    {
      QMessageBox msgBox;
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.setText( tr("The download directory does not exist.") );
      msgBox.setStandardButtons(QMessageBox::Cancel);
      msgBox.setDefaultButton(QMessageBox::Cancel);
      msgBox.exec();
      return false;
    }

  dir.setPath(QString("%1/songbook").arg(path));
  if( dir.exists() && !m_cb3->isChecked() )
    {
      QMessageBox msgBox;
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.setText( QString(tr("The download directory: %1 \n appears to contain an existing songbook/ subdirectory. \n Please select a new location or force remove.")).arg(path) );
      msgBox.setStandardButtons(QMessageBox::Cancel);
      msgBox.setDefaultButton(QMessageBox::Cancel);
      msgBox.exec();
      return false;
    }
  setDownloadPath( m_downloadLineEdit->text() );


  if(m_cb3->isChecked())
    {
      //Confirmation dialog
      QString rmPath = QString("%1/songbook").arg(downloadPath());

      if(QMessageBox::question(this, tr("Remove download directory"), 
			       QString(tr("This will remove all files in %1 \n Are you sure you want to continue ?")).arg(rmPath), 
			       QMessageBox::Yes, 
			       QMessageBox::No, 
			       QMessageBox::NoButton) == QMessageBox::No)
	return false;

      std::cout<<" remove dir "<<std::endl;
      //strange to use a qprocess but 
      //can't find recursive remove  qt method
      QStringList args;
      args << "-rf" << rmPath;
      QProcess rmProcess;
      rmProcess.start("rm", args);
      rmProcess.waitForFinished();
    }

  //Retrieve git repository url
  setGitRepoUrl( m_gitRepoLineEdit->text() );
  
  //Check url
  if (!QUrl(gitRepoUrl()).isValid()) 
    {
      QMessageBox msgBox;
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.setText(tr("The Git repository url is not valid."));
      msgBox.setStandardButtons(QMessageBox::Cancel);
      msgBox.setDefaultButton(QMessageBox::Cancel);
      msgBox.exec();
      return false;
    }
  std::cout<<" apply options success"<<std::endl;
  return true;
}
//------------------------------------------------------------------------------
bool CDownloadDialog::event(QEvent* e)
{
  if(e->type()==QEvent::StatusTip)
    {
      QStatusTipEvent *ev = (QStatusTipEvent*)e;
      m_statusBar->showMessage(ev->tip());
      return true;
    }
  return QDialog::event(e);
}

