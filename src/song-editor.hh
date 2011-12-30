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
#include "code-editor.hh"
#include "song.hh"

#include <QWidget>
#include <QString>
#include <QTextCursor>
#include <QKeyEvent>

class QAction;
class Hunspell;
class CHighlighter;
class FindReplaceDialog;

class QToolBar;

class CodeEditor;
class CSongHeaderEditor;
class CLibrary;

class CSongEditor : public QWidget
{
  Q_OBJECT

public:
  CSongEditor(QWidget *parent = 0);
  ~CSongEditor();

  QString path();
  void setPath(const QString &path);

  QToolBar * toolBar() const;
  CLibrary * library() const;
  void setLibrary(CLibrary *library);

  void readSettings();
  void writeSettings();
  QStringList getWordPropositions(const QString &word);
  Hunspell* checker() const;
  void installHighlighter();

  bool isSpellCheckingEnabled() const;
  void setSpellCheckingEnabled(const bool);

  bool isModified() const;

  Song & song();

  bool isNewSong() const;

public slots:
  void setNewSong(bool newSong);

signals:
  void labelChanged(const QString &label);
  void wordAdded(const QString &word);
  void saved(const QString &path);

protected:
  virtual void keyPressEvent(QKeyEvent *event);

#ifdef ENABLE_SPELL_CHECKING
  void contextMenuEvent(QContextMenuEvent *event);
  QString currentWord();
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
  QString syntaxicColoration(const QString &string);
  void indentSelection();
  void indentLine(const QTextCursor &cursor);
  void trimLine(const QTextCursor &cursor);

  void parseText();

  void saveNewSong();
  void createNewSong();

  CodeEditor *m_songEditor;
  CSongHeaderEditor *m_songHeaderEditor;
  CLibrary *m_library;
  QToolBar *m_toolBar;

  QString m_path;
  QList<QAction*> m_actions;
  CHighlighter* m_highlighter;
  bool m_isSpellCheckingEnabled;

#ifdef ENABLE_SPELL_CHECKING
  QList<QAction *> m_misspelledWordsActs;
  QPoint m_lastPos;
  QStringList m_addedWords;
  uint m_maxSuggestedWords;
  QString m_dictionary;
  QAction* m_spellCheckingAct;
#endif //ENABLE_SPELL_CHECKING

  FindReplaceDialog* m_findReplaceDialog;

  Song m_song;
  bool m_newSong;
};

#endif // __SONG_EDITOR_HH__
