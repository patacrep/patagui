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

#include "songSortFilterProxyModel.hh"

#include "songbook.hh"

#include <QDebug>

CSongSortFilterProxyModel::CSongSortFilterProxyModel(QObject *parent)
  : QSortFilterProxyModel(parent)
{}

CSongSortFilterProxyModel::~CSongSortFilterProxyModel()
{}

bool CSongSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
  QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
  QModelIndex index1 = sourceModel()->index(sourceRow, 1, sourceParent);
  QModelIndex index2 = sourceModel()->index(sourceRow, 4, sourceParent);

  return sourceModel()->data(index0).toString().contains(filterRegExp())
    || sourceModel()->data(index1).toString().contains(filterRegExp())
    || sourceModel()->data(index2).toString().contains(filterRegExp());
}

void CSongSortFilterProxyModel::selectAll()
{
  int rows = rowCount();
  CSongbook *songbook = qobject_cast< CSongbook* >(sourceModel());
  for (int i = 0; i < rows; ++i)
    {
      songbook->setChecked(mapToSource(index(i,0)), true);
    }
}

void CSongSortFilterProxyModel::unselectAll()
{
  int rows = rowCount();
  CSongbook *songbook = qobject_cast< CSongbook* >(sourceModel());
  for (int i = 0; i < rows; ++i)
    {
      songbook->setChecked(mapToSource(index(i,0)), false);
    }
}

void CSongSortFilterProxyModel::invertSelection()
{
  int rows = rowCount();
  CSongbook *songbook = qobject_cast< CSongbook* >(sourceModel());
  for (int i = 0; i < rows; ++i)
    {
      songbook->toggle(mapToSource(index(i,0)));
    }
}
