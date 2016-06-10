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

/**
 * \file logs-highlighter.hh
 * \class LogsHighlighter
 * \brief LogsHighlighter provides colors and highlights for the logs widget.
 *
 * Highlights include filenames and errors/warnings that are output during
 * the LaTeX compilation of a songbook.
 *
 */
class LogsHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    /// Constructor
    LogsHighlighter(QTextDocument *parent = 0);
    /// Destructor
    ~LogsHighlighter();

protected:
    /// Apply highlighting rules for LaTeX logs compilation output
    void highlightBlock(const QString &text);

private:
    struct HighlightingRule {
        QRegExp pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat m_latexFileFormat;
    QTextCharFormat m_latexErrorFormat;
    QTextCharFormat m_latexWarningFormat;
};

#endif // __LOGS_HIGHLIGHTER_HH__
