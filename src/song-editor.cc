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
#include "hunspell/hunspell.hxx"

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
{
  setUndoRedoEnabled(true);

  //Spell-Checking
  QString SpellDic = "./src/hunspell/dictionaries/en_GB.dic";
  // create misspell actions in context menu
  spell_dic = SpellDic.left(SpellDic.length()-4);
  pChecker  = new Hunspell(spell_dic.toLatin1()+".aff",spell_dic.toLatin1()+".dic");
  
  QFileInfo fi(SpellDic);
  if (!(fi.exists() && fi.isReadable())){
    delete pChecker;
    pChecker=0;
  }

  // get user config dictionary
  QSettings setting;
  QString filePath = QFileInfo(setting.fileName()).absoluteFilePath();
  filePath = filePath + "/User_" + QFileInfo(spell_dic.toLatin1()+".dic").fileName();
  qDebug() << qPrintable(filePath);
  fi = QFileInfo(filePath);
  if (fi.exists() && fi.isReadable()){
    pChecker->add_dic(filePath.toLatin1());
  }
  else filePath="";

  addedWords.clear();

  CHighlighter *highlighter = new CHighlighter(document());
  highlighter->setSpellCheck(true);
  highlighter->setDict(SpellDic);
  
  connect(this, SIGNAL(addWord(const QString &)),
	  highlighter, SLOT(slot_addWord(const QString &)));

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

  for (int i = 0; i < MaxWords; ++i) 
    {
      misspelledWordsActs[i] = new QAction(this);
      misspelledWordsActs[i]->setVisible(false);
      connect(misspelledWordsActs[i], SIGNAL(triggered()), this, SLOT(correctWord()));
    }

  readSettings();
}

CSongEditor::~CSongEditor()
{}

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

QString CSongEditor::currentWord()
{
  QTextCursor cursor = cursorForPosition(lastPos);
  QString zeile = cursor.block().text();
  int pos = cursor.columnNumber();
  int end = zeile.indexOf(QRegExp("\\W+"),pos);
  int begin = zeile.left(pos).lastIndexOf(QRegExp("\\W+"),pos);
  zeile=zeile.mid(begin+1,end-begin-1);
  return zeile;
}

void CSongEditor::correctWord() 
{
  QAction *action = qobject_cast<QAction *>(sender());
  if (action)
    {
      QString replacement = action->text();
      QTextCursor cursor = cursorForPosition(lastPos);
      QString zeile = cursor.block().text();
      cursor.select(QTextCursor::WordUnderCursor);
      cursor.deleteChar();
      cursor.insertText(replacement);
    }
}

QStringList CSongEditor::getWordPropositions(const QString &word)
{
  QStringList wordList;
  if(pChecker){
    QByteArray encodedString;
    QString spell_encoding=QString(pChecker->get_dic_encoding());
    QTextCodec *codec = QTextCodec::codecForName(spell_encoding.toLatin1());
    encodedString = codec->fromUnicode(word);
    bool check = pChecker->spell(encodedString.data());
    if(!check){
      char ** wlst;
      int ns = pChecker->suggest(&wlst,encodedString.data());
      if (ns > 0)
	{
	  for (int i=0; i < ns; i++)
	    {
	      wordList.append(codec->toUnicode(wlst[i]));
	      //free(wlst[i]);
	    }
	  //free(wlst);
	  pChecker->free_list(&wlst, ns);
	}// if ns >0
    }// if check
  }
  return wordList;
}

void CSongEditor::contextMenuEvent(QContextMenuEvent *event)
{
  QMenu *menu = createStandardContextMenu();
  lastPos=event->pos();
  QString str = currentWord();
  QStringList liste = getWordPropositions(str);
  if (!liste.isEmpty())
    {
      menu->addSeparator();
      menu->addAction(tr("Add"), this, SLOT(slot_addWord()));
      menu->addAction(tr("Ignore"), this, SLOT(slot_ignoreWord()));
      for (int i = 0; i < qMin(int(MaxWords),liste.size()); ++i) {
	misspelledWordsActs[i]->setText(liste.at(i).trimmed());
	misspelledWordsActs[i]->setVisible(true);
	menu->addAction(misspelledWordsActs[i]);
      }

    } // if  misspelled
  menu->exec(event->globalPos());
  delete menu;
}

void CSongEditor::slot_ignoreWord()
{
  QString str = currentWord();
  QByteArray encodedString;
  QString spell_encoding=QString(pChecker->get_dic_encoding());
  QTextCodec *codec = QTextCodec::codecForName(spell_encoding.toLatin1());
  encodedString = codec->fromUnicode(str);
  pChecker->add(encodedString.data());
  emit addWord(str);
}

void CSongEditor::slot_addWord()
{
  QString str = currentWord();
  QByteArray encodedString;
  QString spell_encoding=QString(pChecker->get_dic_encoding());
  QTextCodec *codec = QTextCodec::codecForName(spell_encoding.toLatin1());
  encodedString = codec->fromUnicode(str);
  pChecker->add(encodedString.data());
  addedWords.append(str);
  emit addWord(str);
}
