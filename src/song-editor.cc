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

#include "song-highlighter.hh"

#ifdef ENABLE_SPELL_CHECKING
#include "hunspell/hunspell.hxx"
#endif //ENABLE_SPELL_CHECKING

#include <QToolBar>
#include <QAction>
#include <QTextDocumentFragment>
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QFileInfo>
#include <QTextCodec>
#include <QMenu>

#include <QDebug>

CSongEditor::CSongEditor()
  : CodeEditor()
  , m_toolBar(new QToolBar(tr("Song edition tools"), this))
  , m_path()
  , m_highlighter(0)
  , m_maxSuggestedWords(0)
{
  setUndoRedoEnabled(true);
  connect(document(), SIGNAL(contentsChanged()), SLOT(documentWasModified()));

  // toolBar
  toolBar()->setMovable(false);
  toolBar()->setContextMenuPolicy(Qt::PreventContextMenu);

  // actions
  QAction* action = new QAction(tr("Save"), this);
  action->setShortcut(QKeySequence::Save);
  action->setIcon(QIcon::fromTheme("document-save", QIcon(":/icons/tango/32x32/actions/document-save.png")));
  action->setStatusTip(tr("Save modifications"));
  connect(action, SIGNAL(triggered()), SLOT(save()));
  addAction(action);
  
  //copy paste
  action = new QAction(tr("Cut"), this);
  action->setShortcut(QKeySequence::Cut);
  action->setIcon(QIcon::fromTheme("edit-cut", QIcon(":/icons/tango/32x32/actions/edit-cut.png")));
  action->setStatusTip(tr("Cut the selection"));
  connect(action, SIGNAL(triggered()), SLOT(cut()));
  addAction(action);
  
  action = new QAction(tr("Copy"), this);
  action->setShortcut(QKeySequence::Copy);
  action->setIcon(QIcon::fromTheme("edit-copy", QIcon(":/icons/tango/32x32/actions/edit-copy.png")));
  action->setStatusTip(tr("Copy the selection"));
  connect(action, SIGNAL(triggered()), SLOT(copy()));
  addAction(action);
  
  action = new QAction(tr("Paste"), this);
  action->setShortcut(QKeySequence::Paste);
  action->setIcon(QIcon::fromTheme("edit-paste", QIcon(":/icons/tango/32x32/actions/edit-paste.png")));
  action->setStatusTip(tr("Paste clipboard content"));
  connect(action, SIGNAL(triggered()), SLOT(paste()));
  addAction(action);
  
  toolBar()->addSeparator();
  
  //undo redo
  action = new QAction(tr("Undo"), this);
  action->setShortcut(QKeySequence::Undo);
  action->setIcon(QIcon::fromTheme("edit-undo", QIcon(":/icons/tango/32x32/actions/edit-undo.png")));
  action->setStatusTip(tr("Undo modifications"));
  connect(action, SIGNAL(triggered()), SLOT(undo()));
  addAction(action);
  
  action = new QAction(tr("Redo"), this);
  action->setShortcut(QKeySequence::Redo);
  action->setIcon(QIcon::fromTheme("edit-redo", QIcon(":/icons/tango/32x32/actions/edit-redo.png")));
  action->setStatusTip(tr("Redo modifications"));
  connect(action, SIGNAL(triggered()), SLOT(redo()));
  addAction(action);

  toolBar()->addSeparator();
  
  //songbook
  action = new QAction(tr("Verse"), this);
  action->setStatusTip(tr("New verse environment"));
  connect(action, SIGNAL(triggered()), SLOT(insertVerse()));
  addAction(action);
  
  action = new QAction(tr("Chorus"), this);
  action->setStatusTip(tr("New chorus environment"));
  connect(action, SIGNAL(triggered()), SLOT(insertChorus()));
  addAction(action);

  readSettings();
}

CSongEditor::~CSongEditor()
{
  delete m_highlighter;
}

QString CSongEditor::path()
{
  return m_path;
}

void CSongEditor::readSettings()
{
  QSettings settings;
  settings.beginGroup("editor");

  QFont font;
  QString fontstr = settings.value("font", QString()).toString();
  if(fontstr.isEmpty())
    {
      font = QFont("Monospace",11);
      font.setStyleHint(QFont::TypeWriter, QFont::PreferAntialias);
    }

  font.fromString(fontstr);
  setFont(font);

  setHighlightMode(settings.value("highlight", true).toBool());
  setLineNumberMode(settings.value("lines", true).toBool());

#ifdef ENABLE_SPELL_CHECKING
  m_maxSuggestedWords = settings.value("maxSuggestedWords", 5).toUInt();
  for(uint i = 0; i < m_maxSuggestedWords; ++i)
    {
      QAction *action = new QAction(this);
      action->setVisible(false);
      connect(action, SIGNAL(triggered()), this, SLOT(correctWord()));
      m_misspelledWordsActs.append(action);
    }
#endif //ENABLE_SPELL_CHECKING

  settings.endGroup();
}

void CSongEditor::writeSettings()
{}

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

void CSongEditor::installHighlighter()
{
  m_highlighter = new CHighlighter(document());

#ifdef ENABLE_SPELL_CHECKING
  m_highlighter->setSpellCheck(true);
  m_highlighter->setDictionary("/usr/share/hunspell/en_US.dic");
  connect(this, SIGNAL(wordAdded(const QString &)),
	  m_highlighter, SLOT(addWord(const QString &)));
#endif //ENABLE_SPELL_CHECKING
}

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

void CSongEditor::documentWasModified()
{
  if (!windowTitle().contains(" *") && document()->isModified())
    {
      setWindowTitle(windowTitle() + " *");
      emit(labelChanged(windowTitle()));
    }
}

void CSongEditor::insertVerse()
{
  QString selection = textCursor().selectedText();
  insertPlainText(QString("\n\\beginverse\n%1\n\\endverse\n").arg(selection)  );
}

void CSongEditor::insertChorus()
{
  QString selection = textCursor().selectedText();
  insertPlainText(QString("\n\\beginchorus\n%1\n\\endchorus\n").arg(selection)  );
}

QToolBar* CSongEditor::toolBar()
{
  return m_toolBar;
}

void CSongEditor::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Tab) 
    indentSelection();
  else 
    QPlainTextEdit::keyPressEvent(event);
}

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

QList<QAction*> CSongEditor::actions() const
{
  return m_actions;
}

void CSongEditor::addAction(QAction* action)
{
  toolBar()->addAction(action);
  m_actions.append(action);
}


#ifdef ENABLE_SPELL_CHECKING
QString CSongEditor::currentWord()
{
  QTextCursor cursor = cursorForPosition(m_lastPos);
  QString word = cursor.block().text();
  int pos = cursor.columnNumber();
  int end = word.indexOf(QRegExp("\\W+"),pos);
  int begin = word.left(pos).lastIndexOf(QRegExp("\\W+"),pos);
  word = word.mid(begin+1,end-begin-1);
  return word;
}

void CSongEditor::correctWord() 
{
  QAction *action = qobject_cast<QAction *>(sender());
  if (action)
    {
      QString replacement = action->text();
      QTextCursor cursor = cursorForPosition(m_lastPos);
      QString zeile = cursor.block().text();
      cursor.select(QTextCursor::WordUnderCursor);
      cursor.deleteChar();
      cursor.insertText(replacement);
    }
}

QStringList CSongEditor::getWordPropositions(const QString &word)
{
  QStringList wordList;
  if(checker())
    {
      QByteArray encodedString;
      QString spell_encoding=QString(checker()->get_dic_encoding());
      QTextCodec *codec = QTextCodec::codecForName(spell_encoding.toLatin1());
      encodedString = codec->fromUnicode(word);
      bool check = checker()->spell(encodedString.data());
      if(!check){
	char ** wlst;
	int ns = checker()->suggest(&wlst,encodedString.data());
	if (ns > 0)
	  {
	    for (int i=0; i < ns; i++)
	      {
		wordList.append(codec->toUnicode(wlst[i]));
		//free(wlst[i]);
	      }
	    //free(wlst);
	    checker()->free_list(&wlst, ns);
	  }// if ns >0
      }// if check
    }
  return wordList;
}

void CSongEditor::contextMenuEvent(QContextMenuEvent *event)
{
  QMenu *menu = createStandardContextMenu();
  QMenu *spellMenu = new QMenu(tr("Suggestions"));
  m_lastPos=event->pos();
  QString str = currentWord();
  QStringList list = getWordPropositions(str);
  int size = qMin(m_maxSuggestedWords, (uint)list.size());
  if (!list.isEmpty())
    {
      for (int i = 0; i < size; ++i)
	{
	  m_misspelledWordsActs[i]->setText(list[i].trimmed());
	  m_misspelledWordsActs[i]->setVisible(true);
	  spellMenu->addAction(m_misspelledWordsActs[i]);
	}
      spellMenu->addSeparator();
      spellMenu->addAction(tr("Add"), this, SLOT(addWord()));
      spellMenu->addAction(tr("Ignore"), this, SLOT(ignoreWord()));
      menu->addMenu(spellMenu);
    }
  menu->exec(event->globalPos());
  delete menu;
}

void CSongEditor::ignoreWord()
{
  QString str = currentWord();
  QByteArray encodedString;
  QString spell_encoding=QString(checker()->get_dic_encoding());
  QTextCodec *codec = QTextCodec::codecForName(spell_encoding.toLatin1());
  encodedString = codec->fromUnicode(str);
  checker()->add(encodedString.data());
  emit wordAdded(str);
}

void CSongEditor::addWord()
{
  QString str = currentWord();
  QByteArray encodedString;
  QString spell_encoding=QString(checker()->get_dic_encoding());
  QTextCodec *codec = QTextCodec::codecForName(spell_encoding.toLatin1());
  encodedString = codec->fromUnicode(str);
  checker()->add(encodedString.data());
  m_addedWords.append(str);
  emit wordAdded(str);
}

Hunspell* CSongEditor::checker() const
{
  if(!m_highlighter) return 0;
  return m_highlighter->checker();
}
#endif //ENABLE_SPELL_CHECKING

