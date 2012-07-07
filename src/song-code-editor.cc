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
#include "search-widget.hh"

#ifdef ENABLE_SPELLCHECK
#include "hunspell/hunspell.hxx"
#endif //ENABLE_SPELLCHECK

#include <QtGlobal>
#include <QSettings>
#include <QTextCodec>
#include <QTextBlock>
#include <QDebug>

#include <QCompleter>
#include <QAbstractItemView>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QScrollBar>
#include <QKeyEvent>
#include <QResizeEvent>

#include <QAction>
#include <QMenu>

CSongCodeEditor::CSongCodeEditor(QWidget *parent)
  : CodeEditor(parent)
  , m_highlighter(0)
  , m_completer(0)
  , m_environmentsHighlighted(true)
  , m_verseColor(QColor(138,226,52).lighter(180))
  , m_chorusColor(QColor(252,175,62).lighter(160))
  , m_bridgeColor(QColor(114,159,207).lighter(170))
  , m_scriptureColor(QColor(173,127,168).lighter(170))
  , m_isSpellCheckAvailable(false)
  , m_isSpellCheckActive(false)
#ifdef ENABLE_SPELLCHECK
  , m_maxSuggestedWords(0)
#endif
  , m_quickSearch(new CSearchWidget(this))
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
    << "\\begin{repeatedchords}" << "\\end{repeatedchords}"
    << "\\beginscripture" << "\\endscripture"
    << "\\rep"      << "\\echo"
    << "\\image"    <<  "\\nolyrics"
    << "\\musicnote" << "\\textnote"
    << "\\dots"  << "\\lilypond"
    << "\\Intro" << "\\Rythm"
    << "\\Outro" << "\\Bridge"
    << "\\Verse" << "\\Chorus"
    << "\\Pattern" << "\\Solo"
    << "\\Adlib" << "\\emph"
    << "\\ifchorded" << "\\ifnorepeatchords"
    << "\\else" << "\\fi";

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
  delete m_quickSearch;
}

void CSongCodeEditor::readSettings()
{
  QSettings settings;
  settings.beginGroup("editor");

  QFont font;
  QString fontstr = settings.value("font", QString()).toString();
  if (fontstr.isEmpty())
    {
      font = QFont("Monospace",11);
      font.setStyleHint(QFont::TypeWriter, QFont::PreferAntialias);
    }

  font.fromString(fontstr);
  setFont(font);

  setEnvironmentsHighlighted(settings.value("color-environments", true).toBool());
  setHighlightMode(settings.value("highlight", true).toBool());
  setLineNumberMode(settings.value("lines", true).toBool());

#ifdef ENABLE_SPELLCHECK
  m_maxSuggestedWords = settings.value("maxSuggestedWords", 5).toUInt();
  for (uint i = 0; i < m_maxSuggestedWords; ++i)
    {
      QAction *action = new QAction(this);
      action->setVisible(false);
      connect(action, SIGNAL(triggered()), this, SLOT(correctWord()));
      m_misspelledWordsActs.append(action);
    }
#endif //ENABLE_SPELLCHECK

  settings.endGroup();
}

void CSongCodeEditor::writeSettings()
{
}

void CSongCodeEditor::installHighlighter()
{
  m_highlighter = new CSongHighlighter(document());
}

void CSongCodeEditor::insertVerse()
{
  QString selection = textCursor().selectedText();
  insertPlainText(QString("\n\\begin{verse}\n%1\n\\end{verse}\n").arg(selection)  );
}

void CSongCodeEditor::insertChorus()
{
  QString selection = textCursor().selectedText();
  insertPlainText(QString("\n\\begin{chorus}\n%1\n\\end{chorus}\n").arg(selection)  );
}

void CSongCodeEditor::insertBridge()
{
  QString selection = textCursor().selectedText();
  insertPlainText(QString("\n\\begin{bridge}\n%1\n\\end{bridge}\n").arg(selection)  );
}

void CSongCodeEditor::insertCompletion(const QString& completion)
{
  Q_ASSERT(completer()->widget() == this);
  QTextCursor cursor = textCursor();
  int extra = completion.length() - completer()->completionPrefix().length();
  cursor.insertText(completion.right(extra));
  setTextCursor(cursor);
}

QString CSongCodeEditor::textUnderCursor() const
{
  QTextCursor tc = textCursor();
  if (tc.atBlockStart())
    return QString();

  static QSet<QChar> delimiters;
  if ( delimiters.isEmpty() )
    {
      delimiters.insert( QChar::fromAscii(',') );
      delimiters.insert( QChar::fromAscii('!') );
      delimiters.insert( QChar::fromAscii('?') );
      delimiters.insert( QChar::fromAscii('.') );
      delimiters.insert( QChar::fromAscii(';') );
      delimiters.insert( QChar::fromAscii('{') );
      delimiters.insert( QChar::fromAscii('}') );
    }

  tc.anchor();
  while ( 1 )
    {
      // the '-1' comes from the TextCursor always being placed between characters
      int pos = tc.position() - 1;
      if ( pos < 0 )
	break;

      QChar ch = document()->characterAt(pos);
      if ( ch.isSpace() || delimiters.contains(ch) )
	break;

      if (tc.atBlockStart())
	break;

      tc.movePosition( QTextCursor::Left, QTextCursor::KeepAnchor );
    }
  return tc.selectedText().trimmed();
}

void CSongCodeEditor::keyPressEvent(QKeyEvent *event)
{
  if (completer() && completer()->popup()->isVisible())
    {
      // The following keys are forwarded by the completer to the widget
      switch (event->key())
	{
	case Qt::Key_Enter:
	case Qt::Key_Return:
	case Qt::Key_Escape:
	case Qt::Key_Tab:
	case Qt::Key_Backtab:
	  event->ignore();
	  return; // let the completer do default behavior
	default:
	  break;
	}
    }
  else if (event->key() == Qt::Key_Tab)
    {
      indentSelection();
      return;
    }


  bool isShortcut = ((event->modifiers() & Qt::ControlModifier) && event->key() == Qt::Key_Space); // CTRL+Space
  if (!completer() || !isShortcut) // do not process the shortcut when we have a completer
    QPlainTextEdit::keyPressEvent(event);

  const bool ctrlOrShift = event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
  if (!completer() || (ctrlOrShift && event->text().isEmpty()))
    return;

  static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]-="); // end of word

  bool hasModifier = (event->modifiers() != Qt::NoModifier) && !ctrlOrShift;
  QString completionPrefix = textUnderCursor();

  if (!isShortcut && (hasModifier || event->text().isEmpty() || completionPrefix.length() < 1
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

void CSongCodeEditor::resizeEvent(QResizeEvent *event)
{
  Q_UNUSED(event);
  m_quickSearch->move(width() - 345, 4);
}

void CSongCodeEditor::toggleQuickSearch()
{
  m_quickSearch->setVisible(!m_quickSearch->isVisible());
}

void CSongCodeEditor::highlightEnvironments()
{
  if (!environmentsHighlighted())
    return;

  SongEnvironment env = None;
  QTextCursor cursor(document());
  QStringList lines = toPlainText().split("\n");
  QList<QTextEdit::ExtraSelection> extraSelections;
  foreach (QString line, lines)
    {
      if (Song::reBeginVerse.indexIn(line) > -1)
	env = Verse;
      else if (Song::reBeginChorus.indexIn(line) > -1)
	env = Chorus;
      else if (Song::reBeginBridge.indexIn(line) > -1)
	env = Bridge;
      else if (Song::reBeginScripture.indexIn(line) > -1)
	env = Scripture;

      if (((env == Verse) && (Song::reEndVerse.indexIn(line) > -1)) ||
	  ((env == Bridge) && (Song::reEndBridge.indexIn(line) > -1)) ||
	  ((env == Chorus) && (Song::reEndChorus.indexIn(line) > -1)) ||
	  ((env == Scripture) && (Song::reEndScripture.indexIn(line) > -1)))
	{
	  cursor.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
	  extraSelections.append(environmentSelection(env, cursor));
	  cursor.movePosition(QTextCursor::PreviousBlock, QTextCursor::KeepAnchor);
	  env = None;
	}
      cursor.movePosition(QTextCursor::NextBlock, (env != None)?
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

void CSongCodeEditor::indent()
{
  QTextCursor cursor = textCursor();
  cursor.beginEditBlock();
  cursor.movePosition(QTextCursor::Start);
  while (!cursor.atEnd())
    {
      indentLine(cursor);
      cursor.movePosition(QTextCursor::NextBlock);
      cursor.movePosition(QTextCursor::EndOfBlock);
    }
  cursor.endEditBlock();
}

void CSongCodeEditor::indentSelection()
{
  QTextCursor cursor = textCursor();
  cursor.beginEditBlock();
  QTextCursor it = textCursor();
  it.setPosition(cursor.anchor());

  //swap such as the cursor "it" always points
  //to the beginning of the selection
  if (it > cursor)
    {
      it.setPosition(cursor.position());
      cursor.setPosition(cursor.anchor());
    }

  it.movePosition(QTextCursor::StartOfBlock);
  while (it <= cursor)
    {
      indentLine(it);
      it.movePosition(QTextCursor::EndOfBlock);
      if (!it.atEnd())
	it.movePosition(QTextCursor::NextBlock);
      else
	break;
    }
  cursor.endEditBlock();
}

void CSongCodeEditor::indentLine(const QTextCursor & cur)
{
  //if line only contains whitespaces, remove them and exit
  if (cur.block().text().trimmed().isEmpty() || cur.atStart())
    {
      trimLine(cur);
      return;
    }

  //get the previous non void line
  QTextCursor cursor(cur);
  QString prevLine;
  do
    {
      if (cursor.atStart())
	return;
      cursor.movePosition(QTextCursor::Up);
      prevLine = cursor.block().text();
    }
  while (cursor.block().text().trimmed().isEmpty());

  //deduce column from previous line
  int spaces = 0;
  while (prevLine.startsWith(" "))
    {
      prevLine.remove(0,1);
      ++spaces;
    }
  int index = spaces/2;

  //add indentation level if previous line begins with \begin
  if (prevLine.startsWith("\\begin"))
    ++index;

  cursor = cur;
  cursor.movePosition (QTextCursor::StartOfBlock);
  //remove indentation level if current line begins with \end
  if (cursor.block().text().contains("\\end") && index!=0)
    --index;

  //performs the correct indentation
  trimLine(cursor);
  for (int i=0; i < index; ++i)
    cursor.insertText("  ");
}

void CSongCodeEditor::trimLine(const QTextCursor & cur)
{
  QTextCursor cursor(cur);
  QString str  = cursor.block().text();
  while ( str.startsWith(" ") )
    {
      cursor.deleteChar();
      str  = cursor.block().text();
    }
}

#ifdef ENABLE_SPELLCHECK
void CSongCodeEditor::setDictionary(const QString & dictionary)
{
  if (highlighter() == 0)
    return;

  highlighter()->setDictionary(dictionary);
  connect(this, SIGNAL(wordAdded(const QString&)), highlighter(), SLOT(addWord(const QString&)));
}

QString CSongCodeEditor::currentWord()
{
  QTextCursor cursor = cursorForPosition(m_lastPos);
  QString word = cursor.block().text();
  int pos = cursor.columnNumber();
  int end = word.indexOf(QRegExp("\\W+"),pos);
  int begin = word.left(pos).lastIndexOf(QRegExp("\\W+"),pos);
  word = word.mid(begin+1,end-begin-1);
  return word;
}

void CSongCodeEditor::correctWord()
{
  QAction *action = qobject_cast<QAction *>(sender());
  if (action)
    {
      QString replacement = action->text();
      QTextCursor cursor = cursorForPosition(m_lastPos);
      QString zeile = cursor.block().text();
      cursor.select(QTextCursor::WordUnderCursor);
      cursor.deleteChar();
      cursor.insertText(replacement);
    }
}

QStringList CSongCodeEditor::getWordPropositions(const QString &word)
{
  if (!checker())
    return QStringList();

  QStringList wordList;
  QByteArray encodedString;
  QString spell_encoding=QString(checker()->get_dic_encoding());
  QTextCodec *codec = QTextCodec::codecForName(spell_encoding.toLatin1());
  encodedString = codec->fromUnicode(word);

  if (checker()->spell(encodedString.data()))
    return wordList;

  char ** wlst;
  int ns = checker()->suggest(&wlst, encodedString.data());
  if (ns > 0)
    {
      for (int i=0; i < ns; i++)
	wordList.append(codec->toUnicode(wlst[i]));
      checker()->free_list(&wlst, ns);
    }
  return wordList;
}
#endif //ENABLE_SPELLCHECK

void CSongCodeEditor::contextMenuEvent(QContextMenuEvent *event)
{
  QMenu *menu = createStandardContextMenu();

  menu->addSeparator();
  QAction *action = new QAction(tr("Comment selection"), this);
  action->setStatusTip(tr("Comment the selection"));
  connect(action, SIGNAL(triggered()), SLOT(commentSelection()));
  menu->addAction(action);

  action = new QAction(tr("Uncomment selection"), this);
  action->setStatusTip(tr("Uncomment the selection"));
  connect(action, SIGNAL(triggered()), SLOT(uncommentSelection()));
  menu->addAction(action);

#ifdef ENABLE_SPELLCHECK
  if (isSpellCheckActive())
    {
      menu->addSeparator();
      QMenu *spellMenu = new QMenu(tr("Suggestions"));
      m_lastPos=event->pos();
      QString str = currentWord();
      QStringList list = getWordPropositions(str);
      int size = qMin(m_maxSuggestedWords, (uint)list.size());
      if (!list.isEmpty())
	{
	  for (int i = 0; i < size; ++i)
	    {
	      m_misspelledWordsActs[i]->setText(list[i].trimmed());
	      m_misspelledWordsActs[i]->setVisible(true);
	      spellMenu->addAction(m_misspelledWordsActs[i]);
	    }
	  spellMenu->addSeparator();
	  spellMenu->addAction(tr("Add"), this, SLOT(addWord()));
	  spellMenu->addAction(tr("Ignore"), this, SLOT(ignoreWord()));
	  menu->addMenu(spellMenu);
	}
    }
#endif //ENABLE_SPELLCHECK

  menu->exec(event->globalPos());
  delete menu;
}

#ifdef ENABLE_SPELLCHECK
void CSongCodeEditor::ignoreWord()
{
  QString str = currentWord();
  QByteArray encodedString;
  QString spell_encoding=QString(checker()->get_dic_encoding());
  QTextCodec *codec = QTextCodec::codecForName(spell_encoding.toLatin1());
  encodedString = codec->fromUnicode(str);
  checker()->add(encodedString.data());
  emit wordAdded(str);
}

void CSongCodeEditor::addWord()
{
  QString str = currentWord();
  QByteArray encodedString;
  QString spell_encoding=QString(checker()->get_dic_encoding());
  QTextCodec *codec = QTextCodec::codecForName(spell_encoding.toLatin1());
  encodedString = codec->fromUnicode(str);
  checker()->add(encodedString.data());
  m_addedWords.append(str);
  emit wordAdded(str);
}

Hunspell* CSongCodeEditor::checker() const
{
  if (!highlighter()) return 0;
  return highlighter()->checker();
}
#endif //ENABLE_SPELLCHECK

void CSongCodeEditor::commentSelection()
{
  QTextCursor cursor = textCursor();
  QStringList selection = cursor.selectedText().split(QChar(0x2029));
  QStringList commentedSelection;
  foreach (QString line, selection)
    commentedSelection << line.prepend("%");
  cursor.removeSelectedText();
  cursor.insertText(commentedSelection.join("\n"));
}

void CSongCodeEditor::uncommentSelection()
{
  QTextCursor cursor = textCursor();
  QStringList selection = cursor.selectedText().split(QChar(0x2029));
  QStringList uncommentedSelection;
  foreach (QString line, selection)
    if (line.trimmed().startsWith("%"))
      uncommentedSelection << line.trimmed().remove(0,1);
    else
      uncommentedSelection << line;
  cursor.removeSelectedText();
  cursor.insertText(uncommentedSelection.join("\n"));
}

bool CSongCodeEditor::isSpellCheckAvailable() const
{
  return m_isSpellCheckAvailable;
}

void CSongCodeEditor::setSpellCheckAvailable(const bool value)
{
  m_isSpellCheckAvailable = value;
}

bool CSongCodeEditor::isSpellCheckActive() const
{
  return m_isSpellCheckActive;
}

void CSongCodeEditor::setSpellCheckActive(const bool value)
{
  m_isSpellCheckActive = value;
#ifdef ENABLE_SPELLCHECK
  // signals are blocked to prevent triggering documentWasModified
  // it avoids marking the song as modified when words are only highlighted
  blockSignals(true);
  highlighter()->setSpellCheckActive(value);
  blockSignals(false);
#endif //ENABLE_SPELLCHECK
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
