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
#ifndef __SONG_HH__
#define __SONG_HH__

#include <QString>
#include <QStringList>
#include <QLocale>

/*!
  \file song.hh
  \struct Song "song.hh"
  \brief Song is the internal representation of a song

  Provides the internal representation of a song from a songbook
  library.
*/
struct Song
{
  QString title; /*!< the title of the song.*/
  QString artist; /*!< the artist of the song.*/
  QString album; /*!< the album of the song.*/
  QString originalSong; /*!< the original song.*/
  QString url; /*!< the website of the artist.*/
  QString coverName; /*!< the cover file name (relative, without extension).*/
  QString coverPath; /*!< the cover file name (absolute).*/
  QString path; /*!< the path of the .sg file (absolute).*/
  QLocale locale; /*!< the locale of the song.*/
  bool isLilypond; /*!< \a true if the song contains lilypond sheets.*/
  bool isWebsite; /*!< \a true if the song contains a link to the website of the artist.*/
  int columnCount; /*!< the number of columns of the song in the PDF.*/
  int capo; /*!< the capo of the song.*/
  int transpose; /*!< the transposition information.*/
  QStringList gtabs; /*!< the list of guitar chords.*/
  QStringList utabs; /*!< the list of ukulele chords.*/
  QStringList lyrics; /*!< the song contents.*/
  QStringList scripture; /*!< the song scriptures (comments/notes at the end in the PDF).*/

  /*!
    Constructs a Song object from a file whose absolute path is \a path.
    \sa fromString, toString
  */
  static Song fromFile(const QString &path);

  /*!
    Constructs a Song object whose content is \a text.
    \sa fromString, toString
  */
  static Song fromString(const QString &text, const QString &path = QString());

  /*!
    Returns the contents of the song \a song.
    \sa fromString, fromFile
  */
  static QString toString(const Song &song);

  /*!
    Converts a language string \a languageName to a QLocale object.
    Language strings are usually strings used by babel (LaTeX module).
    For example: english, french etc.
    \sa languageToString
  */
  static QLocale::Language languageFromString(const QString &languageName = QString());

  /*!
    Converts a QLocale object \a language as a plain string (ie english, french)
    that can be used by babel (LaTeX module).
    \sa languageFromString
  */
  static QString languageToString(const QLocale::Language language);

  /*!
    Converts LaTeX special sequences to utf8 characters.
    For example: \'e -> é.
    \sa utf8ToLatex
  */
  static QString latexToUtf8(const QString & str);

  /*!
    Converts some utf8 characters to LaTeX sequences.
    For example: & -> \&.
    \sa latexToUtf8
  */
  static QString utf8ToLatex(const QString & str);

  /*!
    Extracts the contents of a .sg file.
    This regular expression captures 5 fields:
    \li everything before \\beginsong (prefix)
    \li the song title
    \li the song options (such as artist name, album, cover etc) (options)
    \li the song contents
    \li everything after \\endsong (post)
   */
  static QRegExp reSgFile;

  /*!
    Extracts the artist of the song from a .sg file.
  */
  static QRegExp reArtist;

  /*!
    Extracts the album of the song from a .sg file.
  */
  static QRegExp reAlbum;

  /*!
    Extracts the original song of the song from a .sg file.
  */
  static QRegExp reOriginalSong;

  /*!
    Extracts the artist website of the song from a .sg file.
  */
  static QRegExp reUrl;

  /*!
    Extracts the cover of the song from a .sg file.
  */
  static QRegExp reCoverName;

  /*!
    Looks for lilypond macros within a .sg file.
  */
  static QRegExp reLilypond;

  /*!
    Extracts the language of the song from a .sg file.
  */
  static QRegExp reLanguage;

  /*!
    Extracts the number of columns of the song from a .sg file.
  */
  static QRegExp reColumnCount;

  /*!
    Extracts the capo of the song from a .sg file.
  */
  static QRegExp reCapo;

  /*!
    Extracts the transposition information of the song from a .sg file.
  */
  static QRegExp reTranspose;

  /*!
    Extracts the cover filename associated with the song from a .sg file.
  */
  static QRegExp reCover;

  /*!
    Extracts gtab macros (guitar diagrams) that are used for CChord objects.
  */
  static QRegExp reGtab;

  /*!
    Extracts utab macros (ukulele diagrams) that are used for CChord objects.
  */
  static QRegExp reUtab;
};

#endif // __SONG_HH__
