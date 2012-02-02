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
#ifndef __IDENTITY_PROXY_MODEL_HH__
#define __IDENTITY_PROXY_MODEL_HH__

#include <QAbstractProxyModel>

// Class based on the identity proxy model from Qt 4.8

class CIdentityProxyModel : public QAbstractProxyModel
{
  Q_OBJECT

public:
  CIdentityProxyModel(QObject *parent = 0);
  ~CIdentityProxyModel();

  virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  virtual Qt::ItemFlags flags(const QModelIndex &index) const;
  virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

  virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
  virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;

  virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
  virtual QModelIndex parent(const QModelIndex &index ) const;

  virtual QModelIndex mapFromSource(const QModelIndex &sourceIndex) const;
  virtual QModelIndex mapToSource(const QModelIndex &proxyIndex) const;

  virtual void setSourceModel(QAbstractItemModel *sourceModel);

private slots:
  void sourceColumnsAboutToBeInserted(const QModelIndex &parent, int start, int end);
  void sourceColumnsInserted(const QModelIndex &parent, int start, int end);
  void sourceColumnsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest);
  void sourceColumnsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest);
  void sourceColumnsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
  void sourceColumnsRemoved(const QModelIndex &parent, int start, int end);

  void sourceRowsAboutToBeInserted(const QModelIndex &parent, int start, int end);
  void sourceRowsInserted(const QModelIndex &parent, int start, int end);
  void sourceRowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest);
  void sourceRowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest);
  void sourceRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
  void sourceRowsRemoved(const QModelIndex &parent, int start, int end);

  void sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
  void sourceHeaderDataChanged(Qt::Orientation orientation, int first, int last);

  void sourceLayoutAboutToBeChanged();
  void sourceLayoutChanged();

  void sourceModelAboutToBeReset();
  void sourceModelReset();
};

#endif // __IDENTITY_PROXY_MODEL_HH__
