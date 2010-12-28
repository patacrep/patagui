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
#include "make-songbook.hh"

CMakeSongbook::CMakeSongbook(CMainWindow* AParent)
  : CBuildEngine(AParent)
{
  setFileName("make");
  
  setProcessOptions(QStringList());
  
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LATEX_OPTIONS", "-halt-on-error");
  m_process->setProcessEnvironment(env);
}

QWidget* CMakeSongbook::mainWidget()
{
  return NULL;
}

void CMakeSongbook::setProcessOptions(const QStringList & value)
{
  if(value.contains("clean") || value.contains("cleanall"))
    {
      setStatusActionMessage(tr("Removing temporary LaTeX files. Please wait..."));
      setStatusSuccessMessage(tr("Cleaning completed."));
      setStatusErrorMessage(tr("An error occured during the cleaning operation."));
    }
  else
    {
      setStatusActionMessage(tr("Building the songbook. Please wait..."));
      setStatusSuccessMessage(tr("Songbook successfully generated."));
      setStatusErrorMessage(tr("An error occured during the songbook generation.\n " 
			       "You may check compilation logs for more information."));
    }
}
