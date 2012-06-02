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
#include <QStringList>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QDebug>
#include <QRegExp>

#include "utils.hh"

namespace SbUtils
{

  QString stringToFilename(const QString & AString, const QString & sep)
  {
    QString str(AString.toLower());
  
    //replace whitespaces with separator
    str.replace(QRegExp("(\\s+)|(\\W+)"), sep);
    str.remove(QRegExp("_+$"));

    //replace utf8 characters
    str.replace(QRegExp("[àâ]"), "a");
    str.replace(QRegExp("[ïî]"), "i");
    str.replace(QRegExp("[óô]"), "o");
    str.replace(QRegExp("[ùúû]"), "u");
    str.replace(QRegExp("[éèêë]"), "e");
    str.replace(QString("ñ"), QString("n"));
    str.replace(QString("ç"), QString("c"));

    return str;
  }
  //------------------------------------------------------------------------------
  bool copyFile(const QString & ASourcePath, const QString & ATargetDirectory)
  {
    //qDebug() << "Copying file " << ASourcePath << " to the direcotry " << ATargetDirectory;
    QFile sourceFile(ASourcePath);
    if(sourceFile.exists())
      {
	QFileInfo sourceFileInfo(ASourcePath);
	QString targetPath = QString("%1/%2").arg(ATargetDirectory).arg(sourceFileInfo.fileName());
	QFile targetFile(targetPath);
	QFileInfo targetFileInfo(targetPath);

	//todo: Q_OBJECT/CMakeList -> to use tr macro for MessageBox strings
	//ask for confirmation
	if( targetFile.exists() && 
	    QMessageBox::question(NULL, QString("File conflict"),
				  QString("Replace the file \"%1\" ?").arg(targetFileInfo.fileName()),
				  QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton) == QMessageBox::Yes)
	  {
	    targetFile.remove();
	  }
	return sourceFile.copy(targetPath); //QFile::copy does not overwrite data
      }
    return false;
  }
}
