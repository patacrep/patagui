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

#include <QFile>
#include <QFileInfo>
#include <QRegExp>
#include <QColor>

#include <QDebug>

const QRegExp Song::reSgFile("(.*)\\\\begin\\{?song\\}?\\{([^\\}]+)\\}[^[]*\\[([^]]*)\\](.*)\\s*\\\\endsong(.*)");
const QRegExp Song::reArtist("by=\\{?([^,\\{\\}]+)");
const QRegExp Song::reAlbum("album=\\{?([^,\\{\\}]+)");
const QRegExp Song::reOriginalSong("original=\\{?([^,\\{\\}]+)");
const QRegExp Song::reUrl("url=\\{?([^,\\{\\}]+)");
const QRegExp Song::reCoverName("cov=\\{?([^,\\{\\}]+)");
const QRegExp Song::reLilypond("\\\\lilypond");
const QRegExp Song::reLanguage("\\\\selectlanguage\\{([^\\}]+)");
const QRegExp Song::reColumnCount("\\\\songcolumns\\{([^\\}]+)");
const QRegExp Song::reCapo("\\\\capo\\{([^\\}]+)");
const QRegExp Song::reTranspose("\\\\transpose\\{([^\\}]+)");
const QRegExp Song::reCover("\\\\cover");

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

Song Song::fromString(const QString &text, const QString &path)
{
  Song song;
  reSgFile.indexIn(text);

  QString prefix = reSgFile.cap(1);
  QString options = reSgFile.cap(3);
  QString content = reSgFile.cap(4);
  QString post = reSgFile.cap(5);

  // path
  song.path = path;

  // path (for cover)
  song.coverPath = QFileInfo(path).absolutePath();

  reColumnCount.indexIn(prefix);
  song.columnCount = reColumnCount.cap(1).toInt();

  // title
  song.title = latexToUtf8(reSgFile.cap(2));

  // options
  reArtist.indexIn(options);
  song.artist = latexToUtf8(reArtist.cap(1));

  reAlbum.indexIn(options);
  song.album = latexToUtf8(reAlbum.cap(1));

  reOriginalSong.indexIn(options);
  song.originalSong = latexToUtf8(reOriginalSong.cap(1));

  reUrl.indexIn(options);
  song.url = reUrl.cap(1).replace("http://","");
  if (song.url.endsWith("/"))
    song.url.chop(1);
  song.isWebsite = !song.url.isEmpty();

  reCoverName.indexIn(options);
  song.coverName = reCoverName.cap(1);

  // content
  song.isLilypond = QBool(reLilypond.indexIn(content) > -1);

  //locale
  reLanguage.indexIn(prefix);
  song.locale = QLocale(languageFromString(reLanguage.cap(1)), QLocale::AnyCountry);

  song.capo = 0;
  song.transpose = 0;

  QStringList lines = content.split("\n");
  QString line;
  bool preliminaryFinished = false;
  foreach (line, lines)
    {
      if (!preliminaryFinished)
        {
          if (reCapo.indexIn(line) != -1)
            {
              song.capo = reCapo.cap(1).toInt();
              continue;
            }
          else if (reTranspose.indexIn(line) != -1)
            {
              song.transpose = reTranspose.cap(1).toInt();
              continue;
            }
          else if (line.contains("\\gtab"))
            {
	      song.gtabs << line.trimmed();
              continue;
            }
          else if (line.contains("\\utab"))
            {
	      song.utabs << line.trimmed();
              continue;
            }
          else if (reCover.indexIn(line) != -1 || line.trimmed().isEmpty())
            {
              continue;
            }
          else if (!line.trimmed().startsWith("%"))
            {
              preliminaryFinished = true;
            }
        }
      if (preliminaryFinished)
        {
          song.lyrics << line;
        }
    }
  // remove blank line at the end of input
  if (!song.lyrics.isEmpty())
    while (song.lyrics.last().trimmed().isEmpty())
      {
	if (song.lyrics.isEmpty())
	  break;
	song.lyrics.removeLast();
      }

  lines = post.split("\n");
  foreach (line, lines)
    {
      song.scripture << line;
    }

  return song;
}

QString Song::toString(const Song &song)
{
  QString text;
  text.append(QString("\\selectlanguage{%1}\n").arg(Song::languageToString(song.locale.language())));
  if (song.columnCount > 0)
    text.append(QString("\\songcolumns{%1}\n").arg(song.columnCount));

  text.append(QString("\\beginsong{%1}\n  [by={%2}").arg(utf8ToLatex(song.title)).arg(utf8ToLatex(song.artist)));

  if (!song.coverName.isEmpty())
    text.append(QString(",cov={%1}").arg(song.coverName));

  if (!song.album.isEmpty())
    text.append(QString(",album={%1}").arg(utf8ToLatex(song.album)));

  if (!song.originalSong.isEmpty())
    text.append(QString(",%\n  original={%1}").arg(utf8ToLatex(song.originalSong)));

  if (!song.url.isEmpty())
    text.append(QString(",%\n  url={%1}").arg(song.url));

  text.append(QString("]\n\n"));

  if (!song.coverName.isEmpty())
    text.append(QString("  \\cover\n"));

  if (song.transpose != 0)
    text.append(QString("  \\transpose{%1}\n").arg(song.transpose));

  if (song.capo > 0)
    text.append(QString("  \\capo{%1}\n").arg(song.capo));

  foreach (QString gtab, song.gtabs)
    {
      text.append(QString("  %1\n").arg(gtab));
    }

  foreach (QString utab, song.utabs)
    {
      text.append(QString("  %1\n").arg(utab));
    }

  text.append(QString("\n"));

  foreach (QString lyric, song.lyrics)
    {
      text.append(QString("%1\n").arg(lyric));
    }

  text.append(QString("\\endsong\n"));

  foreach (QString line, song.scripture)
    {
      text.append(QString("%1\n").arg(line));
    }

  return text;
}

QLocale::Language Song::languageFromString(const QString &languageName)
{
  if (languageName == "french")
    return QLocale::French;
  else if (languageName == "english")
    return QLocale::English;
  else if (languageName == "spanish")
    return QLocale::Spanish;
  else if (languageName == "portuguese")
    return QLocale::Portuguese;
  else if (languageName == "italian")
    return QLocale::Italian;

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
    case QLocale::Portuguese:
      return "portuguese";
    case QLocale::Italian:
      return "italian";
    default:
      return "english";
    }
}

QString Song::latexToUtf8(const QString & str)
{
  QString result(str);
  result.replace(QRegExp("([^\\\\])~"), QString("\\1%1").arg(QChar(QChar::Nbsp)));
  result.replace(QRegExp("\\\\([&~])"), "\\1");
  result.replace(QRegExp("(\\{?\\\\dots\\}?|\\{?\\\\ldots\\}?)"),  "...");
  result.replace("\\%",  "%");
  return result;
}

QString Song::utf8ToLatex(const QString & str)
{
  QString result(str);
  result.replace(QRegExp("([&~])"), "\\\\1");
  result.replace(QChar(QChar::Nbsp), "~");
  result.replace("...", "\\dots");
  result.replace("%", "\\%");
  return result;
}
