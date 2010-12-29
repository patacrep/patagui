// Copyright (C) 2009 Romain Goffe, Alexandre Dupas
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
#include <QStringList>

#include "utils.hh"

namespace SbUtils
{
  //------------------------------------------------------------------------------
  QString latexToUtf8(const QString & AString)
  {
    QString str(AString);
    str.replace(QString("\\'e"), QString("é"));
    str.replace(QString("\\`e"), QString("è"));
    str.replace(QString("\\^e"), QString("ê"));
    str.replace(QString("\\¨e"), QString("ë"));
    str.replace(QString("\\¨i"), QString("ï"));
    str.replace(QString("\\^i"), QString("î"));
    str.replace(QString("\\^o"), QString("ô"));
    str.replace(QString("\\`u"), QString("ù"));
    str.replace(QString("\\`a"), QString("à"));
    str.replace(QString("\\^a"), QString("â"));
    str.replace(QString("\\&"), QString("&"));
    str.replace(QString("\\~"), QString("~"));
    str.replace(QString("~"), QString(" "));
    str.replace(QString("\\dots"), QString("..."));

    return str;
  }
  //------------------------------------------------------------------------------
  QString filenameToString(const QString AString)
  {
    QString str(AString);
    if (str.isEmpty())
      return str;

    //Set the first letter to uppercase
    str[0] = str[0].toUpper();
    str.replace(QString("_"), QString(" "));
    str.replace(QString("-"), QString(" "));
    return str;
  }
  //------------------------------------------------------------------------------
  QString stringToFilename(const QString & AString, const QString & sep)
  {
    QString str(AString);
    QString item;
  
    QStringList list = QStringList() 
      <<"é"<<"è"<<"ê"<<"ë";
  
    foreach(item, list)
      str.replace(item, QString("e"));

    list = QStringList() 
      << " "<<"&"<<"'"<<"`"<<"("<<")"<<"["<<"]"<<"{"<<"}"
      <<"_"<<"~"<<","<<"?"<<"!"<<":"<<";"<<"."<<"%";
  
    foreach(item, list)
      str.replace(item, sep);

    str.replace(QString("à"), QString("a"));
    str.replace(QString("â"), QString("a"));
    str.replace(QString("ï"), QString("i"));
    str.replace(QString("î"), QString("i"));
    str.replace(QString("ô"), QString("o"));
    str.replace(QString("ù"), QString("u"));

    return str;
  }
}
