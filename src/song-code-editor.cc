// Copyright (C) 2009-2012, Romain Goffe <romain.goffe@gmail.com>
// Copyright (C) 2009-2012, Alexandre Dupas <alexandre.dupas@gmail.com>
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
#include "song-code-editor.hh"
#include "song-highlighter.hh"
#include "song.hh"

#include <QSettings>
#include <QTextCodec>
#include <QTextBlock>
#include <QDebug>

CSongCodeEditor::CSongCodeEditor(QWidget *parent)
  : CodeEditor(parent)
  , m_highlighter(0)
{
  setUndoRedoEnabled(true);
  connect(this, SIGNAL(cursorPositionChanged()), SLOT(highlight()));

  CSongHighlighter *highlighter = new CSongHighlighter(document());
  Q_UNUSED(highlighter);
  readSettings();
}

CSongCodeEditor::~CSongCodeEditor()
{
  delete m_highlighter;
}

void CSongCodeEditor::readSettings()
{
  QSettings settings;
  settings.beginGroup("editor");

  QFont font;
  QString fontstr = settings.value("font", QString()).toString();
  if(fontstr.isEmpty())
    {
      font = QFont("Monospace",11);
      font.setStyleHint(QFont::TypeWriter, QFont::PreferAntialias);
    }

  font.fromString(fontstr);
  setFont(font);

  m_verseColor = settings.value("verseColor").value<QColor>();
  m_chorusColor = settings.value("chorusColor").value<QColor>();
  m_scriptureColor = settings.value("scriptureColor").value<QColor>();

  if(!m_verseColor.isValid())
    {
      m_verseColor = QColor(138,226,52).lighter(170);
      m_chorusColor = QColor(138,226,52).lighter(170);
      m_scriptureColor = QColor(138,226,52).lighter(170);
    }


  setHighlightMode(settings.value("highlight", true).toBool());
  setLineNumberMode(settings.value("lines", true).toBool());

  settings.endGroup();
}

void CSongCodeEditor::writeSettings()
{
}

void CSongCodeEditor::installHighlighter()
{
  m_highlighter = new CSongHighlighter(document());
}

void CSongCodeEditor::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Tab)
    indentSelection();
  else
    QPlainTextEdit::keyPressEvent(event);
}

void CSongCodeEditor::highlight()
{
  SongEnvironment env = None;
  QTextCursor cursor(document());
  QStringList lines = toPlainText().split("\n");
  QList<QTextEdit::ExtraSelection> extraSelections;
  foreach (QString line, lines)
    {
      if(Song::reBeginVerse.indexIn(line) > -1)
	env = Verse;
      else if(Song::reBeginChorus.indexIn(line) > -1)
	env = Chorus;
      else if(Song::reBeginScripture.indexIn(line) > -1)
	env = Scripture;

      if( ((env == Verse) && (Song::reEndVerse.indexIn(line) > -1)) ||
	  ((env == Chorus) && (Song::reEndChorus.indexIn(line) > -1)) ||
	  ((env == Scripture) && (Song::reEndScripture.indexIn(line) > -1)) )
	{
	  cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor);
	  extraSelections.append(environmentSelection(env, cursor));
	  cursor.movePosition(QTextCursor::Up, QTextCursor::KeepAnchor);
	  env = None;
	}
      cursor.movePosition(QTextCursor::Down, (env != None)?
			  QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);
    }
  extraSelections.append(currentLineSelection());
  setExtraSelections(extraSelections);
}

QTextEdit::ExtraSelection CSongCodeEditor::environmentSelection(const SongEnvironment & env,
							    const QTextCursor & cursor)
{
  QColor backgroundColor;
  switch(env)
    {
    case Verse:
      backgroundColor = m_verseColor;
      break;
    case Chorus:
      backgroundColor = m_chorusColor;
      break;
    case Scripture:
      backgroundColor = m_scriptureColor;
      break;
    default:
      break;
    }
  QTextEdit::ExtraSelection selection;
  selection.format.setBackground(backgroundColor);
  selection.format.setProperty(QTextFormat::FullWidthSelection, true);
  selection.cursor = cursor;
  return selection;
}

void CSongCodeEditor::indentSelection()
{
  QTextCursor cursor = textCursor();
  QTextCursor it = textCursor();
  it.setPosition(cursor.anchor());

  //swap such as it always points
  //to the beginning of the selection
  if(it > cursor)
    {
      it.setPosition(cursor.position());
      cursor.setPosition(cursor.anchor());
    }

  it.movePosition(QTextCursor::StartOfLine);
  while(it <= cursor)
    {
      indentLine(it);
      it.movePosition(QTextCursor::EndOfLine);
      if(!it.atEnd())
	it.movePosition(QTextCursor::Down);
      else
	break;
    }
}

void CSongCodeEditor::indentLine(const QTextCursor & cur)
{
  //if line only contains whitespaces, remove them and exit
  if(cur.block().text().trimmed().isEmpty() || cur.atStart())
    {
      trimLine(cur);
      return;
    }

  //get the previous non void line
  QTextCursor cursor(cur);
  QString prevLine;
  do
    {
      cursor.movePosition(QTextCursor::Up);
      prevLine = cursor.block().text();
    }
  while(cursor.block().text().trimmed().isEmpty());

  //deduce column from previous line
  int spaces = 0;
  while(prevLine.startsWith(" "))
    {
      prevLine.remove(0,1);
      ++spaces;
    }
  int index = spaces/2;

  //add indentation level if previous line begins with \begin
  if(prevLine.startsWith("\\begin"))
    ++index;

  cursor = cur;
  cursor.movePosition (QTextCursor::StartOfLine);
  //remove indentation level if current line begins with \end
  if(cursor.block().text().contains("\\end") && index!=0)
    --index;

  //performs the correct indentation
  trimLine(cursor);
  for(int i=0; i < index; ++i)
    cursor.insertText("  ");
}

void CSongCodeEditor::trimLine(const QTextCursor & cur)
{
  QTextCursor cursor(cur);
  QString str  = cursor.block().text();
  while( str.startsWith(" ") )
    {
      cursor.movePosition (QTextCursor::StartOfLine);
      cursor.deleteChar();
      str  = cursor.block().text();
    }
}

CSongHighlighter * CSongCodeEditor::highlighter() const
{
  return m_highlighter;
}
