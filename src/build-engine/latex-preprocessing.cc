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
#include "latex-preprocessing.hh"

CLatexPreprocessing::CLatexPreprocessing(CMainWindow* AParent)
  : CBuildEngine(AParent)
{
  setFileName("./utils/latex-preprocessing.py");
  setWindowTitle(tr("LaTeX preprocessing"));
  setStatusActionMessage(tr("Check \".sg\" files for common mistakes. "
			    "Please wait ..."));
}

QWidget* CLatexPreprocessing::mainWidget()
{
  QWidget* widget = new QListWidget;
  QColor orange(252,175,62,150);
  QColor yellow(252,233,79,150);

  //Retrieve rules from ./utils/latex-preprocessing file
  QFile file(QString("%1/utils/latex-preprocessing.py").arg(workingPath()));
  if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      QTextStream in(&file);
      QRegExp filter("^##:");
      QString line;
      bool rule = false;
      QListWidgetItem *item;
      do {
        line = in.readLine();

	if (line.startsWith("###")) //end of rules
	  break;

        if (line.startsWith("##:")) //category
	  {
	    item = new QListWidgetItem("\n" + line.remove(filter) + "\n");
	    item->setBackground(QBrush(orange));
	    static_cast<QListWidget*>(widget)->addItem(item);
	    rule = true;
	  }
	else if(rule)
	  {
	    if(line.startsWith("#")) //subcategory
	      {
		item = new QListWidgetItem(line);
		item->setBackground(QBrush(yellow));
	      }
	    else
	      {
		line.replace(":", "  ->  ");
		line.chop(1);
		item = new QListWidgetItem(line);
	      }
	    static_cast<QListWidget*>(widget)->addItem(item);
	  }
      } while (!line.isNull());
      file.close();
    }
  else
    {
      qWarning() << "CLatexPreprocessing: unable to open file in read mode: utils/latex-preprocessing";
    }
  return widget;
}
