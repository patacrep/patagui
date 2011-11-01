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
#include "song.hh"

#include "utils/utils.hh"

#include <QFile>
#include <QFileInfo>

#include <QDebug>

QRegExp Song::reSong("begin\\{?song\\}?\\{([^[\\}]+)\\}[^[]*\\[([^]]*)\\]");
QRegExp Song::reArtist("by=([^,]+)");
QRegExp Song::reAlbum("album=([^,]+)");
QRegExp Song::reCoverName("cov=([^,]+)");
QRegExp Song::reLilypond("\\\\lilypond");
QRegExp Song::reLanguage("selectlanguage\\{([^\\}]+)");
QRegExp Song::reColumnCount("songcolumns\\{([^\\}]+)");
QRegExp Song::reCapo("capo\\{([^\\}]+)");

Song Song::fromString(const QString &text, const QString &path)
{
  Song song;
  song.path = path;

  reSong.indexIn(text);
  song.title = SbUtils::latexToUtf8(reSong.cap(1));

  reArtist.indexIn(reSong.cap(2));
  song.artist = SbUtils::latexToUtf8(reArtist.cap(1));

  reAlbum.indexIn(reSong.cap(2));
  song.album = SbUtils::latexToUtf8(reAlbum.cap(1));

  song.isLilypond = QBool(reLilypond.indexIn(text) > -1);

  reCoverName.indexIn(reSong.cap(2));
  song.coverName = reCoverName.cap(1);

  song.coverPath = QFileInfo(path).absolutePath();

  reLanguage.indexIn(text);
  song.language = languageFromString(reLanguage.cap(1));

  reColumnCount.indexIn(text);
  song.columnCount = reColumnCount.cap(1).toInt();

  reCapo.indexIn(text);
  song.capo = reCapo.cap(1).toInt();

  return song;
}

Song Song::fromFile(const QString &path)
{
  QFile file(path);

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      qWarning() << "Song::fromFile: unable to open " << path;
      return Song();
    }

  QTextStream stream (&file);
  stream.setCodec("UTF-8");
  QString fileStr = stream.readAll();
  file.close();

  return Song::fromString(fileStr, path);
}

QLocale::Language Song::languageFromString(const QString &languageName)
{
  if (languageName == "french")
    return QLocale::French;
  else if (languageName == "english")
    return QLocale::English;
  else if (languageName == "spanish")
    return QLocale::Spanish;

  return QLocale::system().language();
}

QString Song::languageToString(const QLocale::Language language)
{
  switch (language)
    {
    case QLocale::French:
      return "french";
    case QLocale::English:
      return "english";
    case QLocale::Spanish:
      return "spanish";
    defaut:
      return "english";
    }
}
