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

#include "logs-highlighter.hh"
#include "utils/tango-colors.hh"

#include <QDebug>

CLogsHighlighter::CLogsHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    //LaTeX compilation logs
    //files (light blue)
    QStringList extensions;
    extensions << "pdf" << "jpg" << "png" << "ly" << "sg";
    m_latexFileFormat.setForeground(_TangoSkyBlue1);

    foreach (const QString &extension, extensions)
    {
        rule.pattern = QRegExp(QString("[^(\\s|/)]*\\.%1").arg(extension));
        rule.format = m_latexFileFormat;
        highlightingRules.append(rule);
    }

    //errors (light red)
    m_latexErrorFormat.setForeground(_TangoScarletRed1);

    rule.pattern = QRegExp("^!.*");
    rule.format = m_latexErrorFormat;
    highlightingRules.append(rule);

    //warnings (light orange)
    m_latexWarningFormat.setForeground(_TangoOrange1);

    rule.pattern = QRegExp("^.*(W|w)arning.*$");
    rule.format = m_latexWarningFormat;
    highlightingRules.append(rule);

}

CLogsHighlighter::~CLogsHighlighter()
{}

void CLogsHighlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules)
    {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0)
        {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
    setCurrentBlockState(0);
}
