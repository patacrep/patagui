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
#include "identity-proxy-model.hh"

#include <QDebug>

IdentityProxyModel::IdentityProxyModel(QObject *parent)
    : QAbstractProxyModel(parent)
{
}

IdentityProxyModel::~IdentityProxyModel() {}

QModelIndex IdentityProxyModel::mapFromSource(const QModelIndex &index) const
{
    if (!sourceModel() || !index.isValid())
        return QModelIndex();
    return createIndex(index.row(), index.column(), index.internalPointer());
}

QModelIndex IdentityProxyModel::mapToSource(const QModelIndex &index) const
{
    if (!sourceModel() || !index.isValid())
        return QModelIndex();
    return sourceModel()->index(index.row(), index.column());
}

QVariant IdentityProxyModel::data(const QModelIndex &index, int role) const
{
    return QAbstractProxyModel::data(index, role);
}

Qt::ItemFlags IdentityProxyModel::flags(const QModelIndex &index) const
{
    return QAbstractProxyModel::flags(index);
}

bool IdentityProxyModel::setData(const QModelIndex &index,
                                 const QVariant &value, int role)
{
    return QAbstractProxyModel::setData(index, value, role);
}

int IdentityProxyModel::columnCount(const QModelIndex &parent) const
{
    return sourceModel()->columnCount(mapToSource(parent));
}

int IdentityProxyModel::rowCount(const QModelIndex &parent) const
{
    return sourceModel()->rowCount(mapToSource(parent));
}

QModelIndex IdentityProxyModel::index(int row, int column,
                                      const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();
    const QModelIndex sourceParent = mapToSource(parent);
    const QModelIndex sourceIndex =
        sourceModel()->index(row, column, sourceParent);
    return mapFromSource(sourceIndex);
}

QModelIndex IdentityProxyModel::parent(const QModelIndex &index) const
{
    const QModelIndex sourceIndex = mapToSource(index);
    const QModelIndex sourceParent = sourceIndex.parent();
    return mapFromSource(sourceParent);
}

void IdentityProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    beginResetModel();

    if (sourceModel) {
        disconnect(
            sourceModel,
            SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)), this,
            SLOT(sourceRowsAboutToBeInserted(const QModelIndex &, int, int)));
        disconnect(sourceModel,
                   SIGNAL(rowsInserted(const QModelIndex &, int, int)), this,
                   SLOT(sourceRowsInserted(const QModelIndex &, int, int)));
        disconnect(
            sourceModel,
            SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)), this,
            SLOT(sourceRowsAboutToBeRemoved(const QModelIndex &, int, int)));
        disconnect(sourceModel,
                   SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this,
                   SLOT(sourceRowsRemoved(const QModelIndex &, int, int)));
        disconnect(sourceModel,
                   SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int,
                                             const QModelIndex &, int)),
                   this,
                   SLOT(sourceRowsAboutToBeMoved(const QModelIndex &, int, int,
                                                 const QModelIndex &, int)));
        disconnect(sourceModel, SIGNAL(rowsMoved(const QModelIndex &, int, int,
                                                 const QModelIndex &, int)),
                   this, SLOT(sourceRowsMoved(const QModelIndex &, int, int,
                                              const QModelIndex &, int)));
        disconnect(
            sourceModel,
            SIGNAL(columnsAboutToBeInserted(const QModelIndex &, int, int)),
            this, SLOT(sourceColumnsAboutToBeInserted(const QModelIndex &, int,
                                                      int)));
        disconnect(sourceModel,
                   SIGNAL(columnsInserted(const QModelIndex &, int, int)), this,
                   SLOT(sourceColumnsInserted(const QModelIndex &, int, int)));
        disconnect(
            sourceModel,
            SIGNAL(columnsAboutToBeRemoved(const QModelIndex &, int, int)),
            this,
            SLOT(sourceColumnsAboutToBeRemoved(const QModelIndex &, int, int)));
        disconnect(sourceModel,
                   SIGNAL(columnsRemoved(const QModelIndex &, int, int)), this,
                   SLOT(sourceColumnsRemoved(const QModelIndex &, int, int)));
        disconnect(sourceModel,
                   SIGNAL(columnsAboutToBeMoved(const QModelIndex &, int, int,
                                                const QModelIndex &, int)),
                   this, SLOT(sourceColumnsAboutToBeMoved(
                             const QModelIndex &, int, int, const QModelIndex &,
                             int)));
        disconnect(sourceModel,
                   SIGNAL(columnsMoved(const QModelIndex &, int, int,
                                       const QModelIndex &, int)),
                   this, SLOT(sourceColumnsMoved(const QModelIndex &, int, int,
                                                 const QModelIndex &, int)));
        disconnect(sourceModel, SIGNAL(modelAboutToBeReset()), this,
                   SLOT(sourceModelAboutToBeReset()));
        disconnect(sourceModel, SIGNAL(modelReset()), this,
                   SLOT(sourceModelReset()));
        disconnect(
            sourceModel,
            SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), this,
            SLOT(sourceDataChanged(const QModelIndex &, const QModelIndex &)));
        disconnect(sourceModel,
                   SIGNAL(headerDataChanged(Qt::Orientation, int, int)), this,
                   SLOT(sourceHeaderDataChanged(Qt::Orientation, int, int)));
        disconnect(sourceModel, SIGNAL(layoutAboutToBeChanged()), this,
                   SLOT(sourceLayoutAboutToBeChanged()));
        disconnect(sourceModel, SIGNAL(layoutChanged()), this,
                   SLOT(sourceLayoutChanged()));
    }

    QAbstractProxyModel::setSourceModel(sourceModel);

    if (sourceModel) {
        connect(
            sourceModel,
            SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)),
            SLOT(sourceRowsAboutToBeInserted(const QModelIndex &, int, int)));
        connect(sourceModel,
                SIGNAL(rowsInserted(const QModelIndex &, int, int)),
                SLOT(sourceRowsInserted(const QModelIndex &, int, int)));
        connect(
            sourceModel,
            SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
            SLOT(sourceRowsAboutToBeRemoved(const QModelIndex &, int, int)));
        connect(sourceModel, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
                SLOT(sourceRowsRemoved(const QModelIndex &, int, int)));
        connect(sourceModel,
                SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int,
                                          const QModelIndex &, int)),
                SLOT(sourceRowsAboutToBeMoved(const QModelIndex &, int, int,
                                              const QModelIndex &, int)));
        connect(sourceModel, SIGNAL(rowsMoved(const QModelIndex &, int, int,
                                              const QModelIndex &, int)),
                SLOT(sourceRowsMoved(const QModelIndex &, int, int,
                                     const QModelIndex &, int)));
        connect(sourceModel,
                SIGNAL(columnsAboutToBeInserted(const QModelIndex &, int, int)),
                SLOT(sourceColumnsAboutToBeInserted(const QModelIndex &, int,
                                                    int)));
        connect(sourceModel,
                SIGNAL(columnsInserted(const QModelIndex &, int, int)),
                SLOT(sourceColumnsInserted(const QModelIndex &, int, int)));
        connect(
            sourceModel,
            SIGNAL(columnsAboutToBeRemoved(const QModelIndex &, int, int)),
            SLOT(sourceColumnsAboutToBeRemoved(const QModelIndex &, int, int)));
        connect(sourceModel,
                SIGNAL(columnsRemoved(const QModelIndex &, int, int)),
                SLOT(sourceColumnsRemoved(const QModelIndex &, int, int)));
        connect(sourceModel,
                SIGNAL(columnsAboutToBeMoved(const QModelIndex &, int, int,
                                             const QModelIndex &, int)),
                SLOT(sourceColumnsAboutToBeMoved(const QModelIndex &, int, int,
                                                 const QModelIndex &, int)));
        connect(sourceModel, SIGNAL(columnsMoved(const QModelIndex &, int, int,
                                                 const QModelIndex &, int)),
                SLOT(sourceColumnsMoved(const QModelIndex &, int, int,
                                        const QModelIndex &, int)));
        connect(sourceModel, SIGNAL(modelAboutToBeReset()),
                SLOT(sourceModelAboutToBeReset()));
        connect(sourceModel, SIGNAL(modelReset()), SLOT(sourceModelReset()));
        connect(
            sourceModel,
            SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
            SLOT(sourceDataChanged(const QModelIndex &, const QModelIndex &)));
        connect(sourceModel,
                SIGNAL(headerDataChanged(Qt::Orientation, int, int)),
                SLOT(sourceHeaderDataChanged(Qt::Orientation, int, int)));
        connect(sourceModel, SIGNAL(layoutAboutToBeChanged()),
                SLOT(sourceLayoutAboutToBeChanged()));
        connect(sourceModel, SIGNAL(layoutChanged()),
                SLOT(sourceLayoutChanged()));
    }

    endResetModel();
}

void IdentityProxyModel::sourceColumnsAboutToBeInserted(
    const QModelIndex &parent, int start, int end)
{
    beginInsertColumns(mapFromSource(parent), start, end);
}

void IdentityProxyModel::sourceColumnsAboutToBeMoved(
    const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
    const QModelIndex &destParent, int dest)
{
    beginMoveColumns(mapFromSource(sourceParent), sourceStart, sourceEnd,
                     mapFromSource(destParent), dest);
}

void IdentityProxyModel::sourceColumnsAboutToBeRemoved(
    const QModelIndex &parent, int start, int end)
{
    beginRemoveColumns(mapFromSource(parent), start, end);
}

void IdentityProxyModel::sourceColumnsInserted(const QModelIndex &parent,
                                               int start, int end)
{
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
    endInsertColumns();
}

void IdentityProxyModel::sourceColumnsMoved(const QModelIndex &sourceParent,
                                            int sourceStart, int sourceEnd,
                                            const QModelIndex &destParent,
                                            int dest)
{
    Q_UNUSED(sourceParent);
    Q_UNUSED(sourceStart);
    Q_UNUSED(sourceEnd);
    Q_UNUSED(destParent);
    Q_UNUSED(dest);
    endMoveColumns();
}

void IdentityProxyModel::sourceColumnsRemoved(const QModelIndex &parent,
                                              int start, int end)
{
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
    endRemoveColumns();
}

void IdentityProxyModel::sourceDataChanged(const QModelIndex &topLeft,
                                           const QModelIndex &bottomRight)
{
    dataChanged(mapFromSource(topLeft), mapFromSource(bottomRight));
}

void IdentityProxyModel::sourceHeaderDataChanged(Qt::Orientation orientation,
                                                 int first, int last)
{
    headerDataChanged(orientation, first, last);
}

void IdentityProxyModel::sourceLayoutAboutToBeChanged()
{
    layoutAboutToBeChanged();
}

void IdentityProxyModel::sourceLayoutChanged() { layoutChanged(); }

void IdentityProxyModel::sourceModelAboutToBeReset() { beginResetModel(); }

void IdentityProxyModel::sourceModelReset() { endResetModel(); }

void IdentityProxyModel::sourceRowsAboutToBeInserted(const QModelIndex &parent,
                                                     int start, int end)
{
    beginInsertRows(mapFromSource(parent), start, end);
}

void IdentityProxyModel::sourceRowsAboutToBeMoved(
    const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
    const QModelIndex &destParent, int dest)
{
    beginMoveRows(mapFromSource(sourceParent), sourceStart, sourceEnd,
                  mapFromSource(destParent), dest);
}

void IdentityProxyModel::sourceRowsAboutToBeRemoved(const QModelIndex &parent,
                                                    int start, int end)
{
    beginRemoveRows(mapFromSource(parent), start, end);
}

void IdentityProxyModel::sourceRowsInserted(const QModelIndex &parent,
                                            int start, int end)
{
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
    endInsertRows();
}

void IdentityProxyModel::sourceRowsMoved(const QModelIndex &sourceParent,
                                         int sourceStart, int sourceEnd,
                                         const QModelIndex &destParent,
                                         int dest)
{
    Q_UNUSED(sourceParent);
    Q_UNUSED(sourceStart);
    Q_UNUSED(sourceEnd);
    Q_UNUSED(destParent);
    Q_UNUSED(dest);
    endMoveRows();
}

void IdentityProxyModel::sourceRowsRemoved(const QModelIndex &parent, int start,
                                           int end)
{
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
    endRemoveRows();
}
