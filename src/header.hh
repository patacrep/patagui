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
#ifndef HEADER_HH
#define HEADER_HH

#include <QString>
class CHeader
{

public:
  CHeader(const QString & ADir);
  virtual ~CHeader();
  
  QString title();
  QString subtitle();
  QString author();
  QString version();
  QString mail();
  QString picture();
  QString copyright();
  
  void setTitle(const QString &);
  void setSubtitle(const QString &);
  void setAuthor(const QString &);
  void setVersion(const QString &);
  void setMail(const QString &);
  void setPicture(const QString &);
  void setCopyright(const QString &);

private:
  void updateFile(const QString & ARegExp, const QString & AOption);

  QString m_title;
  QString m_subtitle;
  QString m_author;
  QString m_version;
  QString m_mail;
  QString m_picture;
  QString m_copyright;
  QString m_workingPath;

};
#endif
