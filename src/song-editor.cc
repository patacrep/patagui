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

#include "find-replace-dialog.hh"
#include "song-header-editor.hh"
#include "song-code-editor.hh"
#include "library.hh"
#include "utils/lineedit.hh"

#include <QFile>
#include <QToolBar>
#include <QAction>
#include <QActionGroup>
#include <QTextCodec>
#include <QSettings>
#include <QBoxLayout>
#include <QMessageBox>

#include <QDebug>

CSongEditor::CSongEditor(QWidget *parent)
  : QWidget(parent)
  , m_codeEditor(0)
  , m_songHeaderEditor(0)
  , m_library(0)
  , m_toolBar(0)
  , m_actions(new QActionGroup(this))
  , m_spellCheckingAct(new QAction(this))
  , m_song()
  , m_newSong(true)
  , m_newCover(false)
{
  m_codeEditor = new CSongCodeEditor(this);
  connect(m_codeEditor, SIGNAL(textChanged()), SLOT(documentWasModified()));

  m_songHeaderEditor = new CSongHeaderEditor(this);
  m_songHeaderEditor->setSongEditor(this);
  connect(m_songHeaderEditor, SIGNAL(contentsChanged()), SLOT(documentWasModified()));
#ifdef ENABLE_SPELLCHECK
  connect(m_songHeaderEditor, SIGNAL(languageChanged(const QLocale &)),
	  SLOT(setDictionary(const QLocale &)));
#endif //ENABLE_SPELLCHECK

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
  m_actions->addAction(action);
  toolBar()->addAction(action);

  //copy paste
  action = new QAction(tr("Cut"), this);
  action->setShortcut(QKeySequence::Cut);
  action->setIcon(QIcon::fromTheme("edit-cut", QIcon(":/icons/tango/32x32/actions/edit-cut.png")));
  action->setStatusTip(tr("Cut the selection"));
  connect(action, SIGNAL(triggered()), codeEditor(), SLOT(cut()));
  m_actions->addAction(action);
  toolBar()->addAction(action);

  action = new QAction(tr("Copy"), this);
  action->setShortcut(QKeySequence::Copy);
  action->setIcon(QIcon::fromTheme("edit-copy", QIcon(":/icons/tango/32x32/actions/edit-copy.png")));
  action->setStatusTip(tr("Copy the selection"));
  connect(action, SIGNAL(triggered()), codeEditor(), SLOT(copy()));
  m_actions->addAction(action);
  toolBar()->addAction(action);

  action = new QAction(tr("Paste"), this);
  action->setShortcut(QKeySequence::Paste);
  action->setIcon(QIcon::fromTheme("edit-paste", QIcon(":/icons/tango/32x32/actions/edit-paste.png")));
  action->setStatusTip(tr("Paste clipboard content"));
  connect(action, SIGNAL(triggered()), codeEditor(), SLOT(paste()));
  m_actions->addAction(action);
  toolBar()->addAction(action);

  toolBar()->addSeparator();

  //undo redo
  action = new QAction(tr("Undo"), this);
  action->setShortcut(QKeySequence::Undo);
  action->setIcon(QIcon::fromTheme("edit-undo", QIcon(":/icons/tango/32x32/actions/edit-undo.png")));
  action->setStatusTip(tr("Undo modifications"));
  connect(action, SIGNAL(triggered()), codeEditor(), SLOT(undo()));
  m_actions->addAction(action);
  toolBar()->addAction(action);

  action = new QAction(tr("Redo"), this);
  action->setShortcut(QKeySequence::Redo);
  action->setIcon(QIcon::fromTheme("edit-redo", QIcon(":/icons/tango/32x32/actions/edit-redo.png")));
  action->setStatusTip(tr("Redo modifications"));
  connect(action, SIGNAL(triggered()), codeEditor(), SLOT(redo()));
  m_actions->addAction(action);
  toolBar()->addAction(action);

  toolBar()->addSeparator();

  //find and replace
  m_findReplaceDialog = new CFindReplaceDialog(this);
  m_findReplaceDialog->setModal(false);
  m_findReplaceDialog->setTextEditor(codeEditor());

  action = new QAction(tr("Search and Replace"), this);
  action->setShortcut(QKeySequence::Replace);
  action->setIcon(QIcon::fromTheme("edit-find-replace", QIcon(":/icons/tango/32x32/actions/edit-find-replace.png")));
  action->setStatusTip(tr("Find some text and replace it"));
  connect(action, SIGNAL(triggered()), m_findReplaceDialog, SLOT(show()));
  m_actions->addAction(action);
  toolBar()->addAction(action);

  action = new QAction(tr("Search"), this);
  action->setShortcut(QKeySequence::Find);
  connect(action, SIGNAL(triggered()), codeEditor(), SLOT(toggleQuickSearch()));

  //spellchecking
  m_spellCheckingAct = new QAction(tr("Chec&k spelling"), this);
  m_spellCheckingAct->setIcon(QIcon::fromTheme("tools-check-spelling", QIcon(":/icons/tango/32x32/actions/tools-check-spelling.png")));
  m_spellCheckingAct->setStatusTip(tr("Check current song for incorrect spelling"));
  m_spellCheckingAct->setCheckable(true);
  m_spellCheckingAct->setEnabled(false);
  m_actions->addAction(action);
  toolBar()->addAction(m_spellCheckingAct);

  toolBar()->addSeparator();

  //songbook
  action = new QAction(tr("Verse"), this);
  action->setToolTip(tr("Insert a new verse"));
  action->setStatusTip(tr("Insert a new verse"));
  connect(action, SIGNAL(triggered()), codeEditor(), SLOT(insertVerse()));
  m_actions->addAction(action);
  toolBar()->addAction(action);

  action = new QAction(tr("Chorus"), this);
  action->setToolTip(tr("Insert a new chorus"));
  action->setStatusTip(tr("Insert a new chorus"));
  connect(action, SIGNAL(triggered()), codeEditor(), SLOT(insertChorus()));
  m_actions->addAction(action);
  toolBar()->addAction(action);

  action = new QAction(tr("Bridge"), this);
  action->setToolTip(tr("Insert a new bridge"));
  action->setStatusTip(tr("Insert a new bridge"));
  connect(action, SIGNAL(triggered()), codeEditor(), SLOT(insertBridge()));
  m_actions->addAction(action);
  toolBar()->addAction(action);

  QBoxLayout *mainLayout = new QVBoxLayout();
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);
  mainLayout->addWidget(m_songHeaderEditor);
  mainLayout->addWidget(codeEditor());
  setLayout(mainLayout);

  hide(); // required to hide some widgets from the code editor

  // the editor is set for new song by default
  setWindowTitle("New song");
  setNewSong(true);

  readSettings();
}

CSongEditor::~CSongEditor()
{
  delete m_codeEditor;
  delete m_songHeaderEditor;
  delete m_toolBar;
  delete m_actions;
  delete m_findReplaceDialog;
}

void CSongEditor::readSettings()
{
  QSettings settings;
  settings.beginGroup("editor");
  m_findReplaceDialog->readSettings();
  settings.endGroup();
}

void CSongEditor::writeSettings()
{
  m_findReplaceDialog->writeSettings();
}

QActionGroup* CSongEditor::actionGroup() const
{
  return m_actions;
}

void CSongEditor::closeEvent(QCloseEvent *event)
{
  if (isModified())
    {
      QMessageBox::StandardButton answer =
	QMessageBox::question(this, tr("Songbook-Client"),
			      tr("The document has been modified.\n"
				 "Do you want to save your changes?"),
			      QMessageBox::Save | QMessageBox::Discard
			      | QMessageBox::Cancel,
			      QMessageBox::Save);

      if (answer == QMessageBox::Save)
	{
	  save();
	  writeSettings();
	  event->accept();
	}
      if (answer == QMessageBox::Cancel)
	{
	  event->ignore();
	}
      if (answer == QMessageBox::Discard)
	{
	  event->accept();
	}
    }
}

void CSongEditor::save()
{
  if (!checkSongMandatoryFields())
    return;

  // get the song contents
  parseText();

  // save the song and add it to the library list
  library()->createArtistDirectory(m_song);

  if (isNewCover() && !m_songHeaderEditor->cover().isNull())
    library()->saveCover(m_song, m_songHeaderEditor->cover());
  library()->saveSong(m_song);

  setNewCover(false);
  setNewSong(false);
  setModified(false);
  setWindowTitle(m_song.title);
  emit(labelChanged(windowTitle()));
  setStatusTip(tr("Song saved in: %1").arg(song().path));
}

bool CSongEditor::checkSongMandatoryFields()
{
  QString css("QLineEdit{ border: 1px solid red; border-radius: 2px; }");

  if (song().title.isEmpty())
    {
      setStatusTip(tr("Invalid song title"));
      m_songHeaderEditor->titleLineEdit()->setStyleSheet(css);
      return false;
    }
  m_songHeaderEditor->titleLineEdit()->setStyleSheet(QString());

  if (song().artist.isEmpty())
    {
      setStatusTip(tr("Invalid artist name"));
      m_songHeaderEditor->artistLineEdit()->setStyleSheet(css);
      return false;
    }
  m_songHeaderEditor->artistLineEdit()->setStyleSheet(QString());

  return true;
}

void CSongEditor::parseText()
{
  m_song.lyrics.clear();
  m_song.scripture.clear();

  bool in_scripture = false;

  QStringList lines = codeEditor()->toPlainText().split("\n");
  foreach (QString line, lines)
    {
      if (Song::reBeginScripture.indexIn(line) > -1)
        in_scripture = true;

      if (in_scripture)
        {
          // ensures all lines in a scripture environment end with a % symbol
          if (!line.endsWith("%"))
            line = line.append("%");
          m_song.scripture << line;
        }
      else
        {
          // add a level of indentation
          if (!line.isEmpty())
            line = line.prepend("  ");
          m_song.lyrics << line;
        }

      if (Song::reEndScripture.indexIn(line) > -1)
        in_scripture = false;
    }

  // remove blank line at the end of input
  while (!m_song.lyrics.empty() && m_song.lyrics.last().trimmed().isEmpty())
    {
      m_song.lyrics.removeLast();
    }

  while (!m_song.scripture.isEmpty() && m_song.scripture.last().trimmed().isEmpty())
    {
      m_song.scripture.removeLast();
    }

  // finally insert newline after endsong macro
  m_song.lyrics << QString();
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
  library()->addSong(m_song, true);

  setNewSong(false);
}

void CSongEditor::documentWasModified()
{
  setModified(true);
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
  m_songHeaderEditor->setLibrary(library);
}

bool CSongEditor::isModified() const
{
  return codeEditor()->document()->isModified();
}

void CSongEditor::setModified(bool modified)
{
  if (codeEditor()->document()->isModified() != modified)
    codeEditor()->document()->setModified(modified);

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
  foreach (QString lyric, m_song.lyrics)
    {
      songContent.append(QString("%1\n").arg(lyric));
    }
  foreach (QString line, m_song.scripture)
    {
      songContent.append(QString("%1\n").arg(line));
    }

  codeEditor()->setPlainText(songContent);
  codeEditor()->indent();

#ifdef ENABLE_SPELLCHECK
  setDictionary(song.locale);
#endif //ENABLE_SPELLCHECK

  setNewSong(false);
  setNewCover(false);
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

bool CSongEditor::isNewCover() const
{
  return m_newCover;
}

void CSongEditor::setNewCover(bool newCover)
{
  m_newCover = newCover;
}


CSongCodeEditor* CSongEditor::codeEditor() const
{
  return m_codeEditor;
}

bool CSongEditor::isSpellCheckAvailable() const
{
  return codeEditor()->isSpellCheckAvailable();
}

void CSongEditor::setSpellCheckAvailable(const bool value)
{
  codeEditor()->setSpellCheckAvailable(value);
  m_spellCheckingAct->setEnabled(value);
  if (!value && m_spellCheckingAct->isChecked())
    m_spellCheckingAct->setChecked(false);
}

#ifdef ENABLE_SPELLCHECK
void CSongEditor::setDictionary(const QLocale &locale)
{
  // find the suitable dictionary based on the current song's locale
  QString prefix;
#if defined(Q_OS_WIN32)
  prefix = "";
#else
  prefix = "/usr/share/";
#endif //Q_OS_WIN32
  QString dictionary = QString("%1hunspell/%2.dic").arg(prefix).arg(locale.name());;
  if (!QFile(dictionary).exists())
    {
      setStatusTip(tr("Unable to find the following dictionary: %1").arg(dictionary));
      setSpellCheckAvailable(false);
      return;
    }
  setStatusTip("");
  setSpellCheckAvailable(true);
  codeEditor()->setDictionary(dictionary);
}
#endif //ENABLE_SPELLCHECK

void CSongEditor::installHighlighter()
{
  codeEditor()->installHighlighter();
#ifdef ENABLE_SPELLCHECK
  connect(m_spellCheckingAct, SIGNAL(toggled(bool)),
	  m_codeEditor, SLOT(setSpellCheckActive(bool)));
  connect(m_spellCheckingAct, SIGNAL(toggled(bool)),
	  m_codeEditor, SLOT(setSpellCheckActive(bool)));
#endif //ENABLE_SPELLCHECK
}
