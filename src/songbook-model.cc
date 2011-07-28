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
#include "songbook-model.hh"

#include "library.hh"

#include <QDebug>

CSongbookModel::CSongbookModel(QObject *parent)
  : QAbstractProxyModel(parent)
  , m_selectedSongs()
{}

CSongbookModel::~CSongbookModel()
{
  m_selectedSongs.clear();
}

void CSongbookModel::selectAll()
{
  for (int i = 0; i < m_selectedSongs.size(); ++i)
    {
      m_selectedSongs[i] = true;
    }
  emit(dataChanged(index(0,0),index(m_selectedSongs.size()-1,0)));
}

void CSongbookModel::unselectAll()
{
  for (int i = 0; i < m_selectedSongs.size(); ++i)
    {
      m_selectedSongs[i] = false;
    }
  emit(dataChanged(index(0,0),index(m_selectedSongs.size()-1,0)));
}

void CSongbookModel::invertSelection()
{
  for (int i = 0; i < m_selectedSongs.size(); ++i)
    {
      m_selectedSongs[i] = !m_selectedSongs[i];
    }
  emit(dataChanged(index(0,0),index(m_selectedSongs.size()-1,0)));
}

int CSongbookModel::selectedCount() const
{
  int count = 0;
  for (int i = 0; i < m_selectedSongs.size(); ++i)
    {
      if (m_selectedSongs[i])
	count++;
    }
  return count;
}

QStringList CSongbookModel::selectedPaths() const
{
  QStringList songPaths;

  for (int i = 0; i < m_selectedSongs.size(); ++i)
    {
      if (m_selectedSongs[i])
	songPaths << data(index(i,0), CLibrary::PathRole).toString();
    }
  return songPaths;
}

void CSongbookModel::selectLanguages(const QStringList &languages)
{
  for (int i = 0; i < m_selectedSongs.size(); ++i)
    {
      m_selectedSongs[i] = false;
      if (languages.contains(data(index(i,0), CLibrary::LanguageRole).toString()))
	m_selectedSongs[i] = true;
    }
  emit(dataChanged(index(0,0),index(m_selectedSongs.size()-1,0)));
}

bool CSongbookModel::selectPaths(QStringList &paths)
{
  bool ok = true;
  
  if (paths.count() == 0)
    unselectAll();

  for (int i = 0; i < m_selectedSongs.size(); ++i)
    {
      m_selectedSongs[i] = false;
      if (paths.contains(data(index(i,0), CLibrary::PathRole).toString()))
	m_selectedSongs[i] = true;
      else
	ok = false;
    }
  emit(dataChanged(index(0,0),index(m_selectedSongs.size()-1,0)));
  return ok;
}

QModelIndex CSongbookModel::mapFromSource(const QModelIndex &index) const
{
  if (!sourceModel() || !index.isValid())
    return QModelIndex();
  return createIndex(index.row(), index.column(), index.internalPointer());
}

QModelIndex CSongbookModel::mapToSource(const QModelIndex &index) const
{
  if (!sourceModel() || !index.isValid())
    return QModelIndex();
  return sourceModel()->index(index.row(), index.column());
}

QVariant CSongbookModel::data(const QModelIndex &index, int role) const
{
  if (index.column() == 0 && role == Qt::CheckStateRole)
    {
      return (m_selectedSongs[index.row()] ? Qt::Checked : Qt::Unchecked);
    }
  return QAbstractProxyModel::data(index, role);
}

Qt::ItemFlags CSongbookModel::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return Qt::NoItemFlags;
  return Qt::ItemIsUserCheckable | QAbstractProxyModel::flags(index);
}

bool CSongbookModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (index.column() == 0 && role == Qt::CheckStateRole)
    {
      m_selectedSongs[index.row()] = value.toBool();
      emit(dataChanged(index, index));
      return true;
    }
  return QAbstractProxyModel::setData(index, value, role);
}

int CSongbookModel::columnCount(const QModelIndex &parent) const
{
  return sourceModel()->columnCount(mapToSource(parent));
}

int CSongbookModel::rowCount(const QModelIndex &parent) const
{
  return sourceModel()->rowCount(mapToSource(parent));
}

QModelIndex CSongbookModel::index(int row, int column, const QModelIndex &parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();
  const QModelIndex sourceParent = mapToSource(parent);
  const QModelIndex sourceIndex = sourceModel()->index(row, column, sourceParent);
  return mapFromSource(sourceIndex);
}

QModelIndex CSongbookModel::parent(const QModelIndex &index) const
{
  const QModelIndex sourceIndex = mapToSource(index);
  const QModelIndex sourceParent = sourceIndex.parent();
  return mapFromSource(sourceParent);
}

void CSongbookModel::setSourceModel(QAbstractItemModel *sourceModel)
{
  beginResetModel();
  
  if (sourceModel) {
    disconnect(sourceModel, SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)),
	       this, SLOT(sourceRowsAboutToBeInserted(const QModelIndex &, int, int)));
    disconnect(sourceModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
	       this, SLOT(sourceRowsInserted(const QModelIndex &, int, int)));
    disconnect(sourceModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
	       this, SLOT(sourceRowsAboutToBeRemoved(const QModelIndex &, int, int)));
    disconnect(sourceModel, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
	       this, SLOT(sourceRowsRemoved(const QModelIndex &, int, int)));
    disconnect(sourceModel, SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
	       this, SLOT(sourceRowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
    disconnect(sourceModel, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
	       this, SLOT(sourceRowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
    disconnect(sourceModel, SIGNAL(columnsAboutToBeInserted(const QModelIndex &, int, int)),
	       this, SLOT(sourceColumnsAboutToBeInserted(const QModelIndex &, int, int)));
    disconnect(sourceModel, SIGNAL(columnsInserted(const QModelIndex &, int, int)),
	       this, SLOT(sourceColumnsInserted(const QModelIndex &, int, int)));
    disconnect(sourceModel, SIGNAL(columnsAboutToBeRemoved(const QModelIndex &, int, int)),
	       this, SLOT(sourceColumnsAboutToBeRemoved(const QModelIndex &, int, int)));
    disconnect(sourceModel, SIGNAL(columnsRemoved(const QModelIndex &, int, int)),
	       this, SLOT(sourceColumnsRemoved(const QModelIndex &, int, int)));
    disconnect(sourceModel, SIGNAL(columnsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
	       this, SLOT(sourceColumnsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
    disconnect(sourceModel, SIGNAL(columnsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
	       this, SLOT(sourceColumnsMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
    disconnect(sourceModel, SIGNAL(modelAboutToBeReset()),
	       this, SLOT(sourceModelAboutToBeReset()));
    disconnect(sourceModel, SIGNAL(modelReset()),
	       this, SLOT(sourceModelReset()));
    disconnect(sourceModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
	       this, SLOT(sourceDataChanged(const QModelIndex &, const QModelIndex &)));
    disconnect(sourceModel, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
	       this, SLOT(sourceHeaderDataChanged(Qt::Orientation,int,int)));
    disconnect(sourceModel, SIGNAL(layoutAboutToBeChanged()),
	       this, SLOT(sourceLayoutAboutToBeChanged()));
    disconnect(sourceModel, SIGNAL(layoutChanged()),
	       this, SLOT(sourceLayoutChanged()));
  }

  QAbstractProxyModel::setSourceModel(sourceModel);

  if (sourceModel) {
    connect(sourceModel, SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)),
	    SLOT(sourceRowsAboutToBeInserted(const QModelIndex &, int, int)));
    connect(sourceModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
	    SLOT(sourceRowsInserted(const QModelIndex &, int, int)));
    connect(sourceModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
	    SLOT(sourceRowsAboutToBeRemoved(const QModelIndex &, int, int)));
    connect(sourceModel, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
	    SLOT(sourceRowsRemoved(const QModelIndex &, int, int)));
    connect(sourceModel, SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
	    SLOT(sourceRowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
    connect(sourceModel, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
	    SLOT(sourceRowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
    connect(sourceModel, SIGNAL(columnsAboutToBeInserted(const QModelIndex &, int, int)),
	    SLOT(sourceColumnsAboutToBeInserted(const QModelIndex &, int, int)));
    connect(sourceModel, SIGNAL(columnsInserted(const QModelIndex &, int, int)),
	    SLOT(sourceColumnsInserted(const QModelIndex &, int, int)));
    connect(sourceModel, SIGNAL(columnsAboutToBeRemoved(const QModelIndex &, int, int)),
	    SLOT(sourceColumnsAboutToBeRemoved(const QModelIndex &, int, int)));
    connect(sourceModel, SIGNAL(columnsRemoved(const QModelIndex &, int, int)),
	    SLOT(sourceColumnsRemoved(const QModelIndex &, int, int)));
    connect(sourceModel, SIGNAL(columnsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
	    SLOT(sourceColumnsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
    connect(sourceModel, SIGNAL(columnsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
	    SLOT(sourceColumnsMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
    connect(sourceModel, SIGNAL(modelAboutToBeReset()),
	    SLOT(sourceModelAboutToBeReset()));
    connect(sourceModel, SIGNAL(modelReset()),
	    SLOT(sourceModelReset()));
    connect(sourceModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
	    SLOT(sourceDataChanged(const QModelIndex &, const QModelIndex &)));
    connect(sourceModel, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
	    SLOT(sourceHeaderDataChanged(Qt::Orientation,int,int)));
    connect(sourceModel, SIGNAL(layoutAboutToBeChanged()),
	    SLOT(sourceLayoutAboutToBeChanged()));
    connect(sourceModel, SIGNAL(layoutChanged()),
	    SLOT(sourceLayoutChanged()));
  }

  endResetModel();
}

void CSongbookModel::sourceColumnsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
  beginInsertColumns(mapFromSource(parent), start, end);
}

void CSongbookModel::sourceColumnsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest)
{
  beginMoveColumns(mapFromSource(sourceParent), sourceStart, sourceEnd, mapFromSource(destParent), dest);
}

void CSongbookModel::sourceColumnsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
  beginRemoveColumns(mapFromSource(parent), start, end);
}

void CSongbookModel::sourceColumnsInserted(const QModelIndex &parent, int start, int end)
{
  endInsertColumns();
}

void CSongbookModel::sourceColumnsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest)
{
  endMoveColumns();
}

void CSongbookModel::sourceColumnsRemoved(const QModelIndex &parent, int start, int end)
{
  endRemoveColumns();
}

void CSongbookModel::sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
  dataChanged(mapFromSource(topLeft), mapFromSource(bottomRight));
}

void CSongbookModel::sourceHeaderDataChanged(Qt::Orientation orientation, int first, int last)
{
  headerDataChanged(orientation, first, last);
}

void CSongbookModel::sourceLayoutAboutToBeChanged()
{
  layoutAboutToBeChanged();
}

void CSongbookModel::sourceLayoutChanged()
{
  layoutChanged();
}

void CSongbookModel::sourceModelAboutToBeReset()
{
  m_selectedPaths = selectedPaths();
  beginResetModel();
}

void CSongbookModel::sourceModelReset()
{
  m_selectedSongs.clear();
  for (int i = 0; i < sourceModel()->rowCount(); ++i)
    {
      m_selectedSongs << false;
    }
  selectPaths(m_selectedPaths);
  endResetModel();
}

void CSongbookModel::sourceRowsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
  beginInsertRows(mapFromSource(parent), start, end);
}

void CSongbookModel::sourceRowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest)
{
  beginMoveRows(mapFromSource(sourceParent), sourceStart, sourceEnd, mapFromSource(destParent), dest);
}

void CSongbookModel::sourceRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
  beginRemoveRows(mapFromSource(parent), start, end);
}

void CSongbookModel::sourceRowsInserted(const QModelIndex &parent, int start, int end)
{
  endInsertRows();
}

void CSongbookModel::sourceRowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest)
{
  endMoveRows();
}

void CSongbookModel::sourceRowsRemoved(const QModelIndex &parent, int start, int end)
{
  endRemoveRows();
}
