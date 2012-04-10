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
#ifndef __HIGHLIGHTER_HH__
#define __HIGHLIGHTER_HH__

#include "config.hh"
#include <QSyntaxHighlighter>
#include <QHash>
#include <QTextCharFormat>

class QTextDocument;
class Hunspell;

/**
 * \file song-highlighter.hh
 * \class CSongHighlighter
 * \brief CSongHighlighter provides colors and highlights for the song editor.
 *
 * Highlights include LaTeX keywords and specific commands provided by the
 * Songs LaTeX package (http://songs.sourceforge.net).
 *
 * This class is also used by the hunspell spellchecker to underline
 * unrecognized words.
 *
 */
class CSongHighlighter : public QSyntaxHighlighter
{
  Q_OBJECT

public:
  /// Constructor
  CSongHighlighter(QTextDocument *parent = 0);
  /// Destructor
  ~CSongHighlighter();

  /// Set the dictionnary used by the spellchecker.
  /// @param filename the .dic file that corresponds to a hunspell dictionnary.
  /// Those files are usually located in /usr/share/hunspell/.
  void setDictionary(const QString &filename);

  /// Getter on the Hunspell spellchecker.
  /// @return the hunspell spellchecker
  Hunspell* checker() const;

#ifdef ENABLE_SPELL_CHECKING
public slots:
  /// Add an unrecognized word to the hunspell dictionnary
  /// so that it is not marked as incorrect anymore.
  /// @param word the word that is to be marked as correct.
  void addWord(const QString &word);

  /// Define whether the spellchecker is active or not.
  /// @param state true if the spellchecker is active, false otherwise.
  void setSpellCheck(const bool state);
#endif //ENABLE_SPELL_CHECKING

protected:
  /// Apply highlighting rules for a block of text.
  /// @param text the text on which the rules should be applied.
  void highlightBlock(const QString &text);

  /// Apply spellchecking on a text.
  /// @param text the text on which the spellchecking should be applied.
  void spellCheck(const QString &text);

  /// Apply spellchecking on a particular word.
  /// @param word the word that is to be spellchecked.
  /// @return true if the word is correctly spelled, false otherwise.
  bool checkWord(const QString &word);


private:
  struct HighlightingRule
  {
    QRegExp pattern;
    QTextCharFormat format;
  };
  QVector<HighlightingRule> highlightingRules;

  QTextCharFormat keywordFormat;
  QTextCharFormat keyword2Format;
  QTextCharFormat environmentFormat;
  QTextCharFormat singleLineCommentFormat;
  QTextCharFormat chordFormat;
  QTextCharFormat quotationFormat;
  QTextCharFormat argumentFormat;
  QTextCharFormat optionFormat;

  QTextCharFormat multiLineCommentFormat;

  Hunspell * m_checker;
  bool m_isSpellCheckActive;
  QTextCharFormat m_spellCheckFormat;
  QTextCodec *m_codec;
};

#endif // __HIGHLIGHTER_HH__
