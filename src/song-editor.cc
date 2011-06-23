// Copyright (C) 2009-2011, Romain Goffe <romain.goffe@gmail.com>
// Copyright (C) 2009-2011, Alexandre Dupas <alexandre.dupas@gmail.com>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301, USA.
//******************************************************************************
#include "song-editor.hh"
#include "code-editor.hh"

#include "highlighter.hh"

#include <QToolBar>
#include <QAction>
#include <QTextDocumentFragment>
#include <QFile>
#include <QTextStream>

#include <QDebug>

//------------------------------------------------------------------------------
CSongEditor::CSongEditor()
  : CodeEditor()
  , m_toolBar()
  , m_path()
{
  setUndoRedoEnabled(true);

  CHighlighter *highlighter = new CHighlighter(document());
  Q_UNUSED(highlighter);

  connect(document(), SIGNAL(contentsChanged()), SLOT(documentWasModified()));

  // toolBar
  m_toolBar = new QToolBar(tr("Song edition tools"), this);
  m_toolBar->setMovable(false);
  m_toolBar->setContextMenuPolicy(Qt::PreventContextMenu);

  // actions
  QAction* action = new QAction(tr("Save"), this);
  action->setShortcut(QKeySequence::Save);
  action->setIcon(QIcon::fromTheme("document-save", QIcon(":/icons/tango/document-save")));
  action->setStatusTip(tr("Save modifications"));
  connect(action, SIGNAL(triggered()), SLOT(save()));
  m_toolBar->addAction(action);
  
  m_toolBar->addSeparator();
  
  //copy paste
  action = new QAction(tr("Cut"), this);
  action->setShortcut(QKeySequence::Cut);
  action->setIcon(QIcon::fromTheme("edit-cut", QIcon(":/icons/tango/edit-cut")));
  action->setStatusTip(tr("Cut the selection"));
  connect(action, SIGNAL(triggered()), SLOT(cut()));
  m_toolBar->addAction(action);
  
  action = new QAction(tr("Copy"), this);
  action->setShortcut(QKeySequence::Copy);
  action->setIcon(QIcon::fromTheme("edit-copy", QIcon(":/icons/tango/edit-copy")));
  action->setStatusTip(tr("Copy the selection"));
  connect(action, SIGNAL(triggered()), SLOT(copy()));
  m_toolBar->addAction(action);
  
  action = new QAction(tr("Paste"), this);
  action->setShortcut(QKeySequence::Paste);
  action->setIcon(QIcon::fromTheme("edit-paste", QIcon(":/icons/tango/edit-paste")));
  action->setStatusTip(tr("Paste clipboard content"));
  connect(action, SIGNAL(triggered()), SLOT(paste()));
  m_toolBar->addAction(action);
  
  m_toolBar->addSeparator();
  
  //undo redo
  action = new QAction(tr("Undo"), this);
  action->setShortcut(QKeySequence::Undo);
  action->setIcon(QIcon::fromTheme("edit-undo", QIcon(":/icons/tango/edit-undo")));
  action->setStatusTip(tr("Undo modifications"));
  connect(action, SIGNAL(triggered()), SLOT(undo()));
  m_toolBar->addAction(action);
  
  action = new QAction(tr("Redo"), this);
  action->setShortcut(QKeySequence::Redo);
  action->setIcon(QIcon::fromTheme("edit-redo", QIcon(":/icons/tango/edit-redo")));
  action->setStatusTip(tr("Redo modifications"));
  connect(action, SIGNAL(triggered()), SLOT(redo()));
  m_toolBar->addAction(action);

  m_toolBar->addSeparator();
  
  //songbook
  action = new QAction(tr("Verse"), this);
  action->setStatusTip(tr("New verse environment"));
  connect(action, SIGNAL(triggered()), SLOT(insertVerse()));
  m_toolBar->addAction(action);
  
  action = new QAction(tr("Chorus"), this);
  action->setStatusTip(tr("New chorus environment"));
  connect(action, SIGNAL(triggered()), SLOT(insertChorus()));
  m_toolBar->addAction(action);
}
//------------------------------------------------------------------------------
CSongEditor::~CSongEditor()
{}
//------------------------------------------------------------------------------
QString CSongEditor::path()
{
  return m_path;
}
//------------------------------------------------------------------------------
void CSongEditor::setPath(const QString &path)
{
  QString text;
  QFile file(path);
  if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      QTextStream stream (&file);
      stream.setCodec("UTF-8");
      text = stream.readAll();
      file.close();
    }
  setPlainText(text);
  m_path = path;
}
//------------------------------------------------------------------------------
void CSongEditor::save()
{
  //open file in write mode
  QFile file(path());
  if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
      QTextStream stream (&file);
      stream.setCodec("UTF-8");
      stream << toPlainText();
      file.close();
      document()->setModified(false);
      setWindowTitle(windowTitle().remove(" *"));
      emit(labelChanged(windowTitle()));
    }
  else
    {
      qWarning() << "Mainwindow::songEditorSave warning: unable to open file in write mode";
    }
}
//------------------------------------------------------------------------------
void CSongEditor::documentWasModified()
{
  if (!windowTitle().contains(" *") && document()->isModified())
    {
      setWindowTitle(windowTitle() + " *");
      emit(labelChanged(windowTitle()));
    }
}
//------------------------------------------------------------------------------
void CSongEditor::insertVerse()
{
  QString selection = textCursor().selectedText();
  insertPlainText(QString("\n\\beginverse\n%1\n\\endverse\n").arg(selection)  );
}
//------------------------------------------------------------------------------
void CSongEditor::insertChorus()
{
  QString selection = textCursor().selectedText();
  insertPlainText(QString("\n\\beginchorus\n%1\n\\endchorus\n").arg(selection)  );
}
//------------------------------------------------------------------------------
QToolBar* CSongEditor::toolBar()
{
  return m_toolBar;
}
//------------------------------------------------------------------------------
void CSongEditor::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Tab) 
    indentSelection();
  else 
    QPlainTextEdit::keyPressEvent(event);
}
//------------------------------------------------------------------------------
void CSongEditor::indentSelection()
{
  QTextCursor cursor = textCursor();
  QTextCursor it = textCursor();
  it.setPosition(cursor.anchor());

  //swap such as it always points
  //to the beginning of the selection 
  if(it > cursor)
    {
      it.setPosition(cursor.position());
      cursor.setPosition(cursor.anchor());
    }

  it.movePosition(QTextCursor::StartOfLine);
  while(it <= cursor)
    {
      indentLine(it);
      it.movePosition(QTextCursor::EndOfLine);
      if(!it.atEnd())
	it.movePosition(QTextCursor::Down);
      else
	break;
    }
}
//------------------------------------------------------------------------------
void CSongEditor::indentLine(const QTextCursor & cur)
{
  //if line is only contains whitespaces, remove them and exit
  if(cur.block().text().trimmed().isEmpty() || cur.atStart())
    {
      trimLine(cur);
      return;
    }

  //get the previous non void line
  QTextCursor cursor(cur);
  QString prevLine;
  do
    {
      cursor.movePosition(QTextCursor::Up);
      prevLine = cursor.block().text();
    }
  while(cursor.block().text().trimmed().isEmpty());

  //deduce column from previous line
  int spaces = 0;
  while(prevLine.startsWith(" "))
    {
      prevLine.remove(0,1);
      ++spaces;
    }
  int index = spaces/2;

  //add indentation level if previous line begins with \begin
  if(prevLine.startsWith("\\begin"))
    ++index;

  cursor = cur;
  cursor.movePosition (QTextCursor::StartOfLine);
  //remove indentation level if current line begins with \end
  if(cursor.block().text().contains("\\end") && index!=0)
    --index;
  
  //performs the correct indentation
  trimLine(cursor);
  for(int i=0; i < index; ++i)
    cursor.insertText("  ");
}
//------------------------------------------------------------------------------
void CSongEditor::trimLine(const QTextCursor & cur)
{
  QTextCursor cursor(cur);
  QString str  = cursor.block().text();
  while( str.startsWith(" ") )
    {
      cursor.movePosition (QTextCursor::StartOfLine);
      cursor.deleteChar();
      str  = cursor.block().text();
    }
}
