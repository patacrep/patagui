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
#ifndef __SONG_EDITOR_HH__
#define __SONG_EDITOR_HH__

#include "code-editor.hh"

#include <QToolBar>

class CSongEditor : public CodeEditor
{
  Q_OBJECT

public:
  CSongEditor();
  ~CSongEditor();

  QString path();
  void setPath(const QString & APath);

  QToolBar* toolbar();

signals:
  void labelChanged(const QString &label);

private slots:
  //write modifications of the textEdit into sg file.
  void save();
  void documentWasModified();
  void insertVerse();
  void insertChorus();

private:
  QString syntaxicColoration(const QString &);

  QToolBar* m_toolbar;
  QString m_path;
};

#endif // __SONG_EDITOR_HH__
