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
#include "song-panel.hh"

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

  setColumnWidth(0,200);
  setColumnWidth(1,200);
  setColumnWidth(4,200);

  createActions();
}

CLibraryView::~CLibraryView()
{}

CMainWindow* CLibraryView::parent() const
{
  return m_parent;
}

void CLibraryView::readSettings()
{
  QSettings settings;
  settings.beginGroup("display");
  setColumnHidden(0, !settings.value("artist", true).toBool());
  setColumnHidden(1, !settings.value("title", true).toBool());
  setColumnHidden(2, !settings.value("lilypond", false).toBool());
  setColumnHidden(3, !settings.value("path", false).toBool());
  setColumnHidden(4, !settings.value("album", true).toBool());
  setColumnHidden(5, !settings.value("lang", true).toBool());

  setColumnWidth(0, settings.value("artistWidth", 200).toInt());
  setColumnWidth(1, settings.value("titleWidth", 200).toInt());
  setColumnWidth(2, settings.value("lilypondWidth", 40).toInt());
  setColumnWidth(3, settings.value("pathWidth", 200).toInt());
  setColumnWidth(4, settings.value("albumWidth", 200).toInt());
  setColumnWidth(5, settings.value("langWidth", 40).toInt());

  settings.endGroup();
}

void CLibraryView::writeSettings()
{
  QSettings settings;
  settings.beginGroup("display");
  settings.setValue("artistWidth", columnWidth(0));
  settings.setValue("titleWidth", columnWidth(1));
  settings.setValue("lilypondWidth", columnWidth(2));
  settings.setValue("pathWidth", columnWidth(3));
  settings.setValue("albumWidth", columnWidth(4));
  settings.setValue("langWidth", columnWidth(5));
  settings.endGroup();

}

void CLibraryView::update()
{
  sortByColumn(1, Qt::AscendingOrder);
  sortByColumn(0, Qt::AscendingOrder);
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

  action = new QAction(tr("Information"), this);
  connect(action, SIGNAL(triggered()),
  	  this, SLOT(songInfo()));
  addAction(action);
}

void CLibraryView::songInfo()
{
  QDialog* dialog = new QDialog;
  QDialogButtonBox * buttons = new QDialogButtonBox(QDialogButtonBox::Close);
  connect(buttons, SIGNAL(rejected()), dialog, SLOT(reject()));

  CSongPanel songPanel(this);
  songPanel.setLibrary(model());
  songPanel.setCurrentIndex(currentIndex());

  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(&songPanel);
  layout->addWidget(buttons);

  dialog->setLayout(layout);
  dialog->exec();
}
