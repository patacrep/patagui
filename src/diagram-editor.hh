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
#ifndef __DIAGRAM_EDITOR_HH__
#define __DIAGRAM_EDITOR_HH__

#include <QDialog>
#include <QString>
#include "chord.hh"

class QLineEdit;
class QSpinBox;
class QRadioButton;
class QCheckBox;
class QLabel;

class Chord;
class DiagramArea;

/*!
  \file diagram-editor.hh
  \class DiagramEditor
  \brief DiagramEditor is a dialog for editing or creating chords

  The DiagramEditor class provides a dialog widget for editing or
  creating chords. It is composed of a form where fields are the
  properties of a Chord object and of a list of common chords (a
  DiagramArea object) that may be selected.

  \image html chord-editor.png

  \sa Chord, DiagramArea
*/
class DiagramEditor : public QDialog
{
    Q_OBJECT

    public:
    /// Constructor.
    DiagramEditor(QWidget *parent = 0);

    /// Destructor.
    ~DiagramEditor();

    /*!
    Returns the preferred size for the dialog.
  */
    virtual QSize sizeHint() const;

    /*!
    Returns the Chord object associated with the dialog.
  */
    Chord *chord() const;

    public slots:
    /*!
    Associates the CDiagram object \a diagram with dialog.
    The properties of \a diagram are used to fill the form of the dialog.
  */
    void setChord(Chord *chord);

    private slots:
    bool checkChord();
    void onInstrumentChanged(bool);
    void reset();

    private:
    // chord
    QRadioButton *m_guitar;
    QRadioButton *m_ukulele;
    QLineEdit *m_nameLineEdit;
    QLineEdit *m_stringsLineEdit;
    QSpinBox *m_fretSpinBox;
    QCheckBox *m_importantCheckBox;
    // info
    QLabel *m_infoIconLabel;
    QLabel *m_messageLabel;

    DiagramArea *m_diagramArea;
    Chord *m_chord;
};

#endif // __DIAGRAM_EDITOR_HH__
