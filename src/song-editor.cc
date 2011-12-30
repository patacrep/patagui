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

#include "song-highlighter.hh"
#include "qtfindreplacedialog/findreplacedialog.h"

#ifdef ENABLE_SPELL_CHECKING
#include "hunspell/hunspell.hxx"
#endif //ENABLE_SPELL_CHECKING

#include "code-editor.hh"
#include "song-header-editor.hh"
#include "library.hh"

#include "utils/utils.hh"

#include <QToolBar>
#include <QAction>
#include <QTextDocumentFragment>
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QFileInfo>
#include <QTextCodec>
#include <QMenu>
#include <QBoxLayout>
#include <QApplication>

#include <QDebug>

CSongEditor::CSongEditor(QWidget *parent)
  : QWidget(parent)
  , m_editor(new CodeEditor)
  , m_songHeaderEditor(0)
  , m_library()
  , m_toolBar(new QToolBar(tr("Song edition tools"), this))
  , m_path()
  , m_highlighter(0)
  , m_maxSuggestedWords(0)
  , m_song()
  , m_newSong(true)
{
  m_editor->setUndoRedoEnabled(true);

  m_songHeaderEditor = new CSongHeaderEditor(this);
  m_songHeaderEditor->setSongEditor(this);
  connect(m_songHeaderEditor, SIGNAL(contentsChanged()), SLOT(documentWasModified()));

  CHighlighter *highlighter = new CHighlighter(m_editor->document());
  Q_UNUSED(highlighter);

  connect(m_editor->document(), SIGNAL(contentsChanged()), SLOT(documentWasModified()));

  // toolBar
  toolBar()->setMovable(false);
  toolBar()->setContextMenuPolicy(Qt::PreventContextMenu);

  // actions
  QAction* action = new QAction(tr("Save"), this);
  action->setShortcut(QKeySequence::Save);
  action->setIcon(QIcon::fromTheme("document-save", QIcon(":/icons/tango/32x32/actions/document-save.png")));
  action->setStatusTip(tr("Save modifications"));
  connect(action, SIGNAL(triggered()), SLOT(save()));
  m_toolBar->addAction(action);
  
  //copy paste
  action = new QAction(tr("Cut"), this);
  action->setShortcut(QKeySequence::Cut);
  action->setIcon(QIcon::fromTheme("edit-cut", QIcon(":/icons/tango/32x32/actions/edit-cut.png")));
  action->setStatusTip(tr("Cut the selection"));
  connect(action, SIGNAL(triggered()), m_editor, SLOT(cut()));
  m_toolBar->addAction(action);
  
  action = new QAction(tr("Copy"), this);
  action->setShortcut(QKeySequence::Copy);
  action->setIcon(QIcon::fromTheme("edit-copy", QIcon(":/icons/tango/32x32/actions/edit-copy.png")));
  action->setStatusTip(tr("Copy the selection"));
  connect(action, SIGNAL(triggered()), m_editor, SLOT(copy()));
  m_toolBar->addAction(action);
  
  action = new QAction(tr("Paste"), this);
  action->setShortcut(QKeySequence::Paste);
  action->setIcon(QIcon::fromTheme("edit-paste", QIcon(":/icons/tango/32x32/actions/edit-paste.png")));
  action->setStatusTip(tr("Paste clipboard content"));
  connect(action, SIGNAL(triggered()), m_editor, SLOT(paste()));
  m_toolBar->addAction(action);
  
  toolBar()->addSeparator();
  
  //undo redo
  action = new QAction(tr("Undo"), this);
  action->setShortcut(QKeySequence::Undo);
  action->setIcon(QIcon::fromTheme("edit-undo", QIcon(":/icons/tango/32x32/actions/edit-undo.png")));
  action->setStatusTip(tr("Undo modifications"));
  connect(action, SIGNAL(triggered()), m_editor, SLOT(undo()));
  m_toolBar->addAction(action);
  
  action = new QAction(tr("Redo"), this);
  action->setShortcut(QKeySequence::Redo);
  action->setIcon(QIcon::fromTheme("edit-redo", QIcon(":/icons/tango/32x32/actions/edit-redo.png")));
  action->setStatusTip(tr("Redo modifications"));
  connect(action, SIGNAL(triggered()), m_editor, SLOT(redo()));
  m_toolBar->addAction(action);

  toolBar()->addSeparator();

  //find and replace
  m_findReplaceDialog = new FindReplaceDialog(this);
  m_findReplaceDialog->setModal(false);
  m_findReplaceDialog->setTextEdit(m_editor);

  action = new QAction(tr("Search and Replace"), this);
  action->setShortcut(QKeySequence::Find);
  action->setIcon(QIcon::fromTheme("edit-find"));//, QIcon(":/icons/tango/32x32/actions/edit-find.png")));
  action->setStatusTip(tr("Find some text and replace it"));
  connect(action, SIGNAL(triggered()), m_findReplaceDialog, SLOT(show()));
  addAction(action);

  //spellchecking
  m_spellCheckingAct = new QAction(tr("Chec&k spelling"), this);
  m_spellCheckingAct->setIcon(QIcon::fromTheme("tools-check-spelling"));//, QIcon(":/icons/tango/32x32/actions/tools-check-spelling.png")));
  m_spellCheckingAct->setStatusTip(tr("Check current song for incorrect spelling"));
  m_spellCheckingAct->setCheckable(true);
  m_spellCheckingAct->setEnabled(false);
  addAction(m_spellCheckingAct);

  toolBar()->addSeparator();

  //songbook
  action = new QAction(tr("Verse"), this);
  action->setStatusTip(tr("New verse environment"));
  connect(action, SIGNAL(triggered()), SLOT(insertVerse()));
  m_toolBar->addAction(action);
  
  action = new QAction(tr("Chorus"), this);
  action->setStatusTip(tr("New chorus environment"));
  connect(action, SIGNAL(triggered()), SLOT(insertChorus()));
  m_toolBar->addAction(action);

  QBoxLayout *mainLayout = new QVBoxLayout();
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);
  mainLayout->addWidget(m_songHeaderEditor);
  mainLayout->addWidget(m_editor);
  setLayout(mainLayout);

  hide(); // required to hide some widget from the code editor

  readSettings();
}

CSongEditor::~CSongEditor()
{
  delete m_highlighter;
}

QString CSongEditor::path()
{
  return m_song.path;
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
  m_editor->setFont(font);

  m_editor->setHighlightMode(settings.value("highlight", true).toBool());
  m_editor->setLineNumberMode(settings.value("lines", true).toBool());

#ifdef ENABLE_SPELL_CHECKING
  m_maxSuggestedWords = settings.value("maxSuggestedWords", 5).toUInt();
  for(uint i = 0; i < m_maxSuggestedWords; ++i)
    {
      QAction *action = new QAction(this);
      action->setVisible(false);
      connect(action, SIGNAL(triggered()), this, SLOT(correctWord()));
      m_misspelledWordsActs.append(action);
    }
  m_dictionary = settings.value("dictionary", "/usr/share/hunspell/en_US.dic").toString();
#endif //ENABLE_SPELL_CHECKING

  m_findReplaceDialog->readSettings(settings);

  settings.endGroup();
}

void CSongEditor::writeSettings()
{
  QSettings settings;
  m_findReplaceDialog->writeSettings(settings);
}

void CSongEditor::setPath(const QString &path)
{
  m_song = Song::fromFile(path);
  m_songHeaderEditor->update();

  QString songContent;
  foreach (QString gtab, m_song.gtabs)
    {
      songContent.append(QString("  \\gtab{%1}\n").arg(gtab));
    }

  songContent.append(QString("\n"));

  foreach (QString lyric, m_song.lyrics)
    {
      songContent.append(QString("%1\n").arg(lyric));
    }

  m_editor->setPlainText(songContent);
}

void CSongEditor::installHighlighter()
{
  m_highlighter = new CHighlighter(m_editor->document());

#ifdef ENABLE_SPELL_CHECKING
  //find a suitable dictionary based on the song's language
  QRegExp reLanguage("selectlanguage\\{([^\\}]+)");
  reLanguage.indexIn(m_editor->document()->toPlainText());
  QString lang = reLanguage.cap(1);
  if(!lang.compare("french"))
    m_dictionary = QString("/usr/share/hunspell/fr_FR.dic");
  else if(!lang.compare("english"))
    m_dictionary = QString("/usr/share/hunspell/en_US.dic");
  else if(!lang.compare("spanish"))
    m_dictionary = QString("/usr/share/hunspell/es_ES.dic");
  else
    qWarning() << "CSongbEditor::installHighlighter Unable to find dictionnary for language: " << lang;

  if(!QFile(m_dictionary).exists())
    {
      qWarning() << "CSongbEditor::installHighlighter Unable to open dictionnary: " << m_dictionary;
      return;
    }

  setSpellCheckingEnabled(true);
  m_highlighter->setDictionary(m_dictionary);
  connect(this, SIGNAL(wordAdded(const QString &)),
	  m_highlighter, SLOT(addWord(const QString &)));
  connect(m_spellCheckingAct, SIGNAL(toggled(bool)),
	  m_highlighter, SLOT(setSpellCheck(bool)));
#endif //ENABLE_SPELL_CHECKING
}

void CSongEditor::save()
{
  if (isNewSong())
    {
      createNewSong();
      return;
    }

  parseText();
  // m_song = Song::fromString(m_editor->toPlainText(), path());

  library()->addSong(m_song);
  m_editor->document()->setModified(false);
  setWindowTitle(windowTitle().remove(" *"));
  emit(labelChanged(windowTitle()));
}

void CSongEditor::parseText()
{
  QStringList lines = m_editor->toPlainText().split("\n");
  QString line;
  foreach (line, lines)
    {
      if (Song::reGtab.indexIn(line) != -1)
        {
          m_song.gtabs << Song::reGtab.cap(1);
        }
      else
        {
          m_song.lyrics << line;
        }
    }
}

void CSongEditor::saveNewSong()
{
  if (!isNewSong())
    return;

  if (m_song.title.isEmpty() || m_song.artist.isEmpty())
    {
      qDebug() << "Error: " << m_song.title << " " << m_song.artist;
      return;
    }
  else if (m_song.title == tr("*New song*") || m_song.artist == tr("Unknown"))
    {
      qDebug() << "Warning: " << m_song.title << " " << m_song.artist;
      return;
    }
  createNewSong();
}

void CSongEditor::createNewSong()
{
  if (!isNewSong())
    return;

  //add the song to the library
  library()->addSong(m_song);

  setNewSong(false);
}

void CSongEditor::documentWasModified()
{
  m_editor->document()->setModified(true);
  if (!windowTitle().contains(" *"))
    {
      setWindowTitle(windowTitle() + " *");
      emit(labelChanged(windowTitle()));
    }
}

void CSongEditor::insertVerse()
{
  QString selection = m_editor->textCursor().selectedText();
  m_editor->insertPlainText(QString("\n\\beginverse\n%1\n\\endverse\n").arg(selection)  );
}

void CSongEditor::insertChorus()
{
  QString selection = m_editor->textCursor().selectedText();
  m_editor->insertPlainText(QString("\n\\beginchorus\n%1\n\\endchorus\n").arg(selection)  );
}

QToolBar * CSongEditor::toolBar() const
{
  return m_toolBar;
}

CLibrary * CSongEditor::library() const
{
  return m_library;
}

void CSongEditor::setLibrary(CLibrary *library)
{
  m_library = library;
}

void CSongEditor::keyPressEvent(QKeyEvent *event)
{
  // if (event->key() == Qt::Key_Tab)
  //   indentSelection();
  // else 
  //   QApplication::sendEvent(m_editor, event);
}

void CSongEditor::indentSelection()
{
  QTextCursor cursor = m_editor->textCursor();
  QTextCursor it = m_editor->textCursor();
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
  //if line only contains whitespaces, remove them and exit
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

bool CSongEditor::isModified() const
{
  return m_editor->document()->isModified();
}

Song & CSongEditor::song()
{
  return m_song;
}

#ifdef ENABLE_SPELL_CHECKING
QString CSongEditor::currentWord()
{
  QTextCursor cursor = m_editor->cursorForPosition(m_lastPos);
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
      QTextCursor cursor = m_editor->cursorForPosition(m_lastPos);
      QString zeile = cursor.block().text();
      cursor.select(QTextCursor::WordUnderCursor);
      cursor.deleteChar();
      cursor.insertText(replacement);
    }
}

QStringList CSongEditor::getWordPropositions(const QString &word)
{
  if(!checker())
    return QStringList();

  QStringList wordList;
  QByteArray encodedString;
  QString spell_encoding=QString(checker()->get_dic_encoding());
  QTextCodec *codec = QTextCodec::codecForName(spell_encoding.toLatin1());
  encodedString = codec->fromUnicode(word);

  if(checker()->spell(encodedString.data()))
    return wordList;

  char ** wlst;
  int ns = checker()->suggest(&wlst, encodedString.data());
  if (ns > 0)
    {
      for (int i=0; i < ns; i++)
	wordList.append(codec->toUnicode(wlst[i]));
      checker()->free_list(&wlst, ns);
    }
  return wordList;
}

void CSongEditor::contextMenuEvent(QContextMenuEvent *event)
{
  QMenu *menu = m_editor->createStandardContextMenu();
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

bool CSongEditor::isSpellCheckingEnabled() const
{
  return m_isSpellCheckingEnabled;
}

void CSongEditor::setSpellCheckingEnabled(const bool value)
{
  m_isSpellCheckingEnabled = value;
  m_spellCheckingAct->setEnabled(value);
}
#endif //ENABLE_SPELL_CHECKING

bool CSongEditor::isNewSong() const
{
  return m_newSong;
}

void CSongEditor::setNewSong(bool newSong)
{
  m_newSong = newSong;
}
