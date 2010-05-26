// Copyright (C) 2010 Romain Goffe, Alexandre Dupas
//
// Songbook Creator is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// Songbook Creator is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
// MA  02110-1301, USA.
//******************************************************************************
#include "songbook.hh"

#include <QFile>
#include <QTextStream>

#include <QDebug>

CSongbook::CSongbook()
  : QObject()
  , m_title()
  , m_subtitle()
  , m_author()
  , m_version()
  , m_mail()
  , m_picture()
  , m_pictureCopyright()
  , m_shadeColor()
  , m_fontSize()
  , m_songs()
{}

CSongbook::~CSongbook()
{}

QString CSongbook::title()
{
  return m_title;
}
void CSongbook::setTitle(const QString &title)
{
  m_title = title;
}

QString CSongbook::subtitle()
{
  return m_subtitle;
}
void CSongbook::setSubtitle(const QString &subtitle)
{
  m_subtitle = subtitle;
}

QString CSongbook::author()
{
  return m_author;
}
void CSongbook::setAuthor(const QString &author)
{
  m_author = author;
}

QString CSongbook::version()
{
  return m_version;
}
void CSongbook::setVersion(const QString &version)
{
  m_version = version;
}

QString CSongbook::mail()
{
  return m_mail;
}
void CSongbook::setMail(const QString &mail)
{
  m_mail = mail;
}

QString CSongbook::picture()
{
  return m_picture;
}
void CSongbook::setPicture(const QString &picture)
{
  m_picture = picture;
}

QString CSongbook::pictureCopyright()
{
  return m_pictureCopyright;
}
void CSongbook::setPictureCopyright(const QString &pictureCopyright)
{
  m_pictureCopyright = pictureCopyright;
}

QString CSongbook::shadeColor()
{
  return m_shadeColor;
}
void CSongbook::setShadeColor(const QString &shadeColor)
{
  m_shadeColor = shadeColor;
}

QString CSongbook::fontSize()
{
  return m_fontSize;
}
void CSongbook::setFontSize(QString &fontSize)
{
  m_fontSize = fontSize;
}


QStringList CSongbook::songs()
{
  return m_songs;
}

void CSongbook::setSongs(QStringList songs)
{
  m_songs = songs;
}

void CSongbook::save(QString & filename)
{
  QFile file(filename);
  if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
      QTextStream out(&file);
      out << "\\title{" << title() << "}\n";
      out << "\\subtitle{" << subtitle() << "}\n";
      out << "\\author{" << author() << "}\n";
      out << "\\version{" << version() << "}\n";
      out << "\\mail{" << mail() << "}\n";
      out << "\\picture{" << picture() << "}\n";
      out << "\\picturecopyright{" << pictureCopyright() << "}\n";
      out << "\\definecolor{SongbookShade}{HTML}{" << shadeColor() << "}\n";
      out << "\\renewcommand{\\lyricfont}{\\normalfont" << fontSize() << "}\n";
      out << "\\songlist{\n" <<(m_songs.join("\n")) << "\n}\n";
      file.close();
    }
  else
    {
      qWarning() << "unable to open file in write mode";
    }
}

void CSongbook::load(QString & filename)
{
  QFile file(filename);
  if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      QTextStream in(&file);
      setSongs(in.readAll().split("\n", QString::SkipEmptyParts));
      file.close();
    }
  else
    {
      qWarning() << "unable to open file in read mode";
    } 
}
