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
#include "highlighter.hh"
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
  m_textEdit->setUndoRedoEnabled(true);
  QFile file(APath);
  if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {      
      QTextStream stream (&file);
      QString text = stream.readAll();
      file.close();
      m_textEdit->setText(text);
      new Highlighter(m_textEdit->document()); 
    }
  else
    {
      qWarning() << "CSongEditor warning: unable to open file in read mode";
    }

  connect(m_textEdit->document(), SIGNAL(contentsChanged()), this, SLOT(documentWasModified()));

  //undo redo
  action = new QAction(tr("Undo"), this);
  action->setShortcut(tr("Ctrl+Z"));
  action->setStatusTip(tr("Undo modifications"));
  connect(action, SIGNAL(triggered()), m_textEdit, SLOT(undo()));
  toolbar->addAction(action);

  action = new QAction(tr("Redo"), this);
  action->setStatusTip(tr("Redo modifications"));
  connect(action, SIGNAL(triggered()), m_textEdit, SLOT(redo()));
  toolbar->addAction(action);
 
  QBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(toolbar);
  layout->addWidget(m_textEdit);
  setLayout(layout);
}
//------------------------------------------------------------------------------
CSongEditor::~CSongEditor()
{}
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
int CSongEditor::tabIndex()
{
  return m_tabIndex;
}
//------------------------------------------------------------------------------
void CSongEditor::setTabIndex(int AIndex)
{
  m_tabIndex = AIndex;
}
//------------------------------------------------------------------------------
QString CSongEditor::label()
{
  return m_label;
}
//------------------------------------------------------------------------------
void CSongEditor::setLabel(const QString & ALabel)
{
  m_label = ALabel;
  emit( labelChanged() );
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
      setLabel( label().remove(" *") );
    }
  else
    qWarning() << "Mainwindow::songEditorSave warning: unable to open file in write mode";
}
//------------------------------------------------------------------------------
void CSongEditor::documentWasModified()
{
  if( !label().contains(" *"))
    setLabel(label() + " *");
}
