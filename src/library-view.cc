// Copyright (C) 2009-2011, Romain Goffe <romain.goffe@gmail.com>
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
#include "library-view.hh"

#include "main-window.hh"

CLibraryView::CLibraryView(CMainWindow *parent)
  : QTableView()
  , m_parent(parent)
{
  setShowGrid(false);
  setAlternatingRowColors(true);
  setSelectionMode(QAbstractItemView::MultiSelection);
  setSelectionBehavior(QAbstractItemView::SelectRows);
  setEditTriggers(QAbstractItemView::NoEditTriggers);
  setSortingEnabled(true);
  verticalHeader()->setVisible(false);
  horizontalHeader()->setStretchLastSection(true);
  setContextMenuPolicy(Qt::ActionsContextMenu);

  createActions();
}

CLibraryView::~CLibraryView()
{}

CMainWindow* CLibraryView::parent() const
{
  return m_parent;
}

void CLibraryView::createActions()
{
  connect(this, SIGNAL(doubleClicked(const QModelIndex &)),
	  parent(), SLOT(songEditor(const QModelIndex &)));

  QAction* action = new QAction(tr("Edit"), this);
  connect(action, SIGNAL(triggered()),
	  parent(), SLOT(songEditor()));
  addAction(action);
  
  action = new QAction(tr("Delete"), this);
  connect(action, SIGNAL(triggered()),
	  parent(), SLOT(deleteSong()));
  addAction(action);

  //  action = new QAction(tr("Information"), this);
  //  connect(action, SIGNAL(triggered()),
  //	  this, SLOT(songInfo()));
  // addAction(action);
}
