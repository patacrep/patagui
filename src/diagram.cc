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
#include "diagram.hh"

#include <QPainter>
#include <QDebug>

QRegExp CDiagram::reChord("([^\\}]+)");
QRegExp CDiagram::reFret("\\{(\\d):");
QRegExp CDiagram::reStringsFret(":([^\\}]+)");
QRegExp CDiagram::reStringsNoFret("\\{([^\\}]+)");

CDiagram::CDiagram(QWidget *parent)
  : QWidget(parent)
{
  setBackgroundRole(QPalette::Base);
  setAutoFillBackground(true);
  show();
}

CDiagram::CDiagram(const QString & gtab, QWidget *parent)
  : QWidget(parent)
{
  fromString(gtab);
  setBackgroundRole(QPalette::Base);
  setAutoFillBackground(true);
}

CDiagram::~CDiagram()
{
}

QSize CDiagram::minimumSizeHint() const
{
  return QSize(100, 50);
}

QSize CDiagram::sizeHint() const
{
  return QSize(100, 50);
}

QString CDiagram::toString()
{
  return QString("\\gtab{%1}{%2:%3}").arg(chord()).arg(fret()).arg(strings());
}

void CDiagram::fromString(const QString & gtab)
{
  reChord.indexIn(gtab);
  setChord(reChord.cap(1));

  reFret.indexIn(gtab);
  setFret(reFret.cap(1));

  if(fret().isEmpty())
    {
      reStringsNoFret.indexIn(gtab);
      setStrings(reStringsNoFret.cap(1));
    }
  else
    {
      reStringsFret.indexIn(gtab);
      setStrings(reStringsFret.cap(1));
    }
}

void CDiagram::paintEvent(QPaintEvent *)
{
  QPainter painter(this);
  painter.setPen(Qt::black);

  int cellWidth = 14, cellHeight = 14;
  int width = (strings().length() - 1)*cellWidth;
  int height = 4*cellHeight;
  int padding = 15;
  //draw a vertical line for each string
  for(int i=0; i<strings().length(); ++i)
    {
      painter.drawLine(i*cellWidth+padding, padding, i*cellWidth+padding, height+padding);
    }

  //draw 5 horizontal lines
  for(int i=0; i<5; ++i)
    {
      painter.drawLine(padding, i*cellHeight+padding, width+padding, i*cellHeight+padding);
    }

  //draw played strings
  for(int i=0; i<strings().length(); ++i)
    {
      QRect stringRect(0, 0, cellWidth-4, cellHeight-4);
      int value = strings()[i].digitValue();
      if(value == -1)
	{
	  stringRect.moveTo( (i*cellWidth)+cellWidth/2.0 +3, 3 );
	  painter.setFont(QFont("Arial", 9));
	  painter.drawText(stringRect, Qt::AlignCenter, "X");
	}
      else
	{
	  stringRect.moveTo( (i*cellWidth)+cellWidth/2.0 +3, value*cellHeight+3 );
	  if(value == 0)
	    painter.drawEllipse(stringRect);
	  else
	    fillEllipse(&painter, stringRect, QBrush(QColor(Qt::black)));
	}
    }

  //draw fret
  QRect fretRect(padding-(cellWidth-4), padding+cellHeight/2.0, cellWidth-4, cellHeight);
  painter.setFont(QFont("Arial", 11));
  painter.drawText(fretRect, Qt::AlignCenter, fret());

  //draw chord name
  QRect chordRect(padding, height+padding+10, width, 20);
  painter.setFont(QFont("Arial", 12));
  painter.drawText(chordRect, Qt::AlignCenter, chord());
}

void CDiagram::fillEllipse(QPainter* painter, const QRect & rect, const QBrush & brush)
{
  QPainterPath path;
  path.addEllipse(rect.topLeft().x(), rect.topLeft().y(), rect.width(), rect.height());;
  painter->fillPath(path, brush);
}

QString CDiagram::chord() const
{
  return m_chord;
}

void CDiagram::setChord(const QString & str)
{
  m_chord = str;
}

QString CDiagram::fret() const
{
  return m_fret;
}

void CDiagram::setFret(const QString & str)
{
  m_fret = str;
}

QString CDiagram::strings() const
{
  return m_strings;
}

void CDiagram::setStrings(const QString & str)
{
  m_strings = str;
}
