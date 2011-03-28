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

#include "build-engine.hh"

class CMainWindow;

class CDownload : public CBuildEngine
{
  Q_OBJECT

public:
  CDownload(CMainWindow* AParent);
  virtual QWidget* mainWidget();
  virtual void action();

public slots:
  virtual void processExit(int exitCode, QProcess::ExitStatus exitStatus);

private slots:
  void setGitRepoUrl(QString);
  void setDownloadPath(QString);

private:
  QString downloadPath();
  QString gitRepoUrl();

  bool checkGitDependency();
  bool applyDialogOptions();

  QString m_downloadPath;
  QString m_gitRepoUrl;
};

#endif // __DOWNLOAD_HH__
