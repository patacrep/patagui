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
#include "header.hh"
#include <QRegExp>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <iostream>
//------------------------------------------------------------------------------
CHeader::CHeader(const QString & ADir)
{
  //Last char proof
  QString tmp = ADir;
  while(tmp.endsWith("/"))
    tmp.remove(tmp.lastIndexOf("/"),1);

  m_workingPath = tmp;
}
//------------------------------------------------------------------------------
CHeader::~CHeader()
{}
//------------------------------------------------------------------------------
QString CHeader::title()
{
  return m_title;
}
//------------------------------------------------------------------------------
void CHeader::setTitle(const QString & ATitle)
{
  if(!ATitle.isEmpty())
    {
      m_title = ATitle;
      updateFile("\\\\title\\{([^}]+)", ATitle);
    }
}
//------------------------------------------------------------------------------
QString CHeader::subtitle()
{
  return m_subtitle;
}
//------------------------------------------------------------------------------
void CHeader::setSubtitle(const QString & ASubtitle)
{
  if(!ASubtitle.isEmpty())
    {
      m_subtitle = ASubtitle;
      updateFile("\\\\subtitle\\{([^}]+)", ASubtitle);
    }
}
//------------------------------------------------------------------------------
QString CHeader::author()
{
  return m_author;
}
//------------------------------------------------------------------------------
void CHeader::setAuthor(const QString & AAuthor)
{
  if(!AAuthor.isEmpty())
    {
      m_author = AAuthor;
      updateFile("\\\\author\\{([^}]+)", AAuthor);
    }
}
//------------------------------------------------------------------------------
QString CHeader::version()
{
  return m_version;
}
//------------------------------------------------------------------------------
void CHeader::setVersion(const QString & AVersion)
{
  if(!AVersion.isEmpty())
    {
      m_version = AVersion;
      updateFile("\\\\version\\{([^}]+)", AVersion);
    }
}
//------------------------------------------------------------------------------
QString CHeader::mail()
{
  return m_mail;
}
//------------------------------------------------------------------------------
void CHeader::setMail(const QString & AMail)
{
  if(!AMail.isEmpty())
    {
      m_mail = AMail;
      updateFile("\\\\mail\\{([^}]+)", AMail);
    }
}
//------------------------------------------------------------------------------
QString CHeader::picture()
{
  return m_picture;
}
//------------------------------------------------------------------------------
void CHeader::setPicture(const QString & APicture, bool isPath)
{
  if(!APicture.isEmpty())
    {
      if(isPath)
	{
	  //copy the picture in img/ directory so it can be included by latex
	  QFile file(APicture);
	  QFileInfo fi(APicture);
	  QString target = QString("%1/img/%2").arg(m_workingPath).arg(fi.fileName());
	  file.copy(target);
	  QString basename = fi.baseName();
	  if(!basename.isEmpty())
	    {
	      m_picture = APicture;
	      updateFile("\\\\picture\\{([^}]+)", basename);
	    }
	}
      else
	{
	  m_picture = APicture;
	  updateFile("\\\\picture\\{([^}]+)", APicture);
	}
    }
}
//------------------------------------------------------------------------------
QString CHeader::copyright()
{
  return m_copyright;
}
//------------------------------------------------------------------------------
void CHeader::setCopyright(const QString & ACopyright)
{
  if(!ACopyright.isEmpty())
    {
      m_copyright = ACopyright;
      updateFile("\\\\picturecopyright\\{([^}]+)", ACopyright);
    }
}
//------------------------------------------------------------------------------
void CHeader::updateFile(const QString & ARegExp, const QString & AOption)
{
  QFile file(QString("%1/mybook.tex").arg(m_workingPath));

  QString old, fileStr;
  if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {      
      QTextStream stream (&file);
      fileStr = stream.readAll();
      file.close();
      QRegExp rx(ARegExp);
      rx.indexIn(fileStr);
      old = rx.cap(1);
      fileStr.replace(old, AOption);
      if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
          QTextStream stream (&file);
          stream << fileStr;
          file.close();
        }
    }
  else
    {
      std::cerr << "CHeader warning: unable to open file in read mode" << std::endl;
    }
}
//------------------------------------------------------------------------------
void CHeader::retrieveFields()
{
  //title
  m_title = retrieveField("\\\\title\\{([^}]+)");
    
  //subtitle
  m_subtitle = retrieveField("\\\\subtitle\\{([^}]+)");

  //author
  m_author = retrieveField("\\\\author\\{([^}]+)");
  
  //version
  m_version = retrieveField("\\\\version\\{([^}]+)");

  //mail
  m_mail = retrieveField("\\\\mail\\{([^}]+)");

  //picture
  setPicture( retrieveField("\\\\picture\\{([^}]+)"), false );
 
  //picture copyright
  m_copyright =  retrieveField("\\\\picturecopyright\\{([^}]+)");
}
//------------------------------------------------------------------------------
QString CHeader::retrieveField(const QString & ARegExp)
{
  QString result, fileStr;
  QFile file(QString("%1/mybook.tex").arg(m_workingPath));
  if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {      
      QTextStream stream (&file);
      fileStr = stream.readAll();
      file.close();
      QRegExp rx(ARegExp);
      rx.indexIn(fileStr);
      result = rx.cap(1);
    }
  //  else
  //    {
  //      std::cerr << "CHeader warning: unable to open "
  //		<< QString("%1/mybook.tex").arg(m_workingPath).toStdString() 
  //		<< " in read mode" << std::endl;
  //    }
  return result;
}
