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

#include <QWidget>
#include <QString>
#include <QToolBar>
#include "mainwindow.hh"

class CodeEditor;

class CSongEditor : public QWidget
{
  Q_OBJECT

public:
  CSongEditor(const QString & APath,CMainWindow* mw);
  virtual ~CSongEditor();

  QString filePath();
  void setFilePath(const QString & APath);

  int tabIndex();
  void setTabIndex(int AIndex);

  QString label();
  void setLabel(const QString & ALabel);

  QToolBar* getToolbar();

private:
  QString syntaxicColoration(const QString &);
  QToolBar* toolbar;

private slots:
  //write modifications of the textEdit into sg file.
  void save();
  void documentWasModified();
  void insertVerse();
  void insertChorus();

signals:
  void labelChanged();

public:
  CodeEditor* m_textEdit;
  bool isOk;

private:
  QString m_filePath;
  QString m_label; //tab title
  int m_tabIndex;  //tab index
};

#endif // __SONG_EDITOR_HH__
