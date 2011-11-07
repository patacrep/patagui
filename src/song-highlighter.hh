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

class CHighlighter : public QSyntaxHighlighter
{
  Q_OBJECT

public:
  CHighlighter(QTextDocument *parent = 0);
  ~CHighlighter();
  void clearCheck();
  QStringList misspelled();
  QStringList correct();
  void enableSpellChecking(const bool state);
  void setSpellCheck(const bool state);
  bool setDictionary(const QString &filename);
  Hunspell* checker() const;

#ifdef ENABLE_SPELL_CHECKING
public slots:
  void addWord(const QString &word);
#endif //ENABLE_SPELL_CHECKING

protected:
  void highlightBlock(const QString &text);
  void spellCheck(const QString &text);
  bool checkWord(QString word);


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

  Hunspell * m_checker;
  bool m_isSpellCheckActive;
  QTextCharFormat m_spellCheckFormat;
  QTextCodec *m_codec;
};

#endif // __HIGHLIGHTER_HH__
