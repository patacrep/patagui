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

#include "diagram-area.hh"

#include "diagram-editor.hh"

#include <QBoxLayout>
#include <QPushButton>
#include <QList>
#include <QApplication>
#include <QKeyEvent>
#include <QLabel>
#include <QHeaderView>
#include <QTableView>
#include <QSortFilterProxyModel>
#include <QMenu>
#include <QAction>

#include <QDebug>

CDiagramArea::CDiagramArea(QWidget *parent)
  : QWidget(parent)
  , m_isReadOnly(false)
  , m_addDiagramButton(new QPushButton)
  , m_nbDiagrams(0)
{
  QBoxLayout *addButtonLayout = new QVBoxLayout;
  m_addDiagramButton->setFlat(true);
  m_addDiagramButton->setToolTip(tr("Add a new diagram"));
  m_addDiagramButton->setIcon(QIcon::fromTheme("list-add", QIcon(":/icons/tango/48x48/actions/list-add.png")));
  connect(m_addDiagramButton, SIGNAL(clicked()), this, SLOT(newDiagram()));
  addButtonLayout->addStretch();
  addButtonLayout->addWidget(m_addDiagramButton);

  // diagram model
  m_diagramModel = new CTableDiagram;

  // proxy model (filtering)
  m_proxyModel = new QSortFilterProxyModel;
  m_proxyModel->setSourceModel(m_diagramModel);
  m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
  m_proxyModel->setFilterKeyColumn(-1);

  // diagram view
  m_diagramView = new QTableView;
  m_diagramView->setModel(m_proxyModel);
  m_diagramView->verticalHeader()->hide();
  m_diagramView->horizontalHeader()->hide();
  m_diagramView->setStyleSheet("QTableView::item { border: 0px; padding: 10px;}");
  m_diagramView->setShowGrid(false);
  m_diagramView->setContextMenuPolicy(Qt::CustomContextMenu);

  connect(m_diagramView, SIGNAL(clicked(const QModelIndex &)),
	  this, SLOT(onViewClicked(const QModelIndex &)));
  connect(m_diagramModel, SIGNAL(layoutChanged()),
	  this, SLOT(resizeRows()));

  connect(this, SIGNAL(layoutChanged()), this, SLOT(resizeRows()));
  connect(this, SIGNAL(readOnlyModeChanged()), this, SLOT(update()));

  QBoxLayout *mainLayout = new QHBoxLayout;
  mainLayout->addWidget(m_diagramView, 1);
  mainLayout->addLayout(addButtonLayout);
  mainLayout->addStretch();
  setLayout(mainLayout);

  update();
  setFocusPolicy(Qt::StrongFocus);
}

void CDiagramArea::update()
{
  if (isReadOnly())
    {
      m_addDiagramButton->setVisible(false);
      m_diagramView->setSelectionMode(QAbstractItemView::SingleSelection);
      m_diagramView->setDragEnabled(false);
      m_diagramView->setAcceptDrops(false);
      m_diagramView->setDropIndicatorShown(false);
      m_diagramView->setDragDropMode(QAbstractItemView::NoDragDrop);

      disconnect(m_diagramView, SIGNAL(doubleClicked(const QModelIndex &)), 0, 0);
      disconnect(m_diagramView, SIGNAL(customContextMenuRequested(const QPoint &)), 0, 0);
    }
  else
    {
      m_addDiagramButton->setVisible(true);
      m_diagramView->setSelectionMode(QAbstractItemView::ExtendedSelection);
      m_diagramView->setDragEnabled(true);
      m_diagramView->setAcceptDrops(true);
      m_diagramView->setDropIndicatorShown(true);
      m_diagramView->setDragDropMode(QAbstractItemView::InternalMove);

      connect(m_diagramView, SIGNAL(doubleClicked(const QModelIndex &)),
	      this, SLOT(editDiagram(const QModelIndex &)));
      connect(m_diagramView, SIGNAL(customContextMenuRequested(const QPoint &)),
	      this, SLOT(contextMenu(const QPoint &)));
    }
}

void CDiagramArea::onViewClicked(const QModelIndex & index)
{
  if (index.isValid())
    emit(diagramClicked(m_diagramModel->getDiagram(m_proxyModel->mapToSource(index))));
}

void CDiagramArea::resizeRows()
{
  for (int i=0; i < m_diagramModel->rowCount(); ++i)
    m_diagramView->setRowHeight(i, 120);
}

void CDiagramArea::newDiagram()
{
  editDiagram(QModelIndex());
}

void CDiagramArea::editDiagram(QModelIndex index)
{
  Q_ASSERT(!isReadOnly());

  if (!index.isValid())
    index = m_diagramView->indexAt(m_diagramView->mapFromGlobal(QCursor::pos()));

  bool newChord = !index.isValid();

  CChord *chord = newChord ?
    new CChord : m_diagramModel->getDiagram(m_proxyModel->mapToSource(index));

  CDiagramEditor dialog(this);
  dialog.setChord(chord);

  if (dialog.exec() == QDialog::Accepted)
    {
      if (newChord)
	addDiagram(dialog.chord()->toString());
      else
	m_diagramModel->setData(m_proxyModel->mapToSource(index), dialog.chord()->toString());

      emit(contentsChanged());
    }
}

void CDiagramArea::addDiagram(const QString & chord)
{
  m_diagramModel->addItem(chord);
}

void CDiagramArea::removeDiagram(QModelIndex index)
{
  if (!index.isValid())
    index = m_diagramView->indexAt(m_diagramView->mapFromGlobal(QCursor::pos()));

  if (!index.isValid() || isReadOnly())
    return;

  m_diagramModel->removeItem(m_proxyModel->mapToSource(index));
}

void CDiagramArea::onDiagramChanged()
{
  Q_ASSERT(isReadOnly());
  emit(contentsChanged());
}

bool CDiagramArea::isReadOnly() const
{
  return m_isReadOnly;
}

void CDiagramArea::setReadOnly(bool value)
{
  if (m_isReadOnly != value)
    {
      m_isReadOnly = value;
      emit(readOnlyModeChanged());
    }
}

void CDiagramArea::contextMenu(const QPoint & pos)
{
  Q_UNUSED(pos);

  QMenu menu;

  QAction *action = new QAction(tr("Edit"), this);
  connect(action, SIGNAL(triggered()), this, SLOT(editDiagram()));
  menu.addAction(action);

  action = new QAction(tr("Remove"), this);
  connect(action, SIGNAL(triggered()), this, SLOT(removeDiagram()));
  menu.addAction(action);

  menu.exec(QCursor::pos());
}

void CDiagramArea::setTypeFilter(const CChord::Instrument & type)
{
  m_proxyModel->setFilterRole(Qt::DisplayRole);
  if (type == CChord::Guitar)
    m_proxyModel->setFilterRegExp("gtab");
  else
    m_proxyModel->setFilterRegExp("utab");
  emit(layoutChanged());
}

void CDiagramArea::setNameFilter(const QString & name)
{
  clearFilters();
  m_proxyModel->setFilterRole(CTableDiagram::NameRole);
  m_proxyModel->setFilterRegExp(name);
  emit(layoutChanged());
}

void CDiagramArea::setStringsFilter(const QString & strings)
{
  m_proxyModel->setFilterRole(CTableDiagram::StringsRole);
  m_proxyModel->setFilterRegExp(strings);
  emit(layoutChanged());
}

void CDiagramArea::clearFilters()
{
  m_proxyModel->setFilterRole(Qt::DisplayRole);
  m_proxyModel->setFilterRegExp("");
  emit(layoutChanged());
}

void CDiagramArea::setColumnCount(int value)
{
  m_diagramModel->setColumnCount(value);
}

void CDiagramArea::setRowCount(int value)
{
  m_diagramModel->setRowCount(value);
}

QList< CChord* > CDiagramArea::diagrams()
{
  QList< CChord* > list;
  for (int i = 0; i < m_diagramModel->rowCount(); ++i)
    for (int j = 0; j < m_diagramModel->columnCount(); ++j)
      {
	list << m_diagramModel->getDiagram(m_diagramModel->index(i, j));
      }

  return list;
}

//-------------------------------------------------------------------------

CTableDiagram::CTableDiagram(QWidget *parent)
  : QAbstractTableModel(parent)
  , m_columnCount(0)
  , m_rowCount(0)
{
  m_fixedColumnCount = false;
  m_fixedRowCount = false;
}

CTableDiagram::~CTableDiagram()
{
  foreach (CChord* diagram, m_data)
    delete diagram;
  m_data.clear();
}

int CTableDiagram::columnCount(const QModelIndex &) const
{
  return m_columnCount;
}

void CTableDiagram::setColumnCount(int value)
{
  emit(layoutAboutToBeChanged());
  m_fixedColumnCount = true;
  m_columnCount = value;
  emit(layoutChanged());
}

int CTableDiagram::rowCount(const QModelIndex &) const
{
  return m_rowCount;
}

void CTableDiagram::setRowCount(int value)
{
  emit(layoutAboutToBeChanged());
  m_fixedRowCount = true;
  m_rowCount = value;
  emit(layoutChanged());
}

QVariant CTableDiagram::data(const QModelIndex &index, int role) const
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

bool CTableDiagram::setData(const QModelIndex & index, const QVariant & value, int role)
{
  Q_UNUSED(role);

  if (!index.isValid())
    return false;

  CChord *diagram = new CChord(value.toString());
  if (diagram->isValid())
    {
      int pos = positionFromIndex(index);
      delete m_data[pos];
      m_data[pos] = diagram;
      return true;
    }

  delete diagram;
  return false;
}

void CTableDiagram::insertItem(const QModelIndex & index, const QString & value)
{
  setColumnCount(columnCount() + 1);
  m_fixedColumnCount = false;

  CChord *diagram = new CChord(value);
  if (diagram->isValid())
    m_data.insert(index.column(), diagram);
  else
    delete diagram;
}

void CTableDiagram::removeItem(const QModelIndex & index)
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

void CTableDiagram::addItem(const QString & value)
{
  CChord * diagram = new CChord(value);
  if (!diagram->isValid())
    {
      delete diagram;
      return;
    }

  m_data.append(diagram);

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

QModelIndex CTableDiagram::indexFromPosition(int position)
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

int CTableDiagram::positionFromIndex(const QModelIndex & index) const
{
  return columnCount() * index.row() + index.column();
}

CChord * CTableDiagram::getDiagram(const QModelIndex & index) const
{
  return m_data[positionFromIndex(index)];
}

Qt::DropActions CTableDiagram::supportedDropActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}

Qt::DropActions CTableDiagram::supportedDragActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}

Qt::ItemFlags CTableDiagram::flags(const QModelIndex &index) const
{
  Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index);

  if (index.isValid())
    return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
  else
    return Qt::ItemIsDropEnabled | defaultFlags;
}

QStringList CTableDiagram::mimeTypes() const
{
  QStringList types;
  types << "text/plain";
  return types;
}

QMimeData * CTableDiagram::mimeData(const QModelIndexList &indexes) const
{
  QMimeData *mimeData = new QMimeData();
  foreach (const QModelIndex &index, indexes)
    if (index.isValid())
      mimeData->setText(data(index, Qt::DisplayRole).toString());

  return mimeData;
}

bool CTableDiagram::dropMimeData(const QMimeData *data, Qt::DropAction action,
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
