// Copyright (C) 2009-2012, Romain Goffe <romain.goffe@gmail.com>
// Copyright (C) 2009-2012, Alexandre Dupas <alexandre.dupas@gmail.com>
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

#include "chord-list-model.hh"
#include "chord.hh"

#include <QMimeData>

#include <QDebug>

CChordListModel::CChordListModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_columnCount(0)
    , m_rowCount(0)
{
    m_fixedColumnCount = false;
    m_fixedRowCount = false;
}

CChordListModel::~CChordListModel()
{
    foreach (CChord* chord, m_data)
        delete chord;
    m_data.clear();
}

int CChordListModel::columnCount(const QModelIndex &) const
{
    return m_columnCount;
}

void CChordListModel::setColumnCount(int value)
{
    emit(layoutAboutToBeChanged());
    m_fixedColumnCount = true;
    m_columnCount = value;
    emit(layoutChanged());
}

int CChordListModel::rowCount(const QModelIndex &) const
{
    return m_rowCount;
}

void CChordListModel::setRowCount(int value)
{
    emit(layoutAboutToBeChanged());
    m_fixedRowCount = true;
    m_rowCount = value;
    emit(layoutChanged());
}

QVariant CChordListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || m_data.size()==0)
        return QVariant();

    switch (role)
    {
    case Qt::DisplayRole:
        return m_data[positionFromIndex(index)]->toString();
    case Qt::DecorationRole:
        return *(m_data[positionFromIndex(index)]->toPixmap());
    case Qt::ToolTipRole:
        return data(index, Qt::DisplayRole);
    case NameRole:
        return m_data[positionFromIndex(index)]->name();
    case StringsRole:
        return m_data[positionFromIndex(index)]->strings();
    case ImportantRole:
        return m_data[positionFromIndex(index)]->isImportant();
    default:
        return QVariant();
    }
}

bool CChordListModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    Q_UNUSED(role);

    if (!index.isValid())
        return false;

    CChord *chord = new CChord(value.toString());
    if (chord->isValid())
    {
        int pos = positionFromIndex(index);
        delete m_data[pos];
        m_data[pos] = chord;
        return true;
    }

    delete chord;
    return false;
}

void CChordListModel::insertItem(const QModelIndex & index, const QString & value)
{
    setColumnCount(columnCount() + 1);
    m_fixedColumnCount = false;
    int pos = index.column() < 0 ? columnCount()-1 : index.column();

    CChord *chord = new CChord(value);
    if (chord->isValid())
        m_data.insert(pos, chord);
    else
        delete chord;
}

void CChordListModel::removeItem(const QModelIndex & index)
{
    int pos = positionFromIndex(index);
    delete m_data[pos];
    m_data.remove(pos);

    int row = indexFromPosition(m_data.size()).row();
    int col = indexFromPosition(m_data.size()).column();

    // dynamically reduce the QListModel
    if (rowCount() > row +1)
    {
        setRowCount(row + 1);
        m_fixedRowCount = false;
    }
    if (columnCount() > col +1)
    {
        setColumnCount(col + 1);
        m_fixedColumnCount = false;
    }
}

void CChordListModel::addItem(const QString & value)
{
    CChord * chord = new CChord(value);
    if (!chord->isValid())
    {
        delete chord;
        return;
    }

    m_data.append(chord);

    int row = indexFromPosition(m_data.size()).row();
    int col = indexFromPosition(m_data.size()).column();

    // dynamically expand the QListModel
    if (rowCount() < row +1)
    {
        setRowCount(row + 1);
        m_fixedRowCount = false;
    }
    if (columnCount() < col +1)
    {
        setColumnCount(col + 1);
        m_fixedColumnCount = false;
    }
}

QModelIndex CChordListModel::indexFromPosition(int position)
{
    int row = 1, col = 1;
    if (m_fixedColumnCount)
    {
        row = (position-1)/columnCount();
        col = (position-1)%columnCount();
    }
    if (m_fixedRowCount)
    {
        row = (position-1)%rowCount();
        col = (position-1)/rowCount();
    }
    return QAbstractListModel::createIndex(row, col);
}

int CChordListModel::positionFromIndex(const QModelIndex & index) const
{
    return columnCount() * index.row() + index.column();
}

CChord * CChordListModel::getChord(const QModelIndex & index) const
{
    return m_data[positionFromIndex(index)];
}

Qt::DropActions CChordListModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

Qt::DropActions CChordListModel::supportedDragActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

Qt::ItemFlags CChordListModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractListModel::flags(index);

    if (index.isValid())
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
    else
        return Qt::ItemIsDropEnabled | defaultFlags;
}

QStringList CChordListModel::mimeTypes() const
{
    QStringList types;
    types << "text/plain";
    return types;
}

QMimeData * CChordListModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    foreach (const QModelIndex &index, indexes)
        if (index.isValid())
            mimeData->setText(data(index, Qt::DisplayRole).toString());

    return mimeData;
}

bool CChordListModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                   int row, int column, const QModelIndex &parent)
{
    if (action == Qt::IgnoreAction)
        return true;

    if (!data->hasFormat("text/plain"))
        return false;

    if (row > 1)
        return false;

    int beginColumn;
    if (column != -1)
        beginColumn = column;
    else if (parent.isValid())
        beginColumn = parent.column();
    else
        beginColumn = columnCount();

    QString newItem = data->text();
    insertItem(index(0, beginColumn), newItem);
    for(int j=0; j<columnCount(); ++j)
        if (m_data[j]->toString() == newItem && j != beginColumn)
        {
            removeItem(index(0, j));
            return true;
        }

    return true;
}
