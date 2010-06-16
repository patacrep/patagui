// Copyright (C) 2010 Romain Goffe, Alexandre Dupas
//
// Songbook Creator is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// Songbook Creator is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
// MA  02110-1301, USA.
//******************************************************************************

#include <QtGui>

#include "highlighter.hh"

Highlighter::Highlighter(QTextDocument *parent)
  : QSyntaxHighlighter(parent)
{
  HighlightingRule rule;

  // Keywords1 (orange)
  keywordFormat.setForeground(QColor(206,92,0));
  keywordFormat.setFontWeight(QFont::Bold);
  QStringList keywordPatterns;
  keywordPatterns << "\\bgtab\\b" << "\\becho\\b"
		  << "\\brep\\b" << "\\blilypond\\b"
		  << "\\bimage\\b" << "\\bsongcolumns\\b"
		  << "\\bcover\\b" << "\\bcapo\\b"
		  << "\\bnolyrics\\b" << "\\bmusicnote\\b"
		  << "\\btextnote\\b" << "\\bdots\\b"
		  << "\\bsingle\\b"  ;

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
  keyword2Patterns << "\\bbar\\b" ;

  foreach (const QString &pattern, keyword2Patterns)
    {
      rule.pattern = QRegExp(pattern);
      rule.format = keyword2Format;
      highlightingRules.append(rule);
    }

  //Environments (bold, green)
  environmentFormat.setFontWeight(QFont::Bold);
  environmentFormat.setForeground(QColor(78,154,6));

  rule.pattern = QRegExp("\\bbegin\\b");
  rule.format = environmentFormat;
  highlightingRules.append(rule);

  rule.pattern = QRegExp("\\bend\\b");
  rule.format = environmentFormat;
  highlightingRules.append(rule);

  rule.pattern = QRegExp("\\bbeginverse\\b");
  rule.format = environmentFormat;
  highlightingRules.append(rule);

  rule.pattern = QRegExp("\\bendverse\\b");
  rule.format = environmentFormat;
  highlightingRules.append(rule);

  rule.pattern = QRegExp("\\bbeginchorus\\b");
  rule.format = environmentFormat;
  highlightingRules.append(rule);

  rule.pattern = QRegExp("\\bendchorus\\b");
  rule.format = environmentFormat;
  highlightingRules.append(rule);

  rule.pattern = QRegExp("\\bbeginsong\\b");
  rule.format = environmentFormat;
  highlightingRules.append(rule);

  rule.pattern = QRegExp("\\bendsong\\b");
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

void Highlighter::highlightBlock(const QString &text)
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
