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

#include "chord-table-model.hh"
#include "chord.hh"

#include <QMimeData>

#include <QDebug>

CChordTableModel::CChordTableModel(QObject *parent)
  : QAbstractTableModel(parent)
  , m_columnCount(0)
  , m_rowCount(0)
{
  m_fixedColumnCount = false;
  m_fixedRowCount = false;
}

CChordTableModel::~CChordTableModel()
{
  foreach (CChord* chord, m_data)
    delete chord;
  m_data.clear();
}

int CChordTableModel::columnCount(const QModelIndex &) const
{
  return m_columnCount;
}

void CChordTableModel::setColumnCount(int value)
{
  emit(layoutAboutToBeChanged());
  m_fixedColumnCount = true;
  m_columnCount = value;
  emit(layoutChanged());
}

int CChordTableModel::rowCount(const QModelIndex &) const
{
  return m_rowCount;
}

void CChordTableModel::setRowCount(int value)
{
  emit(layoutAboutToBeChanged());
  m_fixedRowCount = true;
  m_rowCount = value;
  emit(layoutChanged());
}

QVariant CChordTableModel::data(const QModelIndex &index, int role) const
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

bool CChordTableModel::setData(const QModelIndex & index, const QVariant & value, int role)
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

void CChordTableModel::insertItem(const QModelIndex & index, const QString & value)
{
  setColumnCount(columnCount() + 1);
  m_fixedColumnCount = false;

  CChord *chord = new CChord(value);
  if (chord->isValid())
    m_data.insert(index.column(), chord);
  else
    delete chord;
}

void CChordTableModel::removeItem(const QModelIndex & index)
{
  int pos = positionFromIndex(index);
  delete m_data[pos];
  m_data.remove(pos);

  int row = indexFromPosition(m_data.size()).row();
  int col = indexFromPosition(m_data.size()).column();

  // dynamically reduce the QTableModel
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

void CChordTableModel::addItem(const QString & value)
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

  // dynamically expand the QTableModel
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

QModelIndex CChordTableModel::indexFromPosition(int position)
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
  return QAbstractTableModel::createIndex(row, col);
}

int CChordTableModel::positionFromIndex(const QModelIndex & index) const
{
  return columnCount() * index.row() + index.column();
}

CChord * CChordTableModel::getChord(const QModelIndex & index) const
{
  return m_data[positionFromIndex(index)];
}

Qt::DropActions CChordTableModel::supportedDropActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}

Qt::DropActions CChordTableModel::supportedDragActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}

Qt::ItemFlags CChordTableModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index);

  if (index.isValid())
    return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
  else
    return Qt::ItemIsDropEnabled | defaultFlags;
}

QStringList CChordTableModel::mimeTypes() const
{
  QStringList types;
  types << "text/plain";
  return types;
}

QMimeData * CChordTableModel::mimeData(const QModelIndexList &indexes) const
{
  QMimeData *mimeData = new QMimeData();
  foreach (const QModelIndex &index, indexes)
    if (index.isValid())
      mimeData->setText(data(index, Qt::DisplayRole).toString());

  return mimeData;
}

bool CChordTableModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
				 int row, int column, const QModelIndex &parent)
{
  if (action == Qt::IgnoreAction)
    return true;

  if (!data->hasFormat("text/plain"))
    return false;

  int beginRow;
  if (row != -1)
    beginRow = row;
  else if (parent.isValid())
    beginRow = parent.row();
  else
    beginRow = rowCount();

  int beginColumn;
  if (column != -1)
    beginColumn = column;
  else if (parent.isValid())
    beginColumn = parent.column();
  else
    beginColumn = columnCount();

  QString newItem = data->text();

  // disabled since it is still buggy
  Q_UNUSED(beginRow); Q_UNUSED(beginColumn);
  //insertItem(index(beginRow, beginColumn), newItem);

  return true;
}
