// Copyright (C) 2010 Romain Goffe, Alexandre Dupas
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
#include "song-editor.hh"
#include <QToolBar>
#include <QAction>
#include <QTextEdit>
#include <QLayout>
#include <QFile>
#include <QTextStream>
#include <QDebug>

//------------------------------------------------------------------------------
CSongEditor::CSongEditor(const QString & APath)
{
  m_filePath = APath;
  
  // toolbar
  QToolBar* toolbar = new QToolBar;
  toolbar->setMovable(false);

  // actions
  QAction* action = new QAction(tr("Save"), this);
  action->setShortcut(tr("Ctrl+S"));
  action->setStatusTip(tr("Save modifications"));
  connect(action, SIGNAL(triggered()), this, SLOT(save()));
  toolbar->addAction(action);

  //retrieve song text
  m_textEdit = new QTextEdit;
  QFile file(APath);
  if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {      
      QTextStream stream (&file);
      QString text = stream.readAll();
      file.close();
      m_textEdit->setText(text);
    }
  else
    {
      qWarning() << "CSongEditor warning: unable to open file in read mode";
    }
  
  QBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(toolbar);
  layout->addWidget(m_textEdit);
  setLayout(layout);
}
//------------------------------------------------------------------------------
CSongEditor::~CSongEditor()
{
  
}
//------------------------------------------------------------------------------
QString CSongEditor::filePath()
{
  return m_filePath;
}
//------------------------------------------------------------------------------
void CSongEditor::setFilePath(const QString & APath)
{
  if(QFile(APath).exists())
    m_filePath = APath;
  else
    qWarning() << "CSongEditor::setFilePath Error: the file " << APath << " does not exist " ; 
}
//------------------------------------------------------------------------------
void CSongEditor::save()
{
  // retrieve text to save
  QString text = m_textEdit->toPlainText();
  
  //open file in write mode
  QFile file(filePath());
  
  if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
      QTextStream stream (&file);
      stream << text;
      file.close();
    }
  else
    qWarning() << "Mainwindow::songEditorSave warning: unable to open file in write mode";
}
