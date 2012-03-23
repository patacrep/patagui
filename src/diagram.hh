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
#ifndef __DIAGRAM_HH__
#define __DIAGRAM_HH__

#include "diagram.hh"

#include <QWidget>
#include <QString>

/**
 * \file diagram.hh
 * \class CDiagram
 * \brief CDiagram is a widget representing a guitar/ukulele chord diagram
 *
 */
class CDiagram : public QWidget
{
  Q_OBJECT

public:
  CDiagram(QWidget *parent = 0);
  CDiagram(const QString & chord, QWidget *parent = 0);
  ~CDiagram();

  QString toString();
  void fromString(const QString & gtab);

  QString chord() const;
  void setChord(const QString & name);

  QString fret() const;
  void setFret(const QString & fret);

  QString strings() const;
  void setStrings(const QString & chord);

  QSize minimumSizeHint() const;
  QSize sizeHint() const;

protected:
  void paintEvent(QPaintEvent * event);

private:
  void fillEllipse(QPainter* painter, const QRect & rect, const QBrush & brush);

  QString m_chord;
  QString m_fret;
  QString m_strings;

  static QRegExp reChord;
  static QRegExp reFret;
  static QRegExp reStringsFret;
  static QRegExp reStringsNoFret;
};

#endif // __DIAGRAM_HH__
