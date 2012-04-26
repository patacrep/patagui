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

class CSongEditor;
class CDiagram;
class CCoverDropArea;
class LineEdit;

class QSpinBox;
class QComboBox;
class QBoxLayout;
class QToolButton;
class QSpacerItem;

/**
 * \file song-header-editor.hh
 * \class CSongHeaderEditor
 * \brief CSongHeaderEditor is a widget that manages a Song metadata in a CSongEditor
 *
 * A Song is composed of a header (metadata) and a body (lyrics).
 * The CSongHeaderEditor hides the plain text of the song's header to present them
 * in a more user-friendly way through appropriated QWidgets that allow to
 * fill the fields such as artist name, album, cover, capo etc.
 *
 */
class CSongHeaderEditor : public QWidget
{
  Q_OBJECT

public:
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

  LineEdit* titleLineEdit() const;
  LineEdit* artistLineEdit() const;

private:
  void addNewDiagramButton();

private slots:
  void onIndexChanged(const QString &text);
  void onTextEdited(const QString &text);
  void onValueChanged(int value);
  void addDiagram();
  void removeDiagram();

public slots:
  void update();

signals:
  void contentsChanged();

private:
  LineEdit *m_titleLineEdit;
  LineEdit *m_artistLineEdit;
  LineEdit *m_albumLineEdit;
  QComboBox *m_languageComboBox;
  QSpinBox *m_columnCountSpinBox;
  QSpinBox *m_capoSpinBox;
  QSpinBox *m_transposeSpinBox;
  CCoverDropArea *m_coverLabel;

  CSongEditor *m_songEditor;

  QBoxLayout *m_diagramsLayout;
  QToolButton *m_addDiagramButton;
  QSpacerItem *m_spacer;
};


class QMimeData;
class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDragDropEvent;

class CCoverDropArea : public QLabel
{
  Q_OBJECT

  public:
  CCoverDropArea(QWidget *parent = 0);

  Song & song();
  void setSong(const Song & song);

  /// Getter on the song's cover
  /// @return the cover
  const QImage & cover();

  /// Setter on the song's cover
  /// @para cover the cover as an image object
  void setCover(const QImage &cover);

  /// Setter on the song's cover
  /// @para cover the cover as a file object
  void setCover(const QString &path);

private slots:
  void selectCover();

public slots:
  void clear();
  void update();

signals:
  void changed(const QMimeData *mimeData = 0);

protected:
  void dragEnterEvent(QDragEnterEvent *event);
  void dragMoveEvent(QDragMoveEvent *event);
  void dragLeaveEvent(QDragLeaveEvent *event);
  void dropEvent(QDropEvent *event);
  virtual void mousePressEvent(QMouseEvent *event);
  virtual void mouseReleaseEvent(QMouseEvent *event);

private:
  Song m_song;
  QString m_filename;
  QImage m_cover;
};

#endif // __SONG_HEADER_EDITOR_HH__
