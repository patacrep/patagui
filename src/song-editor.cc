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

CEditor::CEditor(QWidget *parent)
  : QWidget(parent)
  , m_actions(new QActionGroup(this))
{
  setAttribute(Qt::WA_DeleteOnClose);

  // toolBar
  m_toolBar = new QToolBar(tr("Edition tools"), this);
  m_toolBar->setMovable(false);
  m_toolBar->setContextMenuPolicy(Qt::PreventContextMenu);

  // actions
  m_saveAct = new QAction(tr("Save"), this);
  m_saveAct->setShortcut(QKeySequence::Save);
  m_saveAct->setIcon(QIcon::fromTheme("document-save", QIcon(":/icons/tango/32x32/actions/document-save.png")));
  m_saveAct->setStatusTip(tr("Save modifications"));
  m_actions->addAction(m_saveAct);
  toolBar()->addAction(m_saveAct);

  //copy paste
  m_cutAct = new QAction(tr("Cut"), this);
  m_cutAct->setShortcut(QKeySequence::Cut);
  m_cutAct->setIcon(QIcon::fromTheme("edit-cut", QIcon(":/icons/tango/32x32/actions/edit-cut.png")));
  m_cutAct->setStatusTip(tr("Cut the selection"));
  m_actions->addAction(m_cutAct);
  toolBar()->addAction(m_cutAct);

  m_copyAct = new QAction(tr("Copy"), this);
  m_copyAct->setShortcut(QKeySequence::Copy);
  m_copyAct->setIcon(QIcon::fromTheme("edit-copy", QIcon(":/icons/tango/32x32/actions/edit-copy.png")));
  m_copyAct->setStatusTip(tr("Copy the selection"));
  m_actions->addAction(m_copyAct);
  toolBar()->addAction(m_copyAct);

  m_pasteAct = new QAction(tr("Paste"), this);
  m_pasteAct->setShortcut(QKeySequence::Paste);
  m_pasteAct->setIcon(QIcon::fromTheme("edit-paste", QIcon(":/icons/tango/32x32/actions/edit-paste.png")));
  m_pasteAct->setStatusTip(tr("Paste clipboard content"));
  m_actions->addAction(m_pasteAct);
  toolBar()->addAction(m_pasteAct);

  toolBar()->addSeparator();

  //undo redo
  m_undoAct = new QAction(tr("Undo"), this);
  m_undoAct->setShortcut(QKeySequence::Undo);
  m_undoAct->setIcon(QIcon::fromTheme("edit-undo", QIcon(":/icons/tango/32x32/actions/edit-undo.png")));
  m_undoAct->setStatusTip(tr("Undo modifications"));
  m_actions->addAction(m_undoAct);
  toolBar()->addAction(m_undoAct);

  m_redoAct = new QAction(tr("Redo"), this);
  m_redoAct->setShortcut(QKeySequence::Redo);
  m_redoAct->setIcon(QIcon::fromTheme("edit-redo", QIcon(":/icons/tango/32x32/actions/edit-redo.png")));
  m_redoAct->setStatusTip(tr("Redo modifications"));
  m_actions->addAction(m_redoAct);
  toolBar()->addAction(m_redoAct);

  m_replaceAct = new QAction(tr("Search and Replace"), this);
  m_replaceAct->setShortcut(QKeySequence::Replace);
  m_replaceAct->setIcon(QIcon::fromTheme("edit-find-replace", QIcon(":/icons/tango/32x32/actions/edit-find-replace.png")));
  m_replaceAct->setStatusTip(tr("Find some text and replace it"));
  m_actions->addAction(m_replaceAct);
  toolBar()->addAction(m_replaceAct);

  m_searchAct = new QAction(tr("Search"), this);
  m_searchAct->setShortcut(QKeySequence::Find);

  //spellchecking
  m_spellCheckingAct = new QAction(tr("Chec&k spelling"), this);
  m_spellCheckingAct->setIcon(QIcon::fromTheme("tools-check-spelling", QIcon(":/icons/tango/32x32/actions/tools-check-spelling.png")));
  m_spellCheckingAct->setStatusTip(tr("Check current song for incorrect spelling"));
  m_spellCheckingAct->setCheckable(true);
  m_spellCheckingAct->setEnabled(false);
#ifndef ENABLE_SPELLCHECK
  m_spellCheckingAct->setVisible(false);
#endif //ENABLE_SPELLCHECK
  m_actions->addAction(m_spellCheckingAct);
  toolBar()->addAction(m_spellCheckingAct);

  toolBar()->addSeparator();

  //songbook
  m_verseAct = new QAction(tr("Verse"), this);
  m_verseAct->setToolTip(tr("Insert a new verse"));
  m_verseAct->setStatusTip(tr("Insert a new verse"));
  m_actions->addAction(m_verseAct);
  toolBar()->addAction(m_verseAct);

  m_chorusAct = new QAction(tr("Chorus"), this);
  m_chorusAct->setToolTip(tr("Insert a new chorus"));
  m_chorusAct->setStatusTip(tr("Insert a new chorus"));
  m_actions->addAction(m_chorusAct);
  toolBar()->addAction(m_chorusAct);

  m_bridgeAct = new QAction(tr("Bridge"), this);
  m_bridgeAct->setToolTip(tr("Insert a new bridge"));
  m_bridgeAct->setStatusTip(tr("Insert a new bridge"));
  m_actions->addAction(m_bridgeAct);
  toolBar()->addAction(m_bridgeAct);
}

CEditor::~CEditor()
{
  delete m_toolBar;
  delete m_actions;
}

QToolBar * CEditor::toolBar() const
{
  m_toolBar->setVisible(false);
  return m_toolBar;
}

QActionGroup* CEditor::actionGroup() const
{
  m_actions->setEnabled(false);
  return m_actions;
}

bool CEditor::isSpellCheckAvailable() const
{
  return false;
}

void CEditor::setSpellCheckAvailable(const bool)
{}


CSongEditor::CSongEditor(QWidget *parent)
  : CEditor(parent)
  , m_codeEditor(0)
  , m_songHeaderEditor(0)
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

  //find and replace
  m_findReplaceDialog = 0;

  //connects
  connect(m_saveAct, SIGNAL(triggered()), SLOT(save()));
  connect(m_cutAct, SIGNAL(triggered()), codeEditor(), SLOT(cut()));
  connect(m_copyAct, SIGNAL(triggered()), codeEditor(), SLOT(copy()));
  connect(m_pasteAct, SIGNAL(triggered()), codeEditor(), SLOT(paste()));
  connect(m_undoAct, SIGNAL(triggered()), codeEditor(), SLOT(undo()));
  connect(m_redoAct, SIGNAL(triggered()), codeEditor(), SLOT(redo()));
  connect(m_searchAct, SIGNAL(triggered()), codeEditor(), SLOT(toggleQuickSearch()));
  connect(m_verseAct, SIGNAL(triggered()), codeEditor(), SLOT(insertVerse()));
  connect(m_chorusAct, SIGNAL(triggered()), codeEditor(), SLOT(insertChorus()));
  connect(m_bridgeAct, SIGNAL(triggered()), codeEditor(), SLOT(insertBridge()));
  connect(m_replaceAct, SIGNAL(triggered()), SLOT(findReplaceDialog()));

  QBoxLayout *mainLayout = new QVBoxLayout();
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);
  mainLayout->addWidget(m_songHeaderEditor);
  mainLayout->addWidget(codeEditor());
  setLayout(mainLayout);

  hide(); // required to hide some widgets from the code editor

  // the editor is set for new song by default
  setWindowTitle(tr("New song"));
  setNewSong(true);

  readSettings();
}

CSongEditor::~CSongEditor()
{
  delete m_codeEditor;
  delete m_songHeaderEditor;
  if (m_findReplaceDialog)
    delete m_findReplaceDialog;
}

void CSongEditor::readSettings()
{}

void CSongEditor::writeSettings()
{
  if (m_findReplaceDialog)
    m_findReplaceDialog->writeSettings();
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
      if (line.contains("\\beginscripture"))
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

      if (line.contains("\\endscripture"))
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

CLibrary * CSongEditor::library() const
{
  return CLibrary::getInstance();
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

QActionGroup* CSongEditor::actionGroup() const
{
  m_actions->setEnabled(true);
  return m_actions;
}

QToolBar * CSongEditor::toolBar() const
{
  m_toolBar->setVisible(true);
  return m_toolBar;
}

void CSongEditor::findReplaceDialog()
{
  if (!m_findReplaceDialog)
    {
      m_findReplaceDialog = new CFindReplaceDialog(this);
      m_findReplaceDialog->setModal(false);
      m_findReplaceDialog->setTextEditor(codeEditor());
      m_findReplaceDialog->readSettings();
    }
  m_findReplaceDialog->show();
}
