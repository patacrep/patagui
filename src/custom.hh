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
#ifndef CUSTOM_HH
#define CUSTOM_HH

#include <QString>
class CCustom
{

public:
  CCustom(const QString & ADir);
  virtual ~CCustom();
  
  QString colorBox();
  QString fontSize();
  
  void setColorBox(QString);
  void setFontSize(int);

  void retrieveFields();

private:
  void updateFile(const QString & ARegExp, const QString & AOption);
  QString retrieveField(const QString & ARegExp);

  QString m_workingPath;

  //Custom Fields of crepbook.cls
  QString m_colorBox;
  QString m_fontSize;
};
#endif
