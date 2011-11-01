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
 * Structure containing all the fields representing a song.
 *
 */
#ifndef __SONG_HH__
#define __SONG_HH__

#include <QString>
#include <QLocale>
#include <QRegExp>

struct Song
{
  QString title;
  QString artist;
  QString album;
  QString coverName;
  QString coverPath;
  QString path;
  QLocale::Language language;
  bool isLilypond;
  int columnCount;
  int capo;

  static Song fromFile(const QString &path);
  static Song fromString(const QString &text, const QString &path = QString());
  static QLocale::Language languageFromString(const QString &languageName = QString());
  static QString languageToString(const QLocale::Language language);

  static QRegExp reSong;
  static QRegExp reArtist;
  static QRegExp reAlbum;
  static QRegExp reCoverName;
  static QRegExp reLilypond;
  static QRegExp reLanguage;
  static QRegExp reColumnCount;
  static QRegExp reCapo;
};

#endif // __SONG_HH__
