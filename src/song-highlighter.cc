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
#endif //ENABLE_SPELLCHECK

#include <QDebug>

CSongHighlighter::CSongHighlighter(QTextDocument *parent)
  : QSyntaxHighlighter(parent)
  , m_checker(NULL)
  , m_isSpellCheckActive(false)
{
  HighlightingRule rule;

  //LaTeX options (overrided by chords)
  optionFormat.setFontItalic(true);
  rule.pattern = QRegExp("\\[([^\\]]+)\\]");
  rule.format = optionFormat;
  highlightingRules.append(rule);

  //LaTeX args (bold)
  argumentFormat.setFontWeight(QFont::Bold);
  rule.pattern = QRegExp("\\{([^}]+)\\}");
  rule.format = argumentFormat;
  highlightingRules.append(rule);

  // Keywords1 (orange)
  keywordFormat.setForeground(QColor(206,92,0));
  keywordFormat.setFontWeight(QFont::Bold);
  QStringList keywordPatterns;
  keywordPatterns << "\\\\gtab"     << "\\\\utab"
		  << "\\\\rep"      << "\\\\lilypond"
		  << "\\\\image"    << "\\\\songcolumns"
		  << "\\\\cover"    << "\\\\capo"
		  << "\\\\nolyrics" << "\\\\musicnote"
		  << "\\\\textnote" << "\\\\dots"
		  << "\\\\single"  << "\\\\echo"
		  << "\\\\transpose" << "\\\\transposition"
		  << "\\\\emph" << "\\\\selectlanguage";

  foreach (const QString &pattern, keywordPatterns)
    {
      rule.pattern = QRegExp(pattern);
      rule.format = keywordFormat;
      highlightingRules.append(rule);
    }

  // Keywords2 (red)
  keyword2Format.setForeground(QColor(164,0,0));
  keyword2Format.setFontWeight(QFont::Bold);
  QStringList keyword2Patterns;
  keyword2Patterns << "\\\\bar"
		   << "\\\\Intro" << "\\\\Rythm"
		   << "\\\\Outro" << "\\\\Bridge"
		   << "\\\\Verse" << "\\\\Chorus"
		   << "\\\\Pattern" << "\\\\Solo"
		   << "\\\\Adlib" << "\\\\else"
		   << "\\\\ifchorded" << "\\\\iflyrics"
		   << "\\\\ifnorepeatchords" << "\\\\fi";

  foreach (const QString &pattern, keyword2Patterns)
    {
      rule.pattern = QRegExp(pattern);
      rule.format = keyword2Format;
      highlightingRules.append(rule);
    }

  //Environments (bold, green)
  environmentFormat.setFontWeight(QFont::Bold);
  environmentFormat.setForeground(QColor(78,154,6));

  QList<QRegExp> regexps;
  regexps << Song::reBegin << Song::reEnd
	  << Song::reBeginScripture << Song::reEndScripture;

  foreach (const QRegExp &regexp, regexps)
    {
      rule.pattern = regexp;
      rule.format = environmentFormat;
      highlightingRules.append(rule);
    }

  //Comments (grey)
  singleLineCommentFormat.setForeground(QColor(136,138,133));
  rule.pattern = QRegExp("%[^\n]*");
  rule.format = singleLineCommentFormat;
  highlightingRules.append(rule);

  //Quotations (violet)
  quotationFormat.setForeground(QColor(92,53,102));
  rule.pattern = QRegExp("\".*\"");
  rule.format = quotationFormat;
  highlightingRules.append(rule);

  rule.pattern = QRegExp("``.*''");
  rule.format = quotationFormat;
  highlightingRules.append(rule);

  //Chords (blue)
  chordFormat.setForeground(QColor(32,74,135));
  chordFormat.setFontWeight(QFont::Bold);
  rule.pattern = QRegExp("\\\\\\[([^\\]]+)\\]");
  rule.format = chordFormat;
  highlightingRules.append(rule);

#ifdef ENABLE_SPELLCHECK
  //Settings for online spellchecking
  m_spellCheckFormat.setUnderlineColor(QColor(Qt::red));
  m_spellCheckFormat.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
#endif //ENABLE_SPELLCHECK
}

CSongHighlighter::~CSongHighlighter()
{
#ifdef ENABLE_SPELLCHECK
  delete m_checker;
#endif //ENABLE_SPELLCHECK
}

void CSongHighlighter::highlightBlock(const QString &text)
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
#endif //ENABLE_SPELLCHECK
}

#ifdef ENABLE_SPELLCHECK
void CSongHighlighter::spellCheck(const QString &text)
{
  if (!m_isSpellCheckActive)
    return;

  QString str = text.simplified();
  if (str.isEmpty())
    return;

  QStringList list = str.split(QRegExp("([^\\w,^\\\\]|(?=\\\\))+"),
			       QString::SkipEmptyParts);
  int l, number;
  foreach (str, list)
    if (str.length()>1 && !str.startsWith('\\') && !checkWord(str))
      {
	number = text.count(QRegExp("\\b" + str + "\\b"));
	l=-1;
	// underline all incorrect occurences of misspelled word
	for (int j=0; j < number; ++j)
	  {
	    l = text.indexOf(QRegExp("\\b" + str + "\\b"),l+1);
	    if (l>=0)
	      setFormat(l, str.length(), m_spellCheckFormat);
	  }
      }
}

bool CSongHighlighter::checkWord(const QString &word)
{
  int check;
  QByteArray encodedString;
  encodedString = m_codec->fromUnicode(word);
  check = m_checker->spell(encodedString.data());
  return bool(check);
}

void CSongHighlighter::setDictionary(const QString &filename)
{
  if (m_checker)
    {
      delete m_checker;
      m_checker = 0;
    }

  QFileInfo fi(filename);
  if (filename.isEmpty() || !fi.exists() || !fi.isReadable())
    {
      qWarning() << tr("CSongHighlighter::setDictionary cannot read open dictionary : ") << filename;
    }
  else
    {
      QString basename = QString("%1/%2").arg(fi.absolutePath()).arg(fi.baseName());
      m_checker = new Hunspell(QString("%1.aff").arg(basename).toLatin1(),
			       QString("%1.dic").arg(basename).toLatin1());
      QString encoded = QString(m_checker->get_dic_encoding());
      m_codec = QTextCodec::codecForName(encoded.toLatin1());
    }

  rehighlight();
}

void CSongHighlighter::addWord(const QString & word)
{
  QByteArray encodedString;
  QString encoded = QString(m_checker->get_dic_encoding());
  QTextCodec *codec = QTextCodec::codecForName(encoded.toLatin1());
  encodedString = codec->fromUnicode(word);
  m_checker->add(encodedString.data());
  rehighlight();
}

void CSongHighlighter::setSpellCheckActive(const bool value)
{
  if (m_isSpellCheckActive != value)
    {
      m_isSpellCheckActive = value;
      rehighlight();
    }
}

Hunspell* CSongHighlighter::checker() const
{
  return m_checker;
}
#endif //ENABLE_SPELLCHECK
