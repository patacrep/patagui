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

/**
 * \file song.hh
 *
 * Provides the internal representation of a song from a songbook
 * library.
 */
#ifndef __SONG_HH__
#define __SONG_HH__

#include <QString>
#include <QStringList>
#include <QLocale>
#include <QRegExp>

/** \struct Song "song.hh"
 *
 * \brief Song is the internal representation of a song
 */
struct Song
{
  QString title;
  QString artist;
  QString album;
  QString coverName;
  QString coverPath;
  QString path;
  QLocale locale;
  bool isLilypond;
  int columnCount;
  int capo;
  int transpose;
  QStringList gtabs;
  QStringList utabs;
  QStringList lyrics;

  static Song fromFile(const QString &path);
  static Song fromString(const QString &text, const QString &path = QString());
  static QString toString(const Song &song);
  static QLocale::Language languageFromString(const QString &languageName = QString());
  static QString languageToString(const QLocale::Language language);

  static QRegExp reSgFile;
  static QRegExp reSong;
  static QRegExp reArtist;
  static QRegExp reAlbum;
  static QRegExp reCoverName;
  static QRegExp reLilypond;
  static QRegExp reLanguage;
  static QRegExp reColumnCount;
  static QRegExp reCapo;
  static QRegExp reTranspose;
  static QRegExp reCover;
  static QRegExp reBlankLine;
  static QRegExp reGtab;
  static QRegExp reUtab;

  static QRegExp reBegin;
  static QRegExp reEnd;
  static QRegExp reBeginVerse;
  static QRegExp reEndVerse;
  static QRegExp reBeginChorus;
  static QRegExp reEndChorus;
  static QRegExp reBeginScripture;
  static QRegExp reEndScripture;
};

#endif // __SONG_HH__
