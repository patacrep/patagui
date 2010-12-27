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
#ifndef __BUILD_ENGINE_HH__
#define __BUILD_ENGINE_HH__

#include <QWidget>
#include <QString>
#include <QProcess>

class QDialog;
class QTextEdit;
class CMainWindow;

class CBuildEngine : public QWidget
{
  Q_OBJECT
  
  public:
  CBuildEngine(CMainWindow* parent=NULL);
  				
public slots:
  void processExit(int exitCode, QProcess::ExitStatus exitStatus);
  void processError(QProcess::ProcessError error);
  void readProcessOut();
  
  void dialog();
  void updateDialog();
  void action();
  
public:
  virtual QWidget* mainWidget()=0;
  CMainWindow* parent();
  
  QString fileName();
  void setFileName(const QString & value);

  QString windowTitle();
  void setWindowTitle(const QString & value);
  
  QString statusSuccessMessage();
  void setStatusSuccessMessage(const QString & value);

  QString statusErrorMessage();
  void setStatusErrorMessage(const QString & value);

  QString statusActionMessage();
  void setStatusActionMessage(const QString & value);

  QStringList processOptions();
  void setProcessOptions(const QStringList & value);
    
  QString workingPath();

protected:
  CMainWindow* m_parent;
  QProcess* m_process;
  QDialog* m_dialog;
  QTextEdit* m_log;//
  
  QString m_fileName;
  QString m_windowTitle;
  QString m_statusActionMessage;
  QString m_statusSuccessMessage;
  QString m_statusErrorMessage;
  QStringList m_processOptions;
  
};

#endif // __BUILD_ENGINE_HH__
