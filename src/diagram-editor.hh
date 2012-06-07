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

class QLineEdit;
class QSpinBox;
class QRadioButton;
class QCheckBox;
class QLabel;

class CDiagram;

/**
 * \file diagram-editor.hh
 * \class CDiagramEditor
 * \brief CDiagramEditor is a dialog that edits chord diagrams
 *
 */
class CDiagramEditor : public QDialog
{
  Q_OBJECT

public:
  CDiagramEditor(QWidget *parent=0);
  ~CDiagramEditor();

  QString chordName() const;
  QString chordStrings() const;
  QString chordFret() const;
  bool isChordImportant() const;
  void setDiagram(CDiagram *diagram);

private slots:
  bool checkChord();

private:
  //chord
  QRadioButton *m_guitar;
  QRadioButton *m_ukulele;
  QLineEdit *m_nameLineEdit;
  QLineEdit *m_stringsLineEdit;
  QSpinBox *m_fretSpinBox;
  QCheckBox *m_importantCheckBox;
  //info
  QLabel *m_infoIconLabel;
  QLabel *m_messageLabel;
};

#endif // __DIAGRAM_EDITOR_HH__
