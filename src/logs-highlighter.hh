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
#ifndef __LOGS_HIGHLIGHTER_HH__
#define __LOGS_HIGHLIGHTER_HH__

#include <QSyntaxHighlighter>

#include <QHash>
#include <QTextCharFormat>

class QTextDocument;
class Hunspell;

class CLogsHighlighter : public QSyntaxHighlighter
{
  Q_OBJECT

public:
  CLogsHighlighter(QTextDocument *parent = 0);
  ~CLogsHighlighter();

protected:
  void highlightBlock(const QString &text);

private:
  struct HighlightingRule
  {
    QRegExp pattern;
    QTextCharFormat format;
  };
  QVector<HighlightingRule> highlightingRules;

  QTextCharFormat m_latexFileFormat;
  QTextCharFormat m_latexErrorFormat;
  QTextCharFormat m_latexWarningFormat;
};

#endif // __LOGS_HIGHLIGHTER_HH__
