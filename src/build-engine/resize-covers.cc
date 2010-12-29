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
#include <QtGui>
#include "resize-covers.hh"

CResizeCovers::CResizeCovers(CMainWindow* AParent)
  : CBuildEngine(AParent)
{
  setFileName("./utils/resize-cover.sh");
  setWindowTitle(tr("Resize covers"));
  setStatusActionMessage(tr("Resizing covers. Please wait ..."));
  setStatusSuccessMessage(tr("Covers correctly resized."));
  setStatusErrorMessage(tr("An error occured while resizing covers."));
}

QWidget* CResizeCovers::mainWidget()
{
  QWidget* widget = new QListWidget;
  QStringList filter;
  filter << "*.jpg" << "*.png" << "*.JPG" ;
  QString path = QString("%1/songs/").arg(workingPath());
  QDirIterator it(path, filter, QDir::NoFilter, QDirIterator::Subdirectories);

  QColor green(138,226,52,100);
  QColor red(239,41,41,100);
  while(it.hasNext())
    {
      QString filename = it.next();
      QFileInfo fi(filename);
      QString name = fi.fileName();
      QPixmap pixmap = QPixmap::fromImage(QImage(filename));
      QIcon cover(pixmap.scaledToWidth(24));

      //create item from current cover
      QListWidgetItem* item = new QListWidgetItem(cover, name);
      if(pixmap.height()>128)
	item->setBackground(QBrush(red));
      else
	item->setBackground(QBrush(green));
      //apppend items
      static_cast<QListWidget*>(widget)->addItem(item);
    }
  return widget; 
}
