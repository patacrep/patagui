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

/**
 * \file sort-filter-proxy-model.hh
 *
 * Subclassing for custom filtering.
 *
 */

#ifndef __SORT_FILTER_PROXY_MODEL_HH__
#define __SORT_FILTER_PROXY_MODEL_HH__

#include <QDate>
#include <QSortFilterProxyModel>

class CSortFilterProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT
  
  public:
  CSortFilterProxyModel(QObject *parent = 0);
  
  protected:
  bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
};

#endif  // __SORT_FILTER_PROXY_MODEL_HH__
