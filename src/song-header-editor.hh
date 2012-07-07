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
#ifndef __SONG_HEADER_EDITOR_HH__
#define __SONG_HEADER_EDITOR_HH__

#include <QWidget>

#include "song.hh"

#include <QString>
#include <QLabel>
#include <QPixmap>

class CSongEditor;
class CCoverDropArea;
class CDiagramArea;
class CLibrary;
class LineEdit;

class QSpinBox;
class QComboBox;
class QBoxLayout;
class QStackedLayout;

/*!
  \file song-header-editor.hh
  \class CSongHeaderEditor
  \brief CSongHeaderEditor is a widget that manages a Song metadata in a CSongEditor

  A Song is composed of a header (metadata) and a body (lyrics).
  The CSongHeaderEditor hides the plain text of the song's header to present them
  in a more user-friendly way through appropriated QWidgets that allow to
  fill the fields such as artist name, album, cover, capo etc.

  Two different views can be toggled:
  \li the full-view mode allows to edit the song metadata
  \image html song-header-editor-full.png
  \li the mini-view mode saves vertical space and provides minimal read-only information
  \image html song-header-editor-mini.png

*/
class CSongHeaderEditor : public QWidget
{
  Q_OBJECT

public:
  enum ViewMode { FullViewMode, MiniViewMode };

  /// Constructor.
  CSongHeaderEditor(QWidget *parent = 0);
  /// Destructor.
  ~CSongHeaderEditor();

  /// Getter on the song from whose metadata is used
  /// to build this instance of CSongHeaderEditor.
  /// @return the Song
  Song & song();

  /// Getter on the embedding SongEditor
  /// @return the embedding song editor
  CSongEditor * songEditor() const;

  /// Setter on the embedding SongEditor
  /// @param songEditor the embedding song editor
  void setSongEditor(CSongEditor *songEditor);

  /// Getter on the song's cover
  /// @return the cover
  const QImage & cover();

  /*!
    Sets the library \a library. The library is used to provide
    completion models for title and artist line edits.
  */
  void setLibrary(CLibrary* library);

  /*!
    Returns the title line edit widget.
  */
  LineEdit* titleLineEdit() const;

  /*!
    Returns the artist line edit widget.
  */
  LineEdit* artistLineEdit() const;

  /*!
    Returns the preferred size of the widget.
  */
  QSize sizeHint() const;

private slots:
  void onIndexChanged(const QString &text);
  void onTextEdited(const QString &text);
  void onValueChanged(int value);
  void onDiagramsChanged();
  void onCoverChanged();

public slots:
  /*!
    Updates the elements of the header from the song contents.
  */
  void update();

  /*!
    Toggles between full-view mode and mini-view mode.
  */
  void toggleView();

signals:
  void contentsChanged();
  void languageChanged(const QLocale &);

private:
  CSongEditor *m_songEditor;

  LineEdit *m_titleLineEdit;
  LineEdit *m_artistLineEdit;
  LineEdit *m_albumLineEdit;
  LineEdit *m_originalSongLineEdit;
  LineEdit *m_urlLineEdit;
  QComboBox *m_languageComboBox;
  QSpinBox *m_columnCountSpinBox;
  QSpinBox *m_capoSpinBox;
  QSpinBox *m_transposeSpinBox;
  CCoverDropArea *m_coverLabel;

  CDiagramArea *m_diagramArea;

  ViewMode m_viewMode;
  QStackedLayout *m_stackedLayout;
};


class QMimeData;
class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDragDropEvent;

/*!
  \file song-header-editor.hh
  \class CCoverDropArea
  \brief CCoverDropArea displays the cover of a song.

  The cover area displays the cover of a song within a raised box. If
  there is no cover to display, it displays a generic image-missing
  icon. Clicking on the area prompts a file picker dialog where an
  external \a .jpg cover can be selected. The cover area also accepts
  drops from local \a .jpg files.

  \image html cover-drop-area.png
  \image html cover-drop-area2.png

*/
class CCoverDropArea : public QLabel
{
  Q_OBJECT

  public:
  CCoverDropArea(CSongHeaderEditor *parent);

  Song & song();

  /// Getter on the song's cover
  /// @return the cover
  const QImage & cover();

  /// Setter on the song's cover
  /// @param cover the cover as an image object
  void setCover(const QImage &cover);

  /// Setter on the song's cover
  /// @param cover the cover as a file object
  void setCover(const QString &cover);

  CSongHeaderEditor * parent() const;
  void setParent(CSongHeaderEditor *p);

private slots:
  void selectCover();
  void clearCover();

public slots:
  void clear();
  void update();

signals:
  void changed(const QMimeData *mimeData = 0);
  void coverChanged();
  void miniCoverChanged(const QPixmap &);

protected:
  void dragEnterEvent(QDragEnterEvent *event);
  void dragMoveEvent(QDragMoveEvent *event);
  void dragLeaveEvent(QDragLeaveEvent *event);
  void dropEvent(QDropEvent *event);
  virtual void mousePressEvent(QMouseEvent *event);
  virtual void mouseReleaseEvent(QMouseEvent *event);
  virtual void contextMenuEvent(QContextMenuEvent *event);

private:
  QString m_filename;
  QImage m_cover;
  CSongHeaderEditor * m_parent;
};

#endif // __SONG_HEADER_EDITOR_HH__
