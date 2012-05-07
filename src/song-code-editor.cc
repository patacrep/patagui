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

#include <QCompleter>
#include <QAbstractItemView>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QScrollBar>

CSongCodeEditor::CSongCodeEditor(QWidget *parent)
  : CodeEditor(parent)
  , m_highlighter(0)
  , m_completer(0)
  , m_environmentsHighlighted(true)
  , m_verseColor(QColor(138,226,52).lighter(180))
  , m_chorusColor(QColor(252,175,62).lighter(160))
  , m_bridgeColor(QColor(114,159,207).lighter(170))
  , m_scriptureColor(QColor(173,127,168).lighter(170))
{
  setUndoRedoEnabled(true);
  connect(this, SIGNAL(cursorPositionChanged()), SLOT(highlightEnvironments()));

  CSongHighlighter *highlighter = new CSongHighlighter(document());
  Q_UNUSED(highlighter);

  QStringList wordList = QStringList()
    << "\\begin{verse}" << "\\end{verse}"
    << "\\begin{verse*}" << "\\end{verse*}"
    << "\\begin{chorus}" << "\\end{chorus}"
    << "\\begin{bridge}" << "\\end{bridge}"
    << "\\rep"      << "\\echo"
    << "\\image"    <<  "\\nolyrics"
    << "\\musicnote" << "\\textnote"
    << "\\dots"  << "\\lilypond"
    << "\\Intro" << "\\Rythm"
    << "\\Outro" << "\\Bridge"
    << "\\Verse" << "\\Chorus"
    << "\\Pattern" << "\\Solo";

  m_completer = new QCompleter(wordList, this);
  m_completer->setWidget(this);
  m_completer->setCompletionMode(QCompleter::PopupCompletion);
  QObject::connect(m_completer, SIGNAL(activated(QString)),
		   this, SLOT(insertCompletion(QString)));
  readSettings();
}

CSongCodeEditor::~CSongCodeEditor()
{
  delete m_highlighter;
  delete m_completer;
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

  setEnvironmentsHighlighted(settings.value("color-environments", true).toBool());
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

void CSongCodeEditor::insertCompletion(const QString& completion)
{
  if (completer()->widget() != this)
    return;
  QTextCursor cursor = textCursor();
  int extra = completion.length() - completer()->completionPrefix().length();
  cursor.movePosition(QTextCursor::Left);
  cursor.movePosition(QTextCursor::EndOfWord);
  cursor.insertText(completion.right(extra));
  setTextCursor(cursor);
}

QString CSongCodeEditor::textUnderCursor() const
{
  QTextCursor cursor = textCursor();
  cursor.select(QTextCursor::WordUnderCursor);
  return cursor.selectedText();
}

void CSongCodeEditor::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Tab)
    {
      indentSelection();
      return;
    }
  else if (completer() && completer()->popup()->isVisible())
    {
      // The following keys are forwarded by the completer to the widget
      switch (event->key())
	{
	case Qt::Key_Enter:
	case Qt::Key_Return:
	case Qt::Key_Escape:
	case Qt::Key_Backtab:
	  event->ignore();
	  return; // let the completer do default behavior
	default:
	  break;
	}
    }

  bool isShortcut = ((event->modifiers() & Qt::ControlModifier) && event->key() == Qt::Key_Space); // CTRL+Space
  if (!completer() || !isShortcut) // do not process the shortcut when we have a completer
    QPlainTextEdit::keyPressEvent(event);

  const bool ctrlOrShift = event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
  if (!completer() || (ctrlOrShift && event->text().isEmpty()))
    return;

  static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
  bool hasModifier = (event->modifiers() != Qt::NoModifier) && !ctrlOrShift;
  QString completionPrefix = textUnderCursor();

  if (!isShortcut && (hasModifier || event->text().isEmpty()|| completionPrefix.length() < 3
		      || eow.contains(event->text().right(1))))
    {
      completer()->popup()->hide();
      return;
    }

  if (completionPrefix != completer()->completionPrefix())
    {
      completer()->setCompletionPrefix(completionPrefix);
      completer()->popup()->setCurrentIndex(completer()->completionModel()->index(0, 0));
    }
  QRect cr = cursorRect();
  cr.setWidth(completer()->popup()->sizeHintForColumn(0)
	      + completer()->popup()->verticalScrollBar()->sizeHint().width());
  completer()->complete(cr); // popup it up!
}

void CSongCodeEditor::highlightEnvironments()
{
  if(!environmentsHighlighted())
    return;

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
      else if(Song::reBeginBridge.indexIn(line) > -1)
	env = Bridge;
      else if(Song::reBeginScripture.indexIn(line) > -1)
	env = Scripture;

      if( ((env == Verse) && (Song::reEndVerse.indexIn(line) > -1)) ||
	  ((env == Bridge) && (Song::reEndBridge.indexIn(line) > -1)) ||
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
    case Bridge:
      backgroundColor = m_bridgeColor;
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

QCompleter * CSongCodeEditor::completer() const
{
  return m_completer;
}

bool CSongCodeEditor::environmentsHighlighted() const
{
  return m_environmentsHighlighted;
}

void CSongCodeEditor::setEnvironmentsHighlighted(bool value)
{
  m_environmentsHighlighted = value;
}
