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
  , m_songEditor(0)
  , m_songHeaderEditor(0)
  , m_library(0)
  , m_toolBar(0)
  , m_highlighter(0)
  , m_maxSuggestedWords(0)
  , m_song()
  , m_newSong(true)
{
  m_songEditor = new CodeEditor();
  m_songEditor->setUndoRedoEnabled(true);

  CHighlighter *highlighter = new CHighlighter(m_songEditor->document());
  Q_UNUSED(highlighter);
  connect(m_songEditor->document(), SIGNAL(contentsChanged()), SLOT(documentWasModified()));

  m_songHeaderEditor = new CSongHeaderEditor(this);
  m_songHeaderEditor->setSongEditor(this);
  connect(m_songHeaderEditor, SIGNAL(contentsChanged()), SLOT(documentWasModified()));

  // toolBar
  m_toolBar = new QToolBar(tr("Song edition tools"), this);
  m_toolBar->setMovable(false);
  m_toolBar->setContextMenuPolicy(Qt::PreventContextMenu);

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
  connect(action, SIGNAL(triggered()), m_songEditor, SLOT(cut()));
  m_toolBar->addAction(action);
  
  action = new QAction(tr("Copy"), this);
  action->setShortcut(QKeySequence::Copy);
  action->setIcon(QIcon::fromTheme("edit-copy", QIcon(":/icons/tango/32x32/actions/edit-copy.png")));
  action->setStatusTip(tr("Copy the selection"));
  connect(action, SIGNAL(triggered()), m_songEditor, SLOT(copy()));
  m_toolBar->addAction(action);
  
  action = new QAction(tr("Paste"), this);
  action->setShortcut(QKeySequence::Paste);
  action->setIcon(QIcon::fromTheme("edit-paste", QIcon(":/icons/tango/32x32/actions/edit-paste.png")));
  action->setStatusTip(tr("Paste clipboard content"));
  connect(action, SIGNAL(triggered()), m_songEditor, SLOT(paste()));
  m_toolBar->addAction(action);
  
  toolBar()->addSeparator();
  
  //undo redo
  action = new QAction(tr("Undo"), this);
  action->setShortcut(QKeySequence::Undo);
  action->setIcon(QIcon::fromTheme("edit-undo", QIcon(":/icons/tango/32x32/actions/edit-undo.png")));
  action->setStatusTip(tr("Undo modifications"));
  connect(action, SIGNAL(triggered()), m_songEditor, SLOT(undo()));
  m_toolBar->addAction(action);
  
  action = new QAction(tr("Redo"), this);
  action->setShortcut(QKeySequence::Redo);
  action->setIcon(QIcon::fromTheme("edit-redo", QIcon(":/icons/tango/32x32/actions/edit-redo.png")));
  action->setStatusTip(tr("Redo modifications"));
  connect(action, SIGNAL(triggered()), m_songEditor, SLOT(redo()));
  m_toolBar->addAction(action);

  toolBar()->addSeparator();

  //find and replace
  m_findReplaceDialog = new FindReplaceDialog(this);
  m_findReplaceDialog->setModal(false);
  m_findReplaceDialog->setTextEdit(m_songEditor);

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
  mainLayout->addWidget(m_songEditor);
  setLayout(mainLayout);

  hide(); // required to hide some widgets from the code editor

  // the editor is set for new song by default
  setWindowTitle("New song");
  setNewSong(true);

  readSettings();
}

CSongEditor::~CSongEditor()
{
  delete m_highlighter;
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
  m_songEditor->setFont(font);

  m_songEditor->setHighlightMode(settings.value("highlight", true).toBool());
  m_songEditor->setLineNumberMode(settings.value("lines", true).toBool());

#ifdef ENABLE_SPELL_CHECKING
  m_maxSuggestedWords = settings.value("maxSuggestedWords", 5).toUInt();
  for(uint i = 0; i < m_maxSuggestedWords; ++i)
    {
      QAction *action = new QAction(this);
      action->setVisible(false);
      connect(action, SIGNAL(triggered()), this, SLOT(correctWord()));
      m_misspelledWordsActs.append(action);
    }
  // m_dictionary = settings.value("dictionary", "/usr/share/hunspell/en_US.dic").toString();
#endif //ENABLE_SPELL_CHECKING

  m_findReplaceDialog->readSettings(settings);

  settings.endGroup();
}

void CSongEditor::writeSettings()
{
  QSettings settings;
  m_findReplaceDialog->writeSettings(settings);
}

void CSongEditor::installHighlighter()
{
  m_highlighter = new CHighlighter(m_songEditor->document());
}

#ifdef ENABLE_SPELL_CHECKING
void CSongEditor::setDictionary(QLocale::Language language)
{
  if (m_highlighter == 0)
    return;

  // find the suitable dictionary based on the current song language
  QString dictionary;
  switch (language)
    {
    case QLocale::French:
      dictionary = "/usr/share/hunspell/fr_FR.dic";
      break;
    case QLocale::English:
      dictionary = "/usr/share/hunspell/en_US.dic";
      break;
    case QLocale::Spanish:
      dictionary = "/usr/share/hunspell/es_ES.dic";
      break;
    default:
      qWarning() << "Unable to find dictionnary for " << QLocale::languageToString(language);
      break;
    }

  if (!QFile(dictionary).exists())
    {
      qWarning() << "Unable to open the dictionnary: " << dictionary;
      return;
    }

  setSpellCheckingEnabled(true);
  m_highlighter->setDictionary(dictionary);
  connect(this, SIGNAL(wordAdded(const QString&)), m_highlighter, SLOT(addWord(const QString&)));
  connect(m_spellCheckingAct, SIGNAL(toggled(bool)), m_highlighter, SLOT(setSpellCheck(bool)));
}
#endif //ENABLE_SPELL_CHECKING

void CSongEditor::save()
{
  // get the song contents
  parseText();

  // save the song and add it to the library list
  library()->saveSong(m_song);
  library()->removeSong(m_song.path);
  library()->addSong(m_song);

  setNewSong(false);
  setModified(false);
  setWindowTitle(m_song.title);
  emit(labelChanged(windowTitle()));
}

void CSongEditor::parseText()
{
  m_song.gtabs.clear();
  m_song.lyrics.clear();
  QStringList lines = m_songEditor->toPlainText().split("\n");
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
  setModified(true);
}

void CSongEditor::insertVerse()
{
  QString selection = m_songEditor->textCursor().selectedText();
  m_songEditor->insertPlainText(QString("\n\\beginverse\n%1\n\\endverse\n").arg(selection)  );
}

void CSongEditor::insertChorus()
{
  QString selection = m_songEditor->textCursor().selectedText();
  m_songEditor->insertPlainText(QString("\n\\beginchorus\n%1\n\\endchorus\n").arg(selection)  );
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
  //   QApplication::sendEvent(m_songEditor, event);
}

void CSongEditor::indentSelection()
{
  QTextCursor cursor = m_songEditor->textCursor();
  QTextCursor it = m_songEditor->textCursor();
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
  return m_songEditor->document()->isModified();
}

void CSongEditor::setModified(bool modified)
{
  if (m_songEditor->document()->isModified() != modified)
    m_songEditor->document()->setModified(modified);

  // update the window title
  if (modified && !windowTitle().contains(" *"))
    {
      setWindowTitle(windowTitle() + " *");
      emit(labelChanged(windowTitle()));
    }
  else if (!modified && windowTitle().contains(" *"))
    {
      setWindowTitle(windowTitle().remove(" *"));
      emit(labelChanged(windowTitle()));
    }
}

Song & CSongEditor::song()
{
  return m_song;
}

void CSongEditor::setSong(const Song &song)
{
  m_song = song;

  // update the header editor
  m_songHeaderEditor->update();

  // update the text editor
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

  m_songEditor->setPlainText(songContent);


#ifdef ENABLE_SPELL_CHECKING
  setDictionary(song.language);
#endif //ENABLE_SPELL_CHECKING

  setNewSong(false);
  setWindowTitle(m_song.title);
  setModified(false);
}

bool CSongEditor::isNewSong() const
{
  return m_newSong;
}

void CSongEditor::setNewSong(bool newSong)
{
  m_newSong = newSong;
}

#ifdef ENABLE_SPELL_CHECKING
QString CSongEditor::currentWord()
{
  QTextCursor cursor = m_songEditor->cursorForPosition(m_lastPos);
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
      QTextCursor cursor = m_songEditor->cursorForPosition(m_lastPos);
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
  QMenu *menu = m_songEditor->createStandardContextMenu();
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
