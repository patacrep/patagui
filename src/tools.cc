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

#include "tools.hh"
#include "mainwindow.hh"

CTools::CTools(const QString & APath, CMainWindow* parent)
  : QWidget()
{
  m_workingPath = APath;
  m_parent = parent;
}
//------------------------------------------------------------------------------
QString CTools::workingPath()
{
  return m_workingPath;
}
//------------------------------------------------------------------------------
void CTools::toolProcessExit(int exitCode, QProcess::ExitStatus exitStatus)
{
  if (exitStatus == QProcess::NormalExit && exitCode==0)
    {
      m_parent->progressBar()->hide();
      m_parent->statusBar()->showMessage(tr("Success!"));
    }
}
//------------------------------------------------------------------------------
void CTools::toolProcessError(QProcess::ProcessError error)
{
  QMessageBox msgBox;
  msgBox.setIcon(QMessageBox::Critical);
  msgBox.setText(tr("Sorry: an error occured while applying this tool."));
  msgBox.setStandardButtons(QMessageBox::Cancel);
  msgBox.setDefaultButton(QMessageBox::Cancel);
  msgBox.exec();
}
//------------------------------------------------------------------------------
void CTools::resizeCoversDialog()
{
  QDialog *dialog = new QDialog;

  // Action buttons
  QDialogButtonBox * buttonBox = new QDialogButtonBox;
  QPushButton * buttonResize = new QPushButton(tr("Resize"));
  QPushButton * buttonClose = new QPushButton(tr("Close"));
  buttonResize->setDefault(true);
  buttonBox->addButton(buttonResize, QDialogButtonBox::ActionRole);
  buttonBox->addButton(buttonClose, QDialogButtonBox::DestructiveRole);

  //Connect buttons
  connect(buttonResize, SIGNAL(clicked()),
	  this, SLOT(resizeCovers()) );
  connect(buttonClose, SIGNAL(clicked()),
	  dialog, SLOT(close()) );

  //retrieve cover files
  QStringList filter;
  filter << "*.jpg";
  QString path = QString("%1/songs/").arg(workingPath());
  QDirIterator it(path, filter, QDir::NoFilter, QDirIterator::Subdirectories);

  QListWidget* list = new QListWidget;
  QColor green(138,226,52,100);
  QColor red(239,41,41,100);  
  while(it.hasNext())
    {
      QString filename = it.next();
      QFileInfo fi(filename);
      QString basename = fi.baseName();
      QPixmap pixmap = QPixmap::fromImage(QImage(filename));
      QIcon cover(pixmap.scaledToWidth(24));
      
      //create item from current cover
      QListWidgetItem* item = new QListWidgetItem(cover, basename);
      if(pixmap.height()>128)
	item->setBackground(QBrush(red));
      else
	item->setBackground(QBrush(green));
      //apppend items in coversTable
      list->addItem(item);
    }

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(list);
  mainLayout->addWidget(buttonBox);
  dialog->setLayout(mainLayout);

  dialog->setWindowTitle(tr("Resize covers"));
  dialog->setMinimumWidth(450);
  dialog->setMinimumHeight(450);
  dialog->show();
}
//------------------------------------------------------------------------------
void CTools::resizeCovers()
{
  //todo: do not call extern script but do it here
  m_process = new QProcess(this);
  m_process->setWorkingDirectory(workingPath());
  connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)),
	  this, SLOT(toolProcessExit(int,QProcess::ExitStatus)));

  connect(m_process, SIGNAL(error(QProcess::ProcessError)),
	  this, SLOT(toolProcessError(QProcess::ProcessError)));

  QString msg(tr("Checking for covers to resize ..."));
  m_parent->statusBar()->showMessage(msg);
  m_parent->progressBar()->show();
  m_process->start("./utils/resize-cover.sh");

  if (!m_process->waitForFinished())
    delete m_process;
}
//------------------------------------------------------------------------------
void CTools::globalCheck()
{
  m_process = new QProcess(this);
  m_process->setWorkingDirectory(workingPath());
  connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)),
	  this, SLOT(toolProcessExit(int,QProcess::ExitStatus)));

  connect(m_process, SIGNAL(error(QProcess::ProcessError)),
	  this, SLOT(toolProcessError(QProcess::ProcessError)));

  QString msg(tr("Applying typo rules ..."));
  m_parent->statusBar()->showMessage(msg);
  m_parent->progressBar()->show();
  m_process->start("./utils/latex-preprocessing.py");

  if (!m_process->waitForFinished())
    delete m_process;
}
