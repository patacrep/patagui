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
 * \class SongHighlighter
 * \brief SongHighlighter provides colors and highlights for the song editor.
 *
 * Highlights include LaTeX keywords and specific commands provided by the
 * Songs LaTeX package (http://songs.sourceforge.net).
 *
 * This class is also used by the hunspell spellchecker to underline
 * unrecognized words.
 *
 */
class SongHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    /// Constructor
    SongHighlighter(QTextDocument *parent = 0);
    /// Destructor
    ~SongHighlighter();

    /// Set the dictionary used by the spellchecker.
    /// @param filename the .dic file that corresponds to a hunspell dictionary.
    /// Those files are usually located in /usr/share/hunspell/.
    void setDictionary(const QString &filename);

    /// Getter on the Hunspell spellchecker.
    /// @return the hunspell spellchecker
    Hunspell *checker() const;

public slots:
#ifdef ENABLE_SPELLCHECK

    bool isSpellCheckActive() const;

    /// Add an unrecognized word to the hunspell dictionary
    /// so that it is not marked as incorrect anymore.
    /// @param word the word that is to be marked as correct.
    void addWord(const QString &word);

    /// Define whether the spellchecker is active or not.
    /// @param state true if the spellchecker is active, false otherwise.
    void setSpellCheckActive(const bool state);
#endif // ENABLE_SPELLCHECK

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
    struct HighlightingRule {
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

    Hunspell *m_checker;
    bool m_isSpellCheckActive;
    QTextCharFormat m_spellCheckFormat;
    QTextCodec *m_codec;

    const static QRegExp reLaTeXOption;
    const static QRegExp reLaTeXArgument;

    const static QRegExp reChordsPattern;
    const static QRegExp reCommentsPattern;

    const static QStringList _keywordPatterns;
    const static QStringList _keyword2Patterns;
    const static QStringList _delimiters;

    const static QColor _keywords1Color;
    const static QColor _keywords2Color;
    const static QColor _environmentsColor;
    const static QColor _commentsColor;
    const static QColor _quotesColor;
    const static QColor _chordsColor;
};

#endif // __HIGHLIGHTER_HH__
