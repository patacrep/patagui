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

#include "config.hh"
#include "song.hh"

#include <QWidget>
#include <QString>

class QAction;
class QActionGroup;
class QToolBar;
class CFindReplaceDialog;
class CLibrary;
class CSongCodeEditor;
class CSongHeaderEditor;
class CSongHighlighter;

class CEditor : public QWidget
{
  Q_OBJECT

public:
  CEditor(QWidget *parent = 0);
  virtual ~CEditor();

  virtual QToolBar * toolBar() const;
  virtual QActionGroup * actionGroup() const;

  virtual bool isSpellCheckAvailable() const;
  virtual void setSpellCheckAvailable(const bool);
  virtual void setHighlighter(CSongHighlighter *highlighter);

protected:
  QAction *m_saveAct;
  QAction *m_cutAct;
  QAction *m_copyAct;
  QAction *m_pasteAct;
  QAction *m_undoAct;
  QAction *m_redoAct;
  QAction *m_replaceAct;
  QAction *m_searchAct;
  QAction *m_verseAct;
  QAction *m_chorusAct;
  QAction *m_bridgeAct;
  QAction* m_spellCheckingAct;
  QActionGroup *m_actions;
  QToolBar *m_toolBar;
};

/*!
  \file song-editor.hh
  \class CSongEditor
  \brief CSongEditor is the widget that allows to write a song

  A CSongEditor is embedded into a CTabWidget and is composed of:
   \li a CSongHeaderEditor that manages the song metadata
   \li a CSongCodeEditor that manages the body of the song

  \image html song-editor.png

*/
class CSongEditor : public CEditor
{
  Q_OBJECT
  Q_PROPERTY(bool newSong READ isNewSong WRITE setNewSong)
  Q_PROPERTY(bool newCover READ isNewCover WRITE setNewCover)

public:
  CSongEditor(QWidget *parent = 0);
  ~CSongEditor();

  CLibrary * library() const;

  void readSettings();
  void writeSettings();

  virtual void setHighlighter(CSongHighlighter *highlighter);

  Song & song();
  void setSong(const Song &song);

  CSongCodeEditor * codeEditor() const;

  bool isModified() const;
  bool isNewSong() const;

  //! Getter on the new cover property
  bool isNewCover() const;

  //! Setter on the new cover property
  void setNewCover(bool newCover);

  virtual QToolBar * toolBar() const;
  virtual QActionGroup * actionGroup() const;

  virtual bool isSpellCheckAvailable() const;
  virtual void setSpellCheckAvailable(const bool);

public slots:
  void setModified(bool modified);
  void setNewSong(bool newSong);
#ifdef ENABLE_SPELLCHECK
  void setDictionary(const QLocale & locale);
#endif //ENABLE_SPELLCHECK

signals:
  void labelChanged(const QString &label);
  void saved(const QString &path);

protected:
  void closeEvent(QCloseEvent *event);

private slots:
  //write modifications of the textEdit into sg file.
  void save();
  void documentWasModified();
  void findReplaceDialog();

private:
  void parseText();
  bool checkSongMandatoryFields();
  void saveNewSong();
  void createNewSong();

  CSongCodeEditor *m_codeEditor;
  CSongHeaderEditor *m_songHeaderEditor;
  CFindReplaceDialog* m_findReplaceDialog;

  Song m_song;
  bool m_newSong;
  bool m_newCover;
};


#endif // __SONG_EDITOR_HH__
