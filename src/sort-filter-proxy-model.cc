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

#include "sort-filter-proxy-model.hh"

CSortFilterProxyModel::CSortFilterProxyModel(QObject *parent)
  : QSortFilterProxyModel(parent)
{}
//------------------------------------------------------------------------------
bool CSortFilterProxyModel::filterAcceptsRow(int sourceRow,
					     const QModelIndex &sourceParent) const
{
  QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent); //Artist
  QModelIndex index1 = sourceModel()->index(sourceRow, 1, sourceParent); //Title
  QModelIndex index2 = sourceModel()->index(sourceRow, 4, sourceParent); //Album

  return sourceModel()->data(index0).toString().contains(filterRegExp()) || 
    sourceModel()->data(index1).toString().contains(filterRegExp()) ||
    sourceModel()->data(index2).toString().contains(filterRegExp());
}
//------------------------------------------------------------------------------
