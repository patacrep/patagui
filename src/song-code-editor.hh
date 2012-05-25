// Copyright (C) 2009-2012, Romain Goffe <romain.goffe@gmail.com>
// Copyright (C) 2009-2012, Alexandre Dupas <alexandre.dupas@gmail.com>
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
#ifndef __SONG_CODE_EDITOR_HH__
#define __SONG_CODE_EDITOR_HH__

#include "config.hh"
#include "code-editor.hh"

#include <QTextCursor>

class QKeyEvent;
class QCompleter;
class CSongHighlighter;
class Hunspell;

/**
 * \file song-code-editor.hh
 * \class CSongCodeEditor
 * \brief CSongCodeEditor is the widget to edit a song's content
 *
 */
class CSongCodeEditor : public CodeEditor
{
  Q_OBJECT
  Q_ENUMS(SongEnvironment)

public:
  enum SongEnvironment { Verse, Bridge, Chorus, Scripture, None };

  CSongCodeEditor(QWidget *parent = 0);
  ~CSongCodeEditor();

  void readSettings();
  void writeSettings();

  void installHighlighter();

  CSongHighlighter* highlighter() const;
  QCompleter* completer() const;

  bool environmentsHighlighted() const;
  void setEnvironmentsHighlighted(bool);

  void indent();
  void indentSelection();

  bool isSpellCheckingEnabled() const;
  void setSpellCheckingEnabled(const bool);

  QStringList getWordPropositions(const QString &word);
#ifdef ENABLE_SPELLCHECK
  Hunspell* checker() const;
public slots:
  void setDictionary(const QLocale &locale);
#endif

protected:
  virtual void keyPressEvent(QKeyEvent *event);
  void contextMenuEvent(QContextMenuEvent *event);

signals:
  void wordAdded(const QString &word);

private slots:
  void highlightEnvironments();
  void insertCompletion(const QString &completion);
  void insertVerse();
  void insertChorus();
  void insertBridge();
  void commentSelection();
  void uncommentSelection();

#ifdef ENABLE_SPELLCHECK
  void correctWord();
  void addWord();
  void ignoreWord();

protected slots:
  QString currentWord();
#endif //ENABLE_SPELLCHECK

private:
  void indentLine(const QTextCursor &cursor);
  void trimLine(const QTextCursor &cursor);
  QString textUnderCursor() const;

  QTextEdit::ExtraSelection environmentSelection(const SongEnvironment & env,
						 const QTextCursor & cursor);

  CSongHighlighter* m_highlighter;
  QCompleter* m_completer;

  bool m_environmentsHighlighted;

  QColor m_verseColor;
  QColor m_chorusColor;
  QColor m_bridgeColor;
  QColor m_scriptureColor;

  bool m_isSpellCheckingEnabled;

#ifdef ENABLE_SPELLCHECK
  QList<QAction *> m_misspelledWordsActs;
  QPoint m_lastPos;
  QStringList m_addedWords;
  uint m_maxSuggestedWords;
#endif //ENABLE_SPELLCHECK
};

#endif // __SONG_CODE_EDITOR_HH__
