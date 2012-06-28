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
#ifndef __DIAGRAM_AREA_HH__
#define __DIAGRAM_AREA_HH__


#include <QWidget>
#include <QAbstractTableModel>
#include <QModelIndex>
#include <QString>
#include <QList>
#include <QPoint>
#include <QVector>
#include <QMap>

#include "diagram.hh"


class CTableDiagram : public QAbstractTableModel
{
  Q_OBJECT

  public:
  enum DiagramRoles {
    ChordRole = Qt::UserRole + 1,
    StringsRole = Qt::UserRole + 2,
    TypeRole = Qt::UserRole + 3,
    ImportantRole = Qt::UserRole + 4,
    MaxRole = ImportantRole
  };

  CTableDiagram(QWidget *parent=0);
  ~CTableDiagram();

  virtual int columnCount(const QModelIndex & index = QModelIndex()) const;
  virtual void setColumnCount(int value);

  virtual int rowCount(const QModelIndex & index = QModelIndex()) const;
  virtual void setRowCount(int value);

  /* drag and drop */
  Qt::DropActions supportedDropActions() const;
  Qt::DropActions supportedDragActions() const;
  Qt::ItemFlags flags(const QModelIndex &index) const;
  QStringList mimeTypes() const;
  QMimeData * mimeData(const QModelIndexList &indexes) const;
  bool dropMimeData(const QMimeData *data, Qt::DropAction action,
		    int row, int column, const QModelIndex &parent);

  QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
  bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

  CDiagram * getDiagram(const QModelIndex &) const;

public slots:
  void insertItem(const QModelIndex & index, const QString & value);
  void removeItem(const QModelIndex & index);
  void addItem(const QString & value);

private:
  QModelIndex indexFromPosition(int position);
  int positionFromIndex(const QModelIndex & index) const;

private:
  bool m_fixedColumnCount;
  bool m_fixedRowCount;
  int m_columnCount;
  int m_rowCount;
  QVector<CDiagram*> m_data;
};


class QPushButton;
class QTableView;
class QSortFilterProxyModel;
/**
 * \file diagram-area.hh
 * \class CDiagramArea
 * \brief CDiagramArea is a widget displaying a list of chords on a grid
 *
 */
class CDiagramArea : public QWidget
{
  Q_OBJECT

  public:
  CDiagramArea(QWidget *parent=0);

  bool isReadOnly() const;
  void setReadOnly(bool value);

  void setColumnCount(int value);
  void setRowCount(int value);

  QList< CDiagram* > diagrams();

public slots:
  void newDiagram();
  void addDiagram(const QString & chord);
  void editDiagram(QModelIndex index = QModelIndex());
  void removeDiagram(QModelIndex index = QModelIndex());
  void setTypeFilter(const CDiagram::ChordType & type);
  void setNameFilter(const QString & name);
  void setStringsFilter(const QString & strings);
  void clearFilters();

private slots:
  void update();
  void resizeRows();
  void onDiagramChanged();
  void contextMenu(const QPoint & pos);
  void onViewClicked(const QModelIndex &);

signals:
  void contentsChanged();
  void layoutChanged();
  void readOnlyModeChanged();
  void diagramClicked(CDiagram * diagram);

private:
  bool m_isReadOnly;
  CTableDiagram *m_diagramModel;
  QSortFilterProxyModel *m_proxyModel;
  QTableView *m_diagramView;
  QPushButton *m_addDiagramButton;
  int m_nbDiagrams;
};


#endif //__DIAGRAM_AREA_HH__
