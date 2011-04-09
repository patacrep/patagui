// Copyright 2011 Alexandre Dupas <alexandre.dupas@gmail.com>
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
 * \file library-completion-model.cc
 */

#include "library-completion-model.hh"

#include "library.hh"

#include <QDebug>

CLibraryCompletionModel::CLibraryCompletionModel(QObject *parent)
  : QSortFilterProxyModel(parent)
{
  setDynamicSortFilter(true);
}

CLibraryCompletionModel::~CLibraryCompletionModel()
{}

int CLibraryCompletionModel::rowCount(const QModelIndex & parent) const
{
  return QSortFilterProxyModel::rowCount(parent) * 3;
}

QVariant CLibraryCompletionModel::data(const QModelIndex &index, int role) const
{
  if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
      switch (index.row() % 3)
	{
	case 0:
	  role = CLibrary::TitleRole;
	  break;
	case 1:
	  role = CLibrary::ArtistRole;
	  break;
	case 2:
	  role = CLibrary::AlbumRole;
	  break;
	}
    }
  return QSortFilterProxyModel::data(sibling(index.row()/3, 0, index.parent()), role);
}
