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

#include "build-engine.hh"
#include "mainwindow.hh"
#include "highlighter.hh"

CBuildEngine::CBuildEngine(CMainWindow* parent)
  : QWidget()
{
  m_parent = parent;
  
  m_process = new QProcess(parent);
  m_process->setWorkingDirectory(workingPath());
   
  setStatusSuccessMessage(tr("Success!"));
  setStatusErrorMessage(tr("Error!"));
  
  connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)),
	  this, SLOT(processExit(int,QProcess::ExitStatus)));

  connect(m_process, SIGNAL(error(QProcess::ProcessError)),
	  this, SLOT(processError(QProcess::ProcessError)));

  connect(m_process, SIGNAL(readyReadStandardOutput()),
	  this, SLOT(readProcessOut()));
  
  connect(m_process, SIGNAL(readyReadStandardError()),
	  this, SLOT(readProcessOut()));
}
//------------------------------------------------------------------------------
QString CBuildEngine::workingPath()
{
  return parent()->workingPath();
}
//------------------------------------------------------------------------------
CMainWindow* CBuildEngine::parent()
{
  if(!m_parent) qWarning() << "CBuildEngine::parent invalid parent" ;
  return m_parent ;
}
//------------------------------------------------------------------------------
void CBuildEngine::processExit(int exitCode, QProcess::ExitStatus exitStatus)
{
  parent()->progressBar()->hide();
  
  if (exitStatus == QProcess::NormalExit && exitCode==0)
    parent()->statusBar()->showMessage(statusSuccessMessage());
  else
    processError(QProcess::UnknownError);
}
//------------------------------------------------------------------------------
void CBuildEngine::processError(QProcess::ProcessError error)
{
  parent()->progressBar()->hide();
  
  QMessageBox msgBox;
  msgBox.setIcon(QMessageBox::Critical);
  msgBox.setText(statusErrorMessage());
  msgBox.setDetailedText(parent()->log()->toPlainText());
  msgBox.setStandardButtons(QMessageBox::Cancel);
  msgBox.setDefaultButton(QMessageBox::Cancel);
  msgBox.exec();
}
//------------------------------------------------------------------------------
void CBuildEngine::readProcessOut()
{
  parent()->log()->append(m_process->readAllStandardOutput().data());
}
//------------------------------------------------------------------------------
void CBuildEngine::dialog()
{
  QWidget* widget = mainWidget();
  if(!widget) return;

  m_dialog = new QDialog;

  QDialogButtonBox * buttonBox = new QDialogButtonBox;
  QPushButton * button = new QPushButton(tr("Close"));
  buttonBox->addButton(button, QDialogButtonBox::DestructiveRole);
  connect(button, SIGNAL(clicked()), m_dialog, SLOT(close()) );

  button = new QPushButton(tr("Apply"));
  button->setDefault(true);
  buttonBox->addButton(button, QDialogButtonBox::ActionRole);
  connect(button, SIGNAL(clicked()), this, SLOT(action()) );
  
  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(widget);
  layout->addWidget(buttonBox);
  m_dialog->setLayout(layout);

  m_dialog->setWindowTitle(windowTitle());
  m_dialog->setMinimumWidth(450);
  m_dialog->setMinimumHeight(450);
  m_dialog->show();
}
//------------------------------------------------------------------------------
void CBuildEngine::updateDialog()
{
  qDebug() << "CBuildEngine::updateDialog not implemented yet";
  //if(m_dialog)
  //  m_dialog->close();
  //dialog();
}
//------------------------------------------------------------------------------
void CBuildEngine::action()
{
  parent()->statusBar()->showMessage(statusActionMessage());
  parent()->progressBar()->show();
  parent()->log()->clear();
  QApplication::processEvents();
  
  process()->start(fileName(), processOptions());
}
//------------------------------------------------------------------------------
QString CBuildEngine::windowTitle()
{
  return m_windowTitle;
}
//------------------------------------------------------------------------------
void CBuildEngine::setWindowTitle(const QString & value)
{
  m_windowTitle = value;
}
//------------------------------------------------------------------------------
QString CBuildEngine::fileName()
{
  return m_fileName;
}
//------------------------------------------------------------------------------
void CBuildEngine::setFileName(const QString & value)
{
  m_fileName = value;
}
//------------------------------------------------------------------------------
QString CBuildEngine::statusSuccessMessage()
{
  return m_statusSuccessMessage;
}
//------------------------------------------------------------------------------
void CBuildEngine::setStatusSuccessMessage(const QString & value)
{
  m_statusSuccessMessage = value;
}
//------------------------------------------------------------------------------
QString CBuildEngine::statusErrorMessage()
{
  return m_statusErrorMessage;
}
//------------------------------------------------------------------------------
void CBuildEngine::setStatusErrorMessage(const QString & value)
{
  m_statusErrorMessage = value;
}
//------------------------------------------------------------------------------
QString CBuildEngine::statusActionMessage()
{
  return m_statusActionMessage;
}
//------------------------------------------------------------------------------
void CBuildEngine::setStatusActionMessage(const QString & value)
{
  m_statusActionMessage = value;
}
//------------------------------------------------------------------------------
QStringList CBuildEngine::processOptions()
{
  return m_processOptions;
}
//------------------------------------------------------------------------------
void CBuildEngine::setProcessOptions(const QStringList & value)
{
  m_processOptions = value;
}
//------------------------------------------------------------------------------
QProcess* CBuildEngine::process()
{
  return m_process;
}
