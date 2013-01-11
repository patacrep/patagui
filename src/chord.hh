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
#ifndef __CHORD_HH__
#define __CHORD_HH__

#include <QObject>
#include <QString>
#include <QPixmap>
#include <QSize>
#include <QBrush>
#include <QRegExp>


class QPainter;

/*!
  \file chord.hh
  \class CChord
  \brief CChord is an object representing a guitar/ukulele chord

  A chord has a text and a graphical representation.

  The text representation follows the syntax introduced by the Songs
  LaTeX Package (http://songs.sourceforge.net):
  \code
  \gtab{<name>}{<fret>:<strings>}
  \endcode

  Regarding its graphical representation, a chord is a grid (diagram)
  where vertical lines correspond to strings and horizontal lines to
  frets:

  \li filled circles indicate frets to be pinched
  \li empty circles indicate a string that should be played open
  \li X indicate strings that should not be played

  \image html chord.png

*/
class CChord : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
  Q_PROPERTY(QString fret READ fret WRITE setFret NOTIFY fretChanged)
  Q_PROPERTY(QString strings READ strings WRITE setStrings NOTIFY stringsChanged)
  Q_PROPERTY(uint id READ id WRITE setId NOTIFY idChanged)

    public:
  /*!
    \enum StringCount
    This enum type indicates the number of strings of an instrumuent.
  */
    enum StringCount {
    GuitarStringCount=6, /*!< guitar: 6 */
    UkuleleStringCount=4 /*!< ukulele: 4 */
  };

  /*!
    \enum Instrument
    This enum type lists supported instruments.
  */
  enum Instrument {
    Guitar, /*!< guitar. */
    Ukulele /*!< ukulele. */
  };

  /// Constructor.
  CChord(const QString & chord = "\\gtab{}{0:}", uint id=0, QObject *parent = 0);

  /// Destructor.
  ~CChord();

  /*!
    Returns true if the chord is valid; false otherwise. A valid
    chord has a non-empty chord name and a number of strings that
    matches its instrument.
  */
  bool isValid() const;

  /*!
    Returns the string representation of the chord.
    \sa fromString
  */
  QString toString();

  /*!
    Builds a chord from a string.
    \sa toString
  */
  void fromString(const QString & gtab);

  /*!
    Returns the graphical representation (diagram) of the chord.
    \sa toString
  */
  QPixmap* toPixmap();

  /*!
    Returns the chord name.
    For example, given a E-flat minor chord
    \code \\gtab{E&m}{5:X02210} \endcode returns E&m.
    \sa setChord
  */
  QString name() const;

  /*!
    Returns the chord id.
    \sa setId
  */
  uint id() const;

  /*!
    Returns the fret number.
    For example, given a E-flat minor chord
    \code \gtab{E&m}{5:X02210} \endcode returns 5.
    \sa setFret
  */
  QString fret() const;

  /*!
    Returns the chord strings.
    For example, given a E-flat minor chord
    \code \gtab{E&m}{5:X02210} \endcode returns X02210.
    \sa setStrings
  */
  QString strings() const;

  /*!
    Returns the instrument of the chord.
    For example, given a E-flat minor chord
    \code \gtab{E&m}{5:X02210} \endcode returns Instrument::Guitar.
    \sa setType
  */
  Instrument instrument() const;

  /*!
    Returns true if the chord is important; false otherwise. Non
    important chords are well-known fingering for usual chords such as
    \code \gtab{C}{X32010} \endcode
    \sa setImportant
  */
  bool isImportant() const;

  /*!
    Returns the chord color.
    The color is based on the instrument (guitar: blue; ukulele: purple)
    and whether or not it is important (yes: dark; no: light).
    \sa setType, setImportant
  */
  QColor color();

  /*!
    Draws a rounded path around the whole diagram if \a value is true.
    Default is false. The rounded path takes the color of the chord.
    \sa color
  */
  void setDrawBorder(bool value);

public slots:
  /*!
    Sets the chord name \a name.
    \sa chord
  */
  void setName(const QString & name);

  /*!
    Sets the strings \a strings.
    \sa strings
  */
  void setStrings(const QString & strings);

  /*!
    Sets the fret number \a fret.
    \sa fret
  */
  void setFret(const QString & fret);

  /*!
    Sets the chord id \a value.
    \sa setId
  */
  void setId(uint value);

  /*!
    Sets the instrument \a instrument.
    \sa type
  */
  void setInstrument(const Instrument & instrument);

  /*!
    Toggle instrument between Guitar and Ukulele if \a value is true.
    \sa instrumet, setInstrument
  */
  void switchInstrument(bool value);

  /*!
    Marks a chord as important; default is false.
    \sa isImportant
  */
  void setImportant(bool value);

signals:
  void nameChanged();
  void fretChanged();
  void stringsChanged();
  void instrumentChanged();
  void idChanged();

private:
  void fillEllipse(QPainter* painter, const QRect & rect, const QBrush & brush);

  Instrument m_instrument;
  QString m_name;
  QString m_fret;
  QString m_strings;
  bool m_important;
  bool m_isValid;
  bool m_drawBorder;
  QPixmap *m_pixmap;
  uint m_id;

  static QRegExp reChordWithFret;
  static QRegExp reChordWithoutFret;
};

#endif // __CHORD_HH__
