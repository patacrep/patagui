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
#include <QTextDocumentFragment>
#include <QLayout>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDebug>
#include "code-editor.hh"

//------------------------------------------------------------------------------
CSongEditor::CSongEditor(const QString & APath)
{
  m_filePath = APath;

  // toolbar
  QToolBar* toolbar = new QToolBar(this);
  toolbar->setMovable(false);

  //retrieve song text
  m_textEdit = new CodeEditor(this);
  m_textEdit->setUndoRedoEnabled(true);
  QFile file(APath);
  if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      QTextStream stream (&file);
      QString text = stream.readAll();
      file.close();
      m_textEdit->setPlainText(text);
      new CHighlighter(m_textEdit->document());

      connect(m_textEdit->document(), SIGNAL(contentsChanged()), this, SLOT(documentWasModified()));

      // actions
      QAction* action = new QAction(tr("Save"), this);
      action->setShortcut(QKeySequence::Save);
#if QT_VERSION >= 0x040600
      action->setIcon(QIcon::fromTheme("document-save"));
#endif
      action->setStatusTip(tr("Save modifications"));
      connect(action, SIGNAL(triggered()), this, SLOT(save()));
      toolbar->addAction(action);

      toolbar->addSeparator();

      //copy paste
      action = new QAction(tr("Cut"), this);
      action->setShortcut(QKeySequence::Cut);
#if QT_VERSION >= 0x040600
      action->setIcon(QIcon::fromTheme("edit-cut"));
#endif
      action->setStatusTip(tr("Cut the selection"));
      connect(action, SIGNAL(triggered()), m_textEdit, SLOT(cut()));
      toolbar->addAction(action);

      action = new QAction(tr("Copy"), this);
      action->setShortcut(QKeySequence::Copy);
#if QT_VERSION >= 0x040600
      action->setIcon(QIcon::fromTheme("edit-copy"));
#endif
      action->setStatusTip(tr("Copy the selection"));
      connect(action, SIGNAL(triggered()), m_textEdit, SLOT(copy()));
      toolbar->addAction(action);

      action = new QAction(tr("Paste"), this);
      action->setShortcut(QKeySequence::Paste);
#if QT_VERSION >= 0x040600
      action->setIcon(QIcon::fromTheme("edit-paste"));
#endif
      action->setStatusTip(tr("Paste clipboard content"));
      connect(action, SIGNAL(triggered()), m_textEdit, SLOT(paste()));
      toolbar->addAction(action);

      toolbar->addSeparator();

      //undo redo
      action = new QAction(tr("Undo"), this);
      action->setShortcut(QKeySequence::Undo);
#if QT_VERSION >= 0x040600
      action->setIcon(QIcon::fromTheme("edit-undo"));
#endif
      action->setStatusTip(tr("Undo modifications"));
      connect(action, SIGNAL(triggered()), m_textEdit, SLOT(undo()));
      toolbar->addAction(action);

      action = new QAction(tr("Redo"), this);
      action->setShortcut(QKeySequence::Redo);
#if QT_VERSION >= 0x040600
      action->setIcon(QIcon::fromTheme("edit-redo"));
#endif
      action->setStatusTip(tr("Redo modifications"));
      connect(action, SIGNAL(triggered()), m_textEdit, SLOT(redo()));
      toolbar->addAction(action);

      toolbar->addSeparator();

      //songbook
      action = new QAction(tr("Verse"), this);
      action->setStatusTip(tr("New verse environment"));
      connect(action, SIGNAL(triggered()), this, SLOT(insertVerse()));
      toolbar->addAction(action);

      action = new QAction(tr("Chorus"), this);
      action->setStatusTip(tr("New chorus environment"));
      connect(action, SIGNAL(triggered()), this, SLOT(insertChorus()));
      toolbar->addAction(action);

      QBoxLayout* layout = new QVBoxLayout;
      layout->addWidget(toolbar);
      layout->addWidget(m_textEdit);
      setLayout(layout);
      isOk = true;
    }
  else
    {
      isOk = false;
      QMessageBox msgBox;
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.setText(QString(tr("Unable to open file:\n%1")).arg(APath));
      msgBox.setStandardButtons(QMessageBox::Cancel);
      msgBox.setDefaultButton(QMessageBox::Cancel);
      msgBox.exec();
    }
}
//------------------------------------------------------------------------------
CSongEditor::~CSongEditor()
{
  delete m_textEdit;
}
//------------------------------------------------------------------------------
QString CSongEditor::filePath()
{
  return m_filePath;
}
//------------------------------------------------------------------------------
void CSongEditor::setFilePath(const QString & APath)
{
  if (QFile(APath).exists())
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
  //add isModified also it seems strange to avoid document marked as
  //modified the first time it is opened
  if ( !label().contains(" *") &&   m_textEdit->document()->isModified() )
    setLabel(label() + " *");
}
//------------------------------------------------------------------------------
void CSongEditor::insertVerse()
{
  QString selection = m_textEdit->textCursor().selectedText();
  m_textEdit->insertPlainText(QString("\n\\beginverse\n%1\n\\endverse\n").arg(selection)  );
}
//------------------------------------------------------------------------------
void CSongEditor::insertChorus()
{
  QString selection = m_textEdit->textCursor().selectedText();
  m_textEdit->insertPlainText(QString("\n\\beginchorus\n%1\n\\endchorus\n").arg(selection)  );
}
