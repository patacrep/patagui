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
#ifndef __SONG_EDITOR_HH__
#define __SONG_EDITOR_HH__

#include "code-editor.hh"

#include <QToolBar>
#include <QList>

class QAction;
class Hunspell;

class CSongEditor : public CodeEditor
{
  Q_OBJECT

public:
  CSongEditor();
  ~CSongEditor();

  QString path();
  void setPath(const QString & APath);

  QToolBar* toolBar();

  virtual void keyPressEvent(QKeyEvent *event);

  void addAction(QAction*);
  QList<QAction*> actions() const;

  void readSettings();
  void writeSettings();
  QStringList getWordPropositions(const QString &word);

signals:
  void labelChanged(const QString &label);
  void addWord(const QString &word);

private slots:
  //write modifications of the textEdit into sg file.
  void save();
  void documentWasModified();
  void insertVerse();
  void insertChorus();

  void correctWord();
  void slot_addWord();
  void slot_ignoreWord();

protected:
  void contextMenuEvent(QContextMenuEvent *event);

private:
  QString syntaxicColoration(const QString &);
  void indentSelection();
  void indentLine(const QTextCursor & cursor);
  void trimLine(const QTextCursor & cursor);
  QString currentWord();

  QToolBar* m_toolBar;
  QString m_path;
  QList<QAction*> m_actions;

  //Spell-checking
  enum { MaxWords = 5 };
  QAction *m_misspelledWordsActs[MaxWords];
  Hunspell *m_checker;
  QPoint m_lastPos;
  QStringList m_addedWords;
};

#endif // __SONG_EDITOR_HH__
