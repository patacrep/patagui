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
#include "chord.hh"
#include "diagram-editor.hh"

#include <QRect>
#include <QPainter>
#include <QDebug>

QRegExp CChord::reChordWithFret("\\\\[ug]tab[\\*]?\\{([^\\}]+)\\}\\{(\\d):([^\\}]+)");
QRegExp CChord::reChordWithoutFret("\\\\[ug]tab[\\*]?\\{([^\\}]+)\\}\\{([^\\}]+)");

CChord::CChord(const QString & chord, uint id,  QObject *parent)
  : QObject(parent)
  , m_isValid(true)
  , m_id(id)
{
  fromString(chord);
}

CChord::~CChord()
{}

QString CChord::toString()
{
  QString str;
  switch(m_instrument)
    {
    case Guitar:
      str.append("\\gtab");
      break;
    case Ukulele:
      str.append("\\utab");
      break;
    default:
      qWarning() << tr("CChord::toString unsupported chord type");
    }

  if (isImportant())
    str.append("*");

  //the chord name such as Am
  str.append( QString("{%1}{").arg(name()) );
  //the fret
  str.append(QString("%2").arg(fret()));
  //the strings such as X32010 (C chord)
  if (!fret().isEmpty())
    str.append(":");
  str.append(QString("%3}").arg(strings()));

  return str;
}

void CChord::fromString(const QString & str)
{
  QString copy(str);

  if (str.contains("gtab"))
    m_instrument = Guitar;
  else if (str.contains("utab"))
    m_instrument = Ukulele;
  else
    m_isValid = false;

  setImportant(str.contains("*"));

  if (reChordWithFret.indexIn(str) != -1)
    {
      setName(reChordWithFret.cap(1));
      setFret(reChordWithFret.cap(2));
      setStrings(reChordWithFret.cap(3));
    }
  else if (reChordWithoutFret.indexIn(copy.remove("~:")) != -1)
    {
      setName(reChordWithoutFret.cap(1));
      setStrings(reChordWithoutFret.cap(2));
    }

  if (name().isEmpty())
    m_isValid = false;
}


bool CChord::isValid() const
{
  return m_isValid;
}

QString CChord::name() const
{
  return m_name;
}

void CChord::setName(const QString & str)
{
  if (m_name != str)
    {
      m_name = str;
      emit nameChanged();
    }
}

uint CChord::id() const
{
  return m_id;
}

void CChord::setId(uint value)
{
  if (m_id != value)
    {
      m_id = value;
      emit idChanged();
    }
}

QString CChord::fret() const
{
  return m_fret;
}

void CChord::setFret(const QString & str)
{
  if (m_fret != str)
    {
      m_fret = str;
      emit fretChanged();
    }
}

QString CChord::strings() const
{
  return m_strings;
}

void CChord::setStrings(const QString & str)
{
  if (m_strings != str)
    {
      m_strings = str;
      emit stringsChanged();
    }
}

CChord::Instrument CChord::instrument() const
{
  return m_instrument;
}

void CChord::setInstrument(const CChord::Instrument & instru)
{
  m_instrument = instru;
}

bool CChord::isImportant() const
{
  return m_important;
}

void CChord::setImportant(bool value)
{
  m_important = value;
}

QColor CChord::color()
{
  QColor color;

  if (isImportant())
    {
      if (instrument() == Guitar)
	color = QColor(32, 74, 135);
      else if (instrument() == Ukulele)
	color = QColor(92, 53, 102);
    }
  else
    {
      if (instrument() == Guitar)
	color = QColor(114, 159, 207);
      else if (instrument() == Ukulele)
	color = QColor(173, 127, 168);
    }

  return color;
}
