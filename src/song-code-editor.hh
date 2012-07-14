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
class CSearchWidget;
/*!
  \file song-code-editor.hh
  \class CSongCodeEditor
  \brief CSongCodeEditor is the widget to edit a song's content

  \image html song-code-editor.png

 */
class CSongCodeEditor : public CodeEditor
{
  Q_OBJECT
  Q_ENUMS(SongEnvironment)

public:
  /*!
    \enum SongEnvironment
    This enum type describes available LaTeX environments in a song.
    Environments are usually enclosed within
    \\begin{environment} and \\end{environment} macros.
  */
  enum SongEnvironment {
    Verse, /*!< verse environment. */
    Bridge, /*!< bridge environment. */
    Chorus, /*!< chorus environment. */
    Scripture, /*!< scripture environment. */
    None /*!< no environment. */
  };

  /// Constructor.
  CSongCodeEditor(QWidget *parent = 0);

  /// Destructor.
  ~CSongCodeEditor();

  /*!
    Reads editor settings (ie, font, line numbers etc.).
    \sa writeSettings
  */
  void readSettings();

  /*!
    Writes editor settings (ie, font, line numbers etc.).
    \sa readSettings
  */
  void writeSettings();

  /*!
    Creates and associates the syntax highlighter with the song.
    \sa highlighter
  */
  void installHighlighter();

  /*!
    Returns the syntax highlighter.
    \sa installHighlighter
  */
  CSongHighlighter* highlighter() const;

  /*!
    Returns the macro completer. The completer is a pop-up menu
    that suggests common macros for convenience.
  */
  QCompleter* completer() const;

  /*!
    Returns \a true if a backgroundColor is set for environments; \a false otherwise.
    \sa setEnvironmentsHighlighted
  */
  bool environmentsHighlighted() const;

  /*!
    Sets a background color for environments if \a value is \a true.
    \sa environmentsHighlighted
  */
  void setEnvironmentsHighlighted(bool value);

  /*!
    Performs indentation for the whole text in the editor.
    Indentation is 2 whitespaces. A new environment adds a level of indentation.
    \sa indentSelection
  */
  void indent();

  /*!
    Performs indentation for the current selection.
    \sa indent
  */
  void indentSelection();

  /*!
    Returns \a true if spell-checking is available
    (ie, a hunspell dictionary matching the language of the song does exist).
    \sa setSpellCheckAvailable
  */
  bool isSpellCheckAvailable() const;

  /*!
    Sets the availabilty of spell-checking to \a value.
    (ie, a hunspell dictionary matching the language of the song does exist).
    \sa isSpellCheckAvailable
  */
  void setSpellCheckAvailable(const bool value);

  /*!
    Returns \a true if spell-checking is active; \a false otherwise.
    \sa isSpellCheckAvailable
  */
  bool isSpellCheckActive() const;

public slots:
  /*!
    Highlights mispelled words according to \a value.
    \sa isSpellCheckActive
  */
  void setSpellCheckActive(const bool value);

  /*!
    Toggle the visibility of the quick search widget.
  */
  void toggleQuickSearch();

protected:
  /*!
    Performs indentation when hitting the tab key.
    Pops-up completer when hitting the ctrl+space key.
    \sa indent, completer
  */
  virtual void keyPressEvent(QKeyEvent *event);

  /*!
    Reimplements CodeEditor::resizeEvent to move the
    quick search widget in the top-right corner of the editor.
  */
  virtual void resizeEvent(QResizeEvent *event);

  /*!
    Provides custom context menu with specific actions that are relevant for song edition.
    For example, comment/uncomment selection, spell-checking options etc.
  */
  void contextMenuEvent(QContextMenuEvent *event);

signals:
  /*!
    This signal is emitted when a word \a word is added to the spellchecker dictionary.
  */
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
public:
  /*!
    Returns the Hunspell spell-checker associated with this song.
    \sa isSpellCheckAvailable, isSpellCheckActive
  */
  Hunspell* checker() const;

public slots:
  /*!
    Uses the file \a dictionary for spell-checking.
    A \a dictionary is a .dic file for Hunspell.
    \sa checker, isSpellCheckAvailable, isSpellCheckActive
  */
  void setDictionary(const QString &dictionary);

private slots:
  QString currentWord();
  void correctWord();
  void addWord();
  void ignoreWord();

private:
  QStringList getWordPropositions(const QString &word);
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

  bool m_isSpellCheckAvailable;
  bool m_isSpellCheckActive;

#ifdef ENABLE_SPELLCHECK
  QList<QAction *> m_misspelledWordsActs;
  QPoint m_lastPos;
  QStringList m_addedWords;
  uint m_maxSuggestedWords;
#endif //ENABLE_SPELLCHECK

  CSearchWidget *m_quickSearch;
};

#endif // __SONG_CODE_EDITOR_HH__
