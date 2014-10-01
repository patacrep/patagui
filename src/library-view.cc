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

#include <QAction>
#include <QHeaderView>
#include <QSettings>

#include "main-window.hh"

#include <QDebug>

CLibraryView::CLibraryView(CMainWindow *parent)
    : QTableView(parent)
{
    setStyleSheet(" QTableView {margin: -1px -1px}");
    setShowGrid(false);
    setAlternatingRowColors(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSortingEnabled(true);
    verticalHeader()->setVisible(false);

    setContextMenuPolicy(Qt::ActionsContextMenu);

    createActions();
}

CLibraryView::~CLibraryView()
{}

CMainWindow* CLibraryView::parent() const
{
    return qobject_cast< CMainWindow* >(QTableView::parent());
}

void CLibraryView::readSettings()
{
    QSettings settings;
    settings.beginGroup("display");
    setColumnHidden(0, !settings.value("title", true).toBool());
    setColumnHidden(1, !settings.value("artist", true).toBool());
    setColumnHidden(2, !settings.value("lilypond", false).toBool());
    setColumnHidden(3, !settings.value("website", false).toBool());
    setColumnHidden(4, !settings.value("path", false).toBool());
    setColumnHidden(5, !settings.value("album", true).toBool());
    setColumnHidden(6, !settings.value("lang", true).toBool());
    settings.endGroup();
}

void CLibraryView::writeSettings()
{
}

void CLibraryView::update()
{
    sortByColumn(0, Qt::AscendingOrder);
    sortByColumn(1, Qt::AscendingOrder);
}

void CLibraryView::createActions()
{
    connect(this, SIGNAL(doubleClicked(const QModelIndex &)),
            parent(), SLOT(songEditor(const QModelIndex &)));

    connect(this, SIGNAL(pressed(const QModelIndex &)),
            parent(), SLOT(middleClicked(const QModelIndex &)));

    QAction* action = new QAction(tr("Edit"), this);
    connect(action, SIGNAL(triggered()),
            parent(), SLOT(songEditor()));
    addAction(action);

    action = new QAction(tr("Delete"), this);
    connect(action, SIGNAL(triggered()),
            parent(), SLOT(deleteSong()));
    addAction(action);
}

void CLibraryView::resizeColumns()
{
    horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
    horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
}
