// Copyright (C) 2009-2011 Romain Goffe, Alexandre Dupas
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

#include "tab-widget.hh"

#include <QAction>
#include <QTabBar>

#include <QDebug>

//******************************************************************************
//******************************************************************************
CTabWidget::CTabWidget():QTabWidget()
{
  // change the tab mode on os x put between #ifdef __APPLE__ if necessary
  setDocumentMode(true);

  //
  tabBar()->hide();
  QAction* action = new QAction(tr("Next tab"), this);
  action->setShortcut(QKeySequence::NextChild);
  connect(action, SIGNAL(triggered()), this, SLOT(next()));
  action = new QAction(tr("Previous tab"), this);
  action->setShortcut(QKeySequence::PreviousChild);
  connect(action, SIGNAL(triggered()), this, SLOT(prev()));
}
//------------------------------------------------------------------------------
CTabWidget::~CTabWidget()
{}
//------------------------------------------------------------------------------
void CTabWidget::closeTab(int index)
{
  removeTab(index);
  if (count() == 1)
    tabBar()->hide();
}
//------------------------------------------------------------------------------
int CTabWidget::addTab(QWidget* widget, const QString & label)
{
  int res = QTabWidget::addTab(widget, label);
  tabBar()->show();
  if (count() == 1)
    tabBar()->hide();
  return res;
}
//------------------------------------------------------------------------------
void CTabWidget::next()
{
  setCurrentIndex(currentIndex()+1);
}
//------------------------------------------------------------------------------
void CTabWidget::prev()
{
  setCurrentIndex(currentIndex()-1);
}
//******************************************************************************
//******************************************************************************
