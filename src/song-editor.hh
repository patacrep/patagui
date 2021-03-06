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
class Library;
class SongCodeEditor;
class CSongHeaderEditor;
class SongHighlighter;
class FindReplaceDialog;
class Hunspell;

/*!
  \file song-editor.hh
  \class Editor
  \brief Editor is an abstract song editor.

  A Editor defines an editor that contains:
   \li a list of actions in a toolbar
   \li a highlighter to apply syntaxic coloration onto its contents
*/
class Editor : public QWidget
{
    Q_OBJECT

public:
    /// Constructor
    Editor(QWidget *parent = 0);

    /// Destructor
    virtual ~Editor();

    /*!
    Returns the toolbar of the editor.
  */
    virtual QToolBar *toolBar() const;

    /*!
    Returns a group that contains every action
    in the toolbar.
  */
    virtual QActionGroup *actionGroup() const;

    /*!
    Associates this editor with an \a highlighter
    that applies syntaxic coloration onto its contents
  */
    virtual void setHighlighter(SongHighlighter *highlighter);

protected:
    /// Save contents
    QAction *m_saveAct;

    /// Cut current selection to clipboard
    QAction *m_cutAct;

    /// Copy current selection to clipboard
    QAction *m_copyAct;

    /// Paste selection from clipboard
    QAction *m_pasteAct;

    /// Undo previous action
    QAction *m_undoAct;

    /// Redo previous undone action
    QAction *m_redoAct;

    /// Replace a text string with another
    QAction *m_replaceAct;

    /// Search a text string
    QAction *m_searchAct;

    /// Insert new verse environment
    QAction *m_verseAct;

    /// Insert new chorus environment
    QAction *m_chorusAct;

    /// Insert new bridge environment
    QAction *m_bridgeAct;

    /// Underline mispelled words
    QAction *m_spellCheckingAct;

protected:
    QActionGroup *m_actions;
    QToolBar *m_toolBar;
};

/*!
  \file song-editor.hh
  \class SongEditor
  \brief SongEditor is the widget that allows to write a song

  A SongEditor is embedded into a TabWidget and is composed of:
   \li a CSongHeaderEditor that manages the song metadata
   \li a SongCodeEditor that manages the body of the song

  \image html song-editor.png

*/
class SongEditor : public Editor
{
    Q_OBJECT
    Q_PROPERTY(bool newSong READ isNewSong WRITE setNewSong)
    Q_PROPERTY(bool newCover READ isNewCover WRITE setNewCover)

public:
    SongEditor(QWidget *parent = 0);
    ~SongEditor();

    Library *library() const;

    void readSettings();
    void writeSettings();

    virtual void setHighlighter(SongHighlighter *highlighter);

    Song &song();
    void setSong(const Song &song);

    SongCodeEditor *codeEditor() const;

    bool isModified() const;
    bool isNewSong() const;

    //! Getter on the new cover property
    bool isNewCover() const;

    virtual QToolBar *toolBar() const;
    virtual QActionGroup *actionGroup() const;

    virtual bool isSpellCheckAvailable() const;
    virtual void setSpellCheckAvailable(const bool);

public slots:
    void setModified(bool modified);
    void setNewSong(bool newSong);

    //! Setter on the new cover property
    void setNewCover(bool newCover);

#ifdef ENABLE_SPELLCHECK
    void setDictionary(const QLocale &locale);
#endif // ENABLE_SPELLCHECK

    void toggleSpellCheckActive(bool);

signals:
    void labelChanged(const QString &label);
    void saved(const QString &path);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    // write modifications of the textEdit into sg file.
    void save();
    void documentWasModified();
    void findReplaceDialog();

private:
    void parseText();
    bool checkSongMandatoryFields();
    void saveNewSong();

    CSongHeaderEditor *m_songHeaderEditor;
    SongCodeEditor *m_codeEditor;
    FindReplaceDialog *m_findReplaceDialog;

    Song m_song;
    bool m_newSong;
    bool m_newCover;
};

#endif // __SONG_EDITOR_HH__
