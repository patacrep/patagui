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
#ifndef __DOWNLOAD_HH__
#define __DOWNLOAD_HH__

#include <QtGui>
#include <QString>

class QProcess;
class QStatusBar;
class CMainWindow;

class CDownloadDialog : public QDialog
{
  Q_OBJECT
  
public:
  CDownloadDialog(CMainWindow* AParent);

  QString downloadPath() const;
  void setDownloadPath(QString);

  QString gitRepoUrl();
  void setGitRepoUrl(const QString &);

private slots:
  void processExit(int exitCode, QProcess::ExitStatus exitStatus);
  void processError(QProcess::ProcessError error);
  void gitProcessExit(int exitCode, QProcess::ExitStatus exitStatus);
  void gitProcessError(QProcess::ProcessError error);
  void readProcessOut();
  void download();
  void browseDownloadPath();

private:
  void checkGitDependency();
  bool applyDialogOptions();

protected:
  bool event(QEvent *e);

private:
  // checking of git is available
  QProcess* m_gitCheck;
  bool m_isGit;
  QString m_gitVersion;

  // main download process
  QProcess* m_process;

  // dialog variables
  QProgressBar* m_progressBar;
  QStatusBar*   m_statusBar;
  QString m_downloadPath;
  QString m_gitRepoUrl;
  QProgressDialog* m_progress;
  QLineEdit* m_downloadLineEdit;
  QLineEdit* m_gitRepoLineEdit;
  QCheckBox* m_cb1; //set as patacrep songbook
  QCheckBox* m_cb2; //synchronize after download
  QCheckBox* m_cb3; //remove existing dir

  // mainwindow
  CMainWindow* m_parent;

};

#endif // __DOWNLOAD_HH__
