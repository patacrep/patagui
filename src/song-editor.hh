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
#ifndef __SONG_EDITOR_HH__
#define __SONG_EDITOR_HH__

#include "config.hh"
#include "song.hh"

#include <QWidget>
#include <QString>
#include <QTextCursor>
#include <QKeyEvent>

class QAction;
class QActionGroup;
class QToolBar;
class Hunspell;
class FindReplaceDialog;
class CLibrary;
class CSongCodeEditor;
class CSongHighlighter;
class CSongHeaderEditor;

/**
 * \file song-editor.hh
 * \class CSongEditor
 * \brief CSongEditor is the widget that allows to write a song
 *
 * A CSongEditor is embedded into a CTabWidget and is composed of:
 *  \li a CSongHeaderEditor that manages the song metadata
 *  \li a CSongCodeEditor that manages the body of the song
 *
 * \image html song-editor.png
 *
 */
class CSongEditor : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(bool newSong READ isNewSong WRITE setNewSong)
  Q_PROPERTY(bool newCover READ isNewCover WRITE setNewCover)

public:
  CSongEditor(QWidget *parent = 0);
  ~CSongEditor();

  QToolBar * toolBar() const;
  CLibrary * library() const;
  void setLibrary(CLibrary *library);

  QActionGroup * actionGroup() const;

  void readSettings();
  void writeSettings();
  QStringList getWordPropositions(const QString &word);
#ifdef ENABLE_SPELL_CHECKING
  Hunspell* checker() const;
#endif
  void installHighlighter();

  bool isSpellCheckingEnabled() const;
  void setSpellCheckingEnabled(const bool);

  Song & song();
  void setSong(const Song &song);

  CSongCodeEditor * codeEditor() const;

  bool isModified() const;
  bool isNewSong() const;

  //! Getter on the new cover property
  bool isNewCover() const;

  //! Setter on the new cover property
  void setNewCover(bool newCover);

public slots:
  void setModified(bool modified);
  void setNewSong(bool newSong);

signals:
  void labelChanged(const QString &label);
  void wordAdded(const QString &word);
  void saved(const QString &path);

protected:

#ifdef ENABLE_SPELL_CHECKING
  void contextMenuEvent(QContextMenuEvent *event);
  QString currentWord();
  void setDictionary(const QLocale &locale);
#endif //ENABLE_SPELL_CHECKING

private slots:
  //write modifications of the textEdit into sg file.
  void save();
  void documentWasModified();
  void insertVerse();
  void insertChorus();

#ifdef ENABLE_SPELL_CHECKING
  void correctWord();
  void addWord();
  void ignoreWord();
#endif //ENABLE_SPELL_CHECKING

private:
  void parseText();
  bool checkSongMandatoryFields();
  void saveNewSong();
  void createNewSong();

  CSongCodeEditor *m_codeEditor;
  CSongHeaderEditor *m_songHeaderEditor;
  CLibrary *m_library;
  QToolBar *m_toolBar;
  QActionGroup *m_actions;

  QAction* m_spellCheckingAct;
  bool m_isSpellCheckingEnabled;

#ifdef ENABLE_SPELL_CHECKING
  QList<QAction *> m_misspelledWordsActs;
  QPoint m_lastPos;
  QStringList m_addedWords;
  uint m_maxSuggestedWords;
#endif //ENABLE_SPELL_CHECKING

  FindReplaceDialog* m_findReplaceDialog;

  Song m_song;
  bool m_newSong;
  bool m_newCover;
};

#endif // __SONG_EDITOR_HH__
