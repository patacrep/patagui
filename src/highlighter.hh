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
#ifndef __HIGHLIGHTER_HH__
#define __HIGHLIGHTER_HH__

#include <QSyntaxHighlighter>

#include <QHash>
#include <QTextCharFormat>

class QTextDocument;

class Highlighter : public QSyntaxHighlighter
{
  Q_OBJECT

  public:
  Highlighter(QTextDocument *parent = 0);

protected:
  void highlightBlock(const QString &text);

private:
  struct HighlightingRule
  {
    QRegExp pattern;
    QTextCharFormat format;
  };
  QVector<HighlightingRule> highlightingRules;

  QRegExp commentStartExpression;
  QRegExp commentEndExpression;

  QTextCharFormat keywordFormat;
  QTextCharFormat keyword2Format;
  QTextCharFormat environmentFormat;
  QTextCharFormat singleLineCommentFormat;
  QTextCharFormat chordFormat;
  QTextCharFormat quotationFormat;
  QTextCharFormat argumentFormat;
  QTextCharFormat optionFormat;

  QTextCharFormat multiLineCommentFormat;
};

#endif // __HIGHLIGHTER_HH__
