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

CTools::CTools(CMainWindow* parent)
  : QWidget()
{
  m_parent = parent;
  m_coverList = NULL;
  m_dialogResizeCovers = NULL;
}
//------------------------------------------------------------------------------
QString CTools::workingPath()
{
  return m_parent->workingPath();
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
  msgBox.setText(tr("Warning: an error occured while applying this tool."));
  msgBox.setStandardButtons(QMessageBox::Cancel);
  msgBox.setDefaultButton(QMessageBox::Cancel);
  msgBox.exec();
}
//------------------------------------------------------------------------------
void CTools::resizeCoversDialog()
{
  if(m_dialogResizeCovers) 
    m_dialogResizeCovers->close();

  m_dialogResizeCovers = new QDialog;

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
	  m_dialogResizeCovers, SLOT(close()) );

  //retrieve cover files 
  coverList();

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(m_coverList);
  mainLayout->addWidget(buttonBox);
  m_dialogResizeCovers->setLayout(mainLayout);
  
  m_dialogResizeCovers->setWindowTitle(tr("Resize covers"));
  m_dialogResizeCovers->setMinimumWidth(450);
  m_dialogResizeCovers->setMinimumHeight(450);
  m_dialogResizeCovers->show();
}
//------------------------------------------------------------------------------
void CTools::coverList()
{
  if(m_coverList) 
    delete m_coverList;

  QStringList filter;
  filter << "*.jpg" << "*.png" << "*.JPG" ;
  QString path = QString("%1/songs/").arg(workingPath());
  QDirIterator it(path, filter, QDir::NoFilter, QDirIterator::Subdirectories);

  m_coverList = new QListWidget;
  QColor green(138,226,52,100);
  QColor red(239,41,41,100);  
  while(it.hasNext())
    {
      QString filename = it.next();
      QFileInfo fi(filename);
      QString name = fi.fileName();
      QPixmap pixmap = QPixmap::fromImage(QImage(filename));
      QIcon cover(pixmap.scaledToWidth(24));
      
      //create item from current cover
      QListWidgetItem* item = new QListWidgetItem(cover, name);
      if(pixmap.height()>128)
	item->setBackground(QBrush(red));
      else
	item->setBackground(QBrush(green));
      //apppend items 
      m_coverList->addItem(item);
    }
}
//------------------------------------------------------------------------------
void CTools::resizeCovers()
{
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

  //warning: slot may be used from mainwindow (no m_coverlist defined)
  if(m_coverList)
      resizeCoversDialog();
}
//------------------------------------------------------------------------------
void CTools::latexPreprocessingDialog()
{
  QDialog *dialog = new QDialog;

  // Action buttons
  QDialogButtonBox * buttonBox = new QDialogButtonBox;
  QPushButton * buttonApply = new QPushButton(tr("Apply"));
  //QPushButton * buttonRule = new QPushButton(tr("Add rule"));
  QPushButton * buttonClose = new QPushButton(tr("Close"));
  buttonApply->setDefault(true);
  buttonBox->addButton(buttonApply, QDialogButtonBox::ActionRole);
  //buttonBox->addButton(buttonRule, QDialogButtonBox::ActionRole);
  buttonBox->addButton(buttonClose, QDialogButtonBox::DestructiveRole);

  //Connect buttons
  connect(buttonApply, SIGNAL(clicked()),
	  this, SLOT(latexPreprocessing()) );
  connect(buttonClose, SIGNAL(clicked()),
	  dialog, SLOT(close()) );
  //connect(buttonRule, SIGNAL(clicked()),
  //	  this, SLOT(addRule()) );

  QListWidget* list = new QListWidget;
  QColor orange(252,175,62,150);
  QColor yellow(252,233,79,150);

  //Retrieve rules from ./utils/latex-preprocessing file
  QFile file(QString("%1/utils/latex-preprocessing.py").arg(workingPath()));
  if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      QTextStream in(&file);
      QRegExp filter("^##:");
      QString line;
      bool rule = false;
      QListWidgetItem *item;
      do {
        line = in.readLine();

	if (line.startsWith("###")) //end of rules
	  break;

        if (line.startsWith("##:")) //category
	  {
	    item = new QListWidgetItem("\n" + line.remove(filter) + "\n");
	    item->setBackground(QBrush(orange));
	    list->addItem(item);
	    rule = true;
	  }
	else if(rule)
	  {
	    if(line.startsWith("#")) //subcategory
	      {	 
		item = new QListWidgetItem(line);
		item->setBackground(QBrush(yellow));
	      }
	    else
	      {
		line.replace(":", "  ->  ");
		line.chop(1);
		item = new QListWidgetItem(line);
	      }
	    list->addItem(item);
	  }
      } while (!line.isNull());
      file.close();
    }
  else
    {
      qWarning() << "unable to open file in read mode: utils/latex-preprocessing";
    }
 
  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(list);
  mainLayout->addWidget(buttonBox);
  dialog->setLayout(mainLayout);

  dialog->setWindowTitle(tr("LaTeX Preprocessing"));
  dialog->setMinimumWidth(400);
  dialog->setMinimumHeight(450);
  dialog->show();
}
//------------------------------------------------------------------------------
void CTools::latexPreprocessing()
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
//------------------------------------------------------------------------------
//void CTools::addRule()
//{
//
//}
