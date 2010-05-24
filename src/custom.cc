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
#include "custom.hh"
#include <QRegExp>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <iostream>
//------------------------------------------------------------------------------
CCustom::CCustom(const QString & ADir)
{
  //Last char proof
  QString tmp = ADir;
  while(tmp.endsWith("/"))
    tmp.remove(tmp.lastIndexOf("/"),1);

  m_workingPath = tmp;
}
//------------------------------------------------------------------------------
CCustom::~CCustom()
{}
//------------------------------------------------------------------------------
QString CCustom::colorBox()
{
  return m_colorBox;
}
//------------------------------------------------------------------------------
void CCustom::setColorBox(QString AColor)
{
  if(!AColor.isEmpty())
    {
      AColor.remove(QChar('#'));
      m_colorBox = AColor;
      updateFile("\\\\definecolor\\{SongbookShade\\}\\{HTML\\}\\{([^}]+)", AColor);
    }
}
//------------------------------------------------------------------------------
QString CCustom::fontSize()
{
  return m_fontSize;
}
//------------------------------------------------------------------------------
void CCustom::setFontSize(int ASize)
{
  QString latexsize;
  switch(ASize)
    {
    case 0:
      latexsize = "footnotesize";
      break;
    case 1:
      latexsize = "small";
      break;
    case 2:
      latexsize = "normalsize";
      break;
    case 3:
      latexsize = "large";
      break;
    case 4:
      latexsize = "Large";
      break;
    default:
      latexsize = "normalsize";
    }

  m_fontSize = latexsize;
  updateFile("\\\\renewcommand\\{\\\\lyricfont\\}\\{\\\\normalfont\\\\([^}]+)", latexsize);
}
//------------------------------------------------------------------------------
void CCustom::updateFile(const QString & ARegExp, const QString & AOption)
{
  QFile file(QString("%1/crepbook.cls").arg(m_workingPath));

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
      std::cerr << "CCustom warning: unable to open file in read mode" << std::endl;
    }
}
//------------------------------------------------------------------------------
void CCustom::retrieveFields()
{
  //boxes color
  m_colorBox = retrieveField("\\\\definecolor\\{SongbookShade\\}\\{HTML\\}\\{([^}]+)");

  //font size
  m_fontSize = retrieveField("\\\\renewcommand\\{\\\\lyricfont\\}\\{\\\\normalfont\\\\([^}]+)");
}
//------------------------------------------------------------------------------
QString CCustom::retrieveField(const QString & ARegExp)
{
  QString result, fileStr;
  QFile file(QString("%1/crepbook.cls").arg(m_workingPath));
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
  //      std::cerr << "CCustom warning: unable to open "
  //		<< QString("%1/crepbook.cls").arg(m_workingPath).toStdString() 
  //		<< " in read mode" << std::endl;
  //    }
  return result;
}
