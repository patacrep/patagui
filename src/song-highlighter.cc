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

#include <QtGui>

#include "config.hh"
#include "song-highlighter.hh"
#ifdef ENABLE_SPELL_CHECKING
#include "hunspell/hunspell.hxx"
#endif //ENABLE_SPELL_CHECKING

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
  keywordPatterns << "\\\\gtab"     << "\\\\echo"
		  << "\\\\rep"      << "\\\\lilypond"
		  << "\\\\image"    << "\\\\songcolumns"
		  << "\\\\cover"    << "\\\\capo"
		  << "\\\\nolyrics" << "\\\\musicnote"
		  << "\\\\textnote" << "\\\\dots"
		  << "\\\\single"  << "\\\\emph"
		  << "\\\\selectlanguage";

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
  keyword2Patterns << "\\\\bar";

  foreach (const QString &pattern, keyword2Patterns)
    {
      rule.pattern = QRegExp(pattern);
      rule.format = keyword2Format;
      highlightingRules.append(rule);
    }

  //Environments (bold, green)
  environmentFormat.setFontWeight(QFont::Bold);
  environmentFormat.setForeground(QColor(78,154,6));

  QStringList regexps;
  regexps << "\\\\begin" << "\\\\end"
	  << "\\\\beginverse" << "\\\\endverse"
	  << "\\\\beginchorus" << "\\\\endchorus"
	  << "\\\\beginsong" << "\\\\endsong"
	  << "\\\\beginscripture" << "\\\\endscripture";

  foreach (const QString &regexp, regexps)
    {
      rule.pattern = QRegExp(regexp);
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

  //todo: remove
  commentStartExpression = QRegExp("/\\*");
  commentEndExpression = QRegExp("\\*/");

#ifdef ENABLE_SPELL_CHECKING
  //Settings for online spellchecking
  m_spellCheckFormat.setUnderlineColor(QColor(Qt::red));
  m_spellCheckFormat.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
#endif //ENABLE_SPELL_CHECKING
}

CSongHighlighter::~CSongHighlighter()
{
#ifdef ENABLE_SPELL_CHECKING
  delete m_checker;
#endif //ENABLE_SPELL_CHECKING
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

  int startIndex = 0;
  if (previousBlockState() != 1)
    startIndex = commentStartExpression.indexIn(text);

  while (startIndex >= 0) {
    int endIndex = commentEndExpression.indexIn(text, startIndex);
    int commentLength;
    if (endIndex == -1) {
      setCurrentBlockState(1);
      commentLength = text.length() - startIndex;
    } else {
      commentLength = endIndex - startIndex
	+ commentEndExpression.matchedLength();
    }
    setFormat(startIndex, commentLength, multiLineCommentFormat);
    startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
  }

#ifdef ENABLE_SPELL_CHECKING
  spellCheck(text);
#endif //ENABLE_SPELL_CHECKING
}

#ifdef ENABLE_SPELL_CHECKING
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
  QFileInfo fi(filename);
  if(filename.isEmpty() || !fi.exists() || !fi.isReadable())
    {
      qWarning() << tr("CSongHighlighter::setDictionary cannot read open dictionary : ") << filename;
      return;
    }

  QString basename = QString("%1/%2").arg(fi.absolutePath()).arg(fi.baseName());
  if(m_checker)
    {
      delete m_checker;
      m_checker = 0;
    }
  m_checker = new Hunspell(QString("%1.aff").arg(basename).toLatin1(),
			   QString("%1.dic").arg(basename).toLatin1());
  QString encoded = QString(m_checker->get_dic_encoding());
  m_codec = QTextCodec::codecForName(encoded.toLatin1());

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

void CSongHighlighter::setSpellCheck(const bool value)
{
  if(m_isSpellCheckActive != value)
    {
      m_isSpellCheckActive = value;
      rehighlight();
    }
}

Hunspell* CSongHighlighter::checker() const
{
  return m_checker;
}
#endif //ENABLE_SPELL_CHECKING
