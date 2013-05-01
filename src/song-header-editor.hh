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

class CCoverDropArea;
class CDiagramArea;
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
  /*!
    \enum ViewMode
    This enum describes available view modes for CSongHeaderEditor objects.
  */
  enum ViewMode {
    FullViewMode, /*!< full view: displays cover, song options and chords (editable). */
    MiniViewMode  /*!< mini view: displays mini cover, artist and title (read-only). */
  };

  /// Constructor.
  CSongHeaderEditor(QWidget *parent = 0);
  /// Destructor.
  virtual ~CSongHeaderEditor();

  /// Getter on the song whose metadata is used
  /// to build this instance of CSongHeaderEditor.
  /// @return the Song
  Song & song();

  /// Setter on the song
  /// @param song the song
  void setSong(const Song &song);

  /// Getter on the song's cover
  /// @return the cover
  const QImage & cover();

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

private:
  /*!
    Updates the elements of the header from the song contents.
  */
  void update();

public slots:

  /*!
    Toggles between full-view mode and mini-view mode.
  */
  void toggleView();

signals:
  /*!
    This signal is emitted when any of the contents in the header is changed.
  */
  void contentsChanged();

  /*!
    This signal is emitted when the language of the song is changed.
  */
  void languageChanged(const QLocale &);

  void newCover(bool);

private:

  /*!
    Sets the artist and album completers from the library.
    Those completers are set on album and artist line edits.
  */
  void setLibraryCompleters();

  Song m_song;

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
  /// Constructor.
  CCoverDropArea(CSongHeaderEditor *parent);

  /*!
    Returns the Song object associated with this cover.
  */
  Song & song();

  /*!
    Returns the cover of the song.
    \sa setCover
  */
  const QImage & cover();

  /*!
    Sets \a cover as the cover of the song.
    \sa cover
  */
  void setCover(const QImage &cover);

  /*!
    Sets the file at (absolute) path \a filename as the cover of the song.
    A .jpg file is expected.
    \sa cover
  */
  void setCover(const QString &filename);

  /*!
    Returns the parent widget.
    \sa setParent
  */
  CSongHeaderEditor * parent() const;

  /*!
    Sets \a p as the parent object.
    \sa parent
  */
  void setParent(CSongHeaderEditor *p);

private slots:
  void selectCover();
  void clearCover();

public slots:
  /*!
    Resets the background of the cover area.
  */
  void clear();

  /*!
    Updates the cover-related fields of the song according to current image.
  */
  void update();

signals:
  /*!
    This signal is emitted whenever new data is dragged within the cover area.
    \sa coverChanged, miniCoverChanged
  */
  void changed(const QMimeData *mimeData = 0);

  /*!
    This signal is emitted whenever a new cover image is displayed.
    \sa coverChanged, miniCoverChanged
  */
  void coverChanged();

  /*!
    This signal is emitted whenever a new cover image is displayed.
    \sa coverChanged, miniCoverChanged
  */
  void miniCoverChanged(const QPixmap & thumbnail);

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
