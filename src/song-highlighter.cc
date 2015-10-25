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

#include <QFileInfo>
#include <QTextCodec>

#include "config.hh"
#include "song-highlighter.hh"
#include "song.hh"
#ifdef ENABLE_SPELLCHECK
#include "hunspell/hunspell.hxx"
#endif // ENABLE_SPELLCHECK

#include "utils/tango-colors.hh"

#include <QDebug>

const QRegExp SongHighlighter::reLaTeXOption("\\[[^\\]]+\\]");
const QRegExp SongHighlighter::reLaTeXArgument("\\{[^}]+\\}");

const QRegExp SongHighlighter::reChordsPattern("\\\\\\[[^\\]]+\\]");
const QRegExp SongHighlighter::reCommentsPattern("%[^\n]*");

const QStringList SongHighlighter::_keywordPatterns(QStringList()
                                                    << "\\\\gtab"
                                                    << "\\\\utab"
                                                    << "\\\\rep"
                                                    << "\\\\lilypond"
                                                    << "\\\\image"
                                                    << "\\\\songcolumns"
                                                    << "\\\\cover"
                                                    << "\\\\capo"
                                                    << "\\\\nolyrics"
                                                    << "\\\\musicnote"
                                                    << "\\\\textnote"
                                                    << "\\\\dots"
                                                    << "\\\\single"
                                                    << "\\\\echo"
                                                    << "\\\\transpose"
                                                    << "\\\\transposition"
                                                    << "\\\\emph"
                                                    << "\\\\selectlanguage");

const QStringList SongHighlighter::_keyword2Patterns(QStringList()
                                                     << "\\\\Intro"
                                                     << "\\\\Rhythm"
                                                     << "\\\\Outro"
                                                     << "\\\\Bridge"
                                                     << "\\\\Verse"
                                                     << "\\\\Chorus"
                                                     << "\\\\Pattern"
                                                     << "\\\\Solo"
                                                     << "\\\\Adlib"
                                                     << "\\\\else"
                                                     << "\\\\ifchorded"
                                                     << "\\\\iflyrics"
                                                     << "\\\\ifnorepeatchords"
                                                     << "\\\\fi");

const QStringList SongHighlighter::_delimiters(QStringList()
                                               << "\\\\begin"
                                               << "\\\\end"
                                               << "\\\\beginscripture"
                                               << "\\\\endscripture");

const QColor SongHighlighter::_keywords1Color(_TangoOrange3); // orange
const QColor SongHighlighter::_keywords2Color(_TangoScarletRed3); // red
const QColor SongHighlighter::_environmentsColor(_TangoChameleon3); // green
const QColor SongHighlighter::_commentsColor(_TangoAluminium4); // grey
const QColor SongHighlighter::_quotesColor(_TangoPlum3); // violet
const QColor SongHighlighter::_chordsColor(_TangoSkyBlue3); // blue

SongHighlighter::SongHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
    , m_checker(0)
    , m_isSpellCheckActive(false)
    , m_codec(0)
{
    HighlightingRule rule;

    // LaTeX options (overrided by chords)
    optionFormat.setFontItalic(true);
    rule.pattern = reLaTeXOption;
    rule.format = optionFormat;
    highlightingRules.append(rule);

    // LaTeX args (bold)
    argumentFormat.setFontWeight(QFont::Bold);
    rule.pattern = reLaTeXArgument;
    rule.format = argumentFormat;
    highlightingRules.append(rule);

    // Keywords1 (orange)
    keywordFormat.setForeground(_keywords1Color);
    keywordFormat.setFontWeight(QFont::Bold);
    foreach (const QString &pattern, _keywordPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // Keywords2 (red)
    keyword2Format.setForeground(_keywords2Color);
    keyword2Format.setFontWeight(QFont::Bold);
    foreach (const QString &pattern, _keyword2Patterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = keyword2Format;
        highlightingRules.append(rule);
    }

    // Environments (bold, green)
    environmentFormat.setFontWeight(QFont::Bold);
    environmentFormat.setForeground(_environmentsColor);
    foreach (QString str, _delimiters) {
        rule.pattern = QRegExp(str);
        rule.format = environmentFormat;
        highlightingRules.append(rule);
    }

    // Comments (grey)
    singleLineCommentFormat.setForeground(_commentsColor);
    rule.pattern = reCommentsPattern;
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // Quotations (violet)
    quotationFormat.setForeground(_quotesColor);
    rule.pattern = QRegExp("\".*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegExp("``.*''");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // Chords (blue)
    chordFormat.setForeground(_chordsColor);
    chordFormat.setFontWeight(QFont::Bold);
    rule.pattern = reChordsPattern;
    rule.format = chordFormat;
    highlightingRules.append(rule);

#ifdef ENABLE_SPELLCHECK
    // Settings for online spellchecking
    m_spellCheckFormat.setUnderlineColor(QColor(Qt::red));
    m_spellCheckFormat.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
#endif // ENABLE_SPELLCHECK
}

SongHighlighter::~SongHighlighter()
{
#ifdef ENABLE_SPELLCHECK
    delete m_checker;
#endif // ENABLE_SPELLCHECK
}

void SongHighlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
    setCurrentBlockState(0);

#ifdef ENABLE_SPELLCHECK
    spellCheck(text);
#endif // ENABLE_SPELLCHECK
}

#ifdef ENABLE_SPELLCHECK
void SongHighlighter::spellCheck(const QString &text)
{
    if (!m_isSpellCheckActive)
        return;

    QString str = text.simplified();
    if (str.isEmpty())
        return;

    QStringList list =
        str.split(QRegExp("([^\\w,^\\\\]|(?=\\\\))+"), QString::SkipEmptyParts);

    foreach (str, list)
        if (str.length() > 1 && !str.startsWith('\\') && !checkWord(str)) {
            int number = text.count(QRegExp("\\b" + str + "\\b"));
            int line = -1;
            // underline all incorrect occurrences of misspelled word
            for (int j = 0; j < number; ++j) {
                line = text.indexOf(QRegExp("\\b" + str + "\\b"), line + 1);
                if (line >= 0)
                    setFormat(line, str.length(), m_spellCheckFormat);
            }
        }
}

bool SongHighlighter::checkWord(const QString &word)
{
    if (!m_codec)
        return false;

    QByteArray encodedString;
    encodedString = m_codec->fromUnicode(word);
    return m_checker->spell(encodedString.data());
}

void SongHighlighter::setDictionary(const QString &filename)
{
    if (m_checker) {
        delete m_checker;
        m_checker = 0;
    }

    QFileInfo fi(filename);
    if (filename.isEmpty() || !fi.exists() || !fi.isReadable()) {
        qWarning()
            << tr("SongHighlighter::setDictionary cannot open dictionary : ")
            << filename;
        delete m_checker;
        m_checker = 0;
    } else {
        QString basename =
            QString("%1/%2").arg(fi.absolutePath()).arg(fi.baseName());
        m_checker = new Hunspell(QString("%1.aff").arg(basename).toLatin1(),
                                 QString("%1.dic").arg(basename).toLatin1());
        QString encoded = QString(m_checker->get_dic_encoding());
        m_codec = QTextCodec::codecForName(encoded.toLatin1());
    }

    rehighlight();
}

void SongHighlighter::addWord(const QString &word)
{
    QByteArray encodedString;
    QString encoded = QString(m_checker->get_dic_encoding());
    QTextCodec *codec = QTextCodec::codecForName(encoded.toLatin1());
    encodedString = codec->fromUnicode(word);
    m_checker->add(encodedString.data());
    rehighlight();
}

void SongHighlighter::setSpellCheckActive(const bool value)
{
    if (m_isSpellCheckActive != value) {
        m_isSpellCheckActive = value;
        rehighlight();
    }
}

bool SongHighlighter::isSpellCheckActive() const
{
    return m_isSpellCheckActive;
}

Hunspell *SongHighlighter::checker() const { return m_checker; }
#endif // ENABLE_SPELLCHECK
