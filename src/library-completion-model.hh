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
 * \file library-completion-model.hh
 */
#ifndef __LIBRARY_COMPLETION_MODEL_HH__
#define __LIBRARY_COMPLETION_MODEL_HH__

#include <QSortFilterProxyModel>

class CLibraryCompletionModel : public QSortFilterProxyModel
{
  Q_OBJECT

public:
  CLibraryCompletionModel(QObject *parent = 0);
  virtual ~CLibraryCompletionModel();

  virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
  virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
};

#endif // __LIBRARY_COMPLETION_MODEL_HH__
