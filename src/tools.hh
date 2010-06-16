// Copyright (C) 2009 Romain Goffe, Alexandre Dupas
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
#ifndef __TOOLS_HH__
#define __TOOLS_HH__

#include <QtGui>
#include <QString>

class QProcess;
class CMainWindow;

class CTools : public QWidget
{
  Q_OBJECT

public:
  CTools(const QString & APath, CMainWindow* AParent);

public slots:
  void toolProcessExit(int exitCode, QProcess::ExitStatus exitStatus);
  void toolProcessError(QProcess::ProcessError error);
  void resizeCovers();
  void globalCheck();

private:
  QString workingPath();
  QProcess* m_process;
  QString m_workingPath;
  CMainWindow* m_parent;
};

#endif // __TOOLS_HH__
