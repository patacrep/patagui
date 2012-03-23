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
#ifndef __SONG_CODE_EDITOR_HH__
#define __SONG_CODE_EDITOR_HH__

#include "config.hh"
#include "code-editor.hh"

#include <QTextCursor>

class QKeyEvent;
class CSongHighlighter;

/**
 * \file song-code-editor.hh
 * \class CSongCodeEditor
 * \brief CSongCodeEditor is the widget to edit a song's content
 *
 */
class CSongCodeEditor : public CodeEditor
{
  Q_OBJECT
  Q_ENUMS(SongEnvironment)

public:
  enum SongEnvironment { Verse, Chorus, Scripture, None };

  CSongCodeEditor(QWidget *parent = 0);
  ~CSongCodeEditor();

  void readSettings();
  void writeSettings();

  void installHighlighter();

  CSongHighlighter* highlighter() const;

protected:
  virtual void keyPressEvent(QKeyEvent *event);

private slots:
  void highlight();

private:
  void indentSelection();
  void indentLine(const QTextCursor &cursor);
  void trimLine(const QTextCursor &cursor);

  QTextEdit::ExtraSelection environmentSelection(const SongEnvironment & env,
						 const QTextCursor & cursor);

  CSongHighlighter* m_highlighter;
};

#endif // __SONG_CODE_EDITOR_HH__
