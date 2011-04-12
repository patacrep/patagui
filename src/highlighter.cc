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

#include "highlighter.hh"

CHighlighter::CHighlighter(QTextDocument *parent)
  : QSyntaxHighlighter(parent)
{
  HighlightingRule rule;

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

  //LaTeX compilation logs
  //files (light blue)
  QStringList extensions;
  extensions << "pdf" << "jpg" << "png";
  m_latexFileFormat.setForeground(QColor(114,159,207));

  foreach (const QString &extension, extensions)
    {
      rule.pattern = QRegExp(QString("[^(\\s|/)]*\\.%1").arg(extension));
      rule.format = m_latexFileFormat;
      highlightingRules.append(rule);
    }
  
  //errors (light red)
  m_latexErrorFormat.setForeground(QColor(239,41,41));

  rule.pattern = QRegExp("^!.*");
  rule.format = m_latexErrorFormat;
  highlightingRules.append(rule);

  //warnings (light orange)
  m_latexWarningFormat.setForeground(QColor(252,175,62));

  rule.pattern = QRegExp("^.*(W|w)arning.*$");
  rule.format = m_latexWarningFormat;
  highlightingRules.append(rule);

  //Environments (bold, green)
  environmentFormat.setFontWeight(QFont::Bold);
  environmentFormat.setForeground(QColor(78,154,6));

  rule.pattern = QRegExp("\\\\begin");
  rule.format = environmentFormat;
  highlightingRules.append(rule);

  rule.pattern = QRegExp("\\\\end");
  rule.format = environmentFormat;
  highlightingRules.append(rule);

  rule.pattern = QRegExp("\\\\beginverse");
  rule.format = environmentFormat;
  highlightingRules.append(rule);

  rule.pattern = QRegExp("\\\\endverse");
  rule.format = environmentFormat;
  highlightingRules.append(rule);

  rule.pattern = QRegExp("\\\\beginchorus");
  rule.format = environmentFormat;
  highlightingRules.append(rule);

  rule.pattern = QRegExp("\\\\endchorus");
  rule.format = environmentFormat;
  highlightingRules.append(rule);

  rule.pattern = QRegExp("\\\\beginsong");
  rule.format = environmentFormat;
  highlightingRules.append(rule);

  rule.pattern = QRegExp("\\\\endsong");
  rule.format = environmentFormat;
  highlightingRules.append(rule);

  rule.pattern = QRegExp("\\\\beginscripture");
  rule.format = environmentFormat;
  highlightingRules.append(rule);

  rule.pattern = QRegExp("\\\\endscripture");
  rule.format = environmentFormat;
  highlightingRules.append(rule);

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

  //LaTeX options (overrided by chords)
  optionFormat.setFontItalic(true);
  rule.pattern = QRegExp("\\[([^\\]]+)\\]");
  rule.format = optionFormat;
  highlightingRules.append(rule);

  //Chords (blue)
  chordFormat.setForeground(QColor(32,74,135));
  chordFormat.setFontWeight(QFont::Bold);
  rule.pattern = QRegExp("\\\\\\[([^\\]]+)\\]");
  rule.format = chordFormat;
  highlightingRules.append(rule);

  //LaTeX args (bold)
  argumentFormat.setFontWeight(QFont::Bold);
  rule.pattern = QRegExp("\\{([^}]+)\\}");
  rule.format = argumentFormat;
  highlightingRules.append(rule);

  //todo: remove
  commentStartExpression = QRegExp("/\\*");
  commentEndExpression = QRegExp("\\*/");

}

void CHighlighter::highlightBlock(const QString &text)
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
}
