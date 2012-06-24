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

#include <QBoxLayout>
#include <QPushButton>
#include <QList>
#include <QApplication>
#include <QKeyEvent>
#include <QLabel>
#include <QHeaderView>

#include <QDebug>

CDiagramArea::CDiagramArea(QWidget *parent)
  : QWidget(parent)
  , m_isReadOnly(false)
  , m_tableWidget(new CTableDiagram)
  , m_addDiagramButton(new QPushButton)
  , m_nbDiagrams(0)
{
  QBoxLayout *addButtonLayout = new QVBoxLayout;
    m_addDiagramButton->setFlat(true);
  m_addDiagramButton->setToolTip(tr("Add a new diagram"));
  m_addDiagramButton->setIcon(QIcon::fromTheme("list-add", QIcon(":/icons/tango/48x48/actions/list-add.png")));
  connect(m_addDiagramButton, SIGNAL(clicked()), this, SLOT(addDiagram()));
  addButtonLayout->addStretch();
  addButtonLayout->addWidget(m_addDiagramButton);

  QBoxLayout *mainLayout = new QHBoxLayout;
  mainLayout->addWidget(m_tableWidget, 1);
  mainLayout->addLayout(addButtonLayout);
  mainLayout->addStretch();
  setLayout(mainLayout);

  setFocusPolicy(Qt::StrongFocus);
}

CDiagramWidget * CDiagramArea::addDiagram()
{
  CDiagramWidget *diagram = new CDiagramWidget("\\gtab{}{0:}", CDiagram::GuitarChord);
  if (diagram->editChord())
    {
      diagram->setReadOnly(isReadOnly());
      m_tableWidget->addCellWidget(++m_nbDiagrams, diagram);
      connect(diagram, SIGNAL(diagramCloseRequested()), SLOT(removeDiagram()));
      connect(diagram, SIGNAL(changed()), SLOT(onDiagramChanged()));
      connect(diagram, SIGNAL(clicked()), SLOT(onDiagramClicked()));
      emit(contentsChanged());
    }
  else
    {
      delete diagram;
      diagram = 0;
    }
  return diagram;
}

CDiagramWidget * CDiagramArea::addDiagram(const QString & chord, const CDiagram::ChordType & type)
{
  CDiagramWidget *diagram = new CDiagramWidget(chord, type);
  diagram->setReadOnly(isReadOnly());
  m_tableWidget->addCellWidget(++m_nbDiagrams, diagram);
  connect(diagram, SIGNAL(diagramCloseRequested()), SLOT(removeDiagram()));
  connect(diagram, SIGNAL(changed()), SLOT(onDiagramChanged()));
  connect(diagram, SIGNAL(clicked()), SLOT(onDiagramClicked()));
  return diagram;
}

void CDiagramArea::removeDiagram()
{
  if (CDiagramWidget *diagram = qobject_cast< CDiagramWidget* >(QObject::sender()))
    {
      //m_tableWidget->removeCellWidget(diagram);
      --m_nbDiagrams;
      disconnect(diagram,0,0,0);
      diagram->setParent(0);
      diagram->deleteLater();
      onDiagramChanged();
    }
}

QList<CDiagramWidget*> CDiagramArea::diagrams() const
{
  return m_tableWidget->diagrams();
}

void CDiagramArea::onDiagramChanged()
{
  emit(contentsChanged());
}

void CDiagramArea::onDiagramClicked()
{
  CDiagramWidget *diagram = qobject_cast< CDiagramWidget* >(QObject::sender());
  diagram->setSelected(!diagram->isSelected());

  // multiple selection via Ctrl is allowed in non read only mode
  if (isReadOnly() || QApplication::keyboardModifiers() != Qt::ControlModifier)
    {
      for (int j=0; j < m_tableWidget->rowCount(); ++j)
      for (int i=0; i < m_tableWidget->columnCount(); ++i)
	{
	  CDiagramWidget *temp = qobject_cast< CDiagramWidget* >(m_tableWidget->cellWidget(i, j));
	  if (temp && temp != diagram)
	    temp->setSelected(false);
	}
    }

  emit(diagramClicked(diagram));
}

void CDiagramArea::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Delete)
    {
      bool changed = false;
      foreach (CDiagramWidget *diagram, diagrams())
	if (diagram->isSelected())
	  {
	    changed = true;
	    diagram->deleteLater();
	  }

      if (changed)
	onDiagramChanged();
    }
}

bool CDiagramArea::isReadOnly() const
{
  return m_isReadOnly;
}

void CDiagramArea::setReadOnly(bool value)
{
  m_isReadOnly = value;
  m_addDiagramButton->setVisible(!value);
  foreach (CDiagramWidget *diagram, diagrams())
    diagram->setReadOnly(value);
}

void CDiagramArea::setTypeFilter(const CDiagram::ChordType & type)
{
  m_tableWidget->setTypeFilter(type);
}

void CDiagramArea::setNameFilter(const QString & name)
{
  m_tableWidget->setNameFilter(name);
}

void CDiagramArea::setImportantFilter(bool onlyImportantDiagrams)
{
  m_tableWidget->setImportantFilter(onlyImportantDiagrams);
}

void CDiagramArea::setStringsFilter(const QString & strings)
{
  m_tableWidget->setStringsFilter(strings);
}

void CDiagramArea::clearFilters()
{
  m_tableWidget->clearFilters();
}

void CDiagramArea::addSeparator(const QString & str)
{
  m_tableWidget->addSeparator(str);
}

void CDiagramArea::setColumnCount(int value)
{
  m_tableWidget->setColumnCount(value);
}

void CDiagramArea::setRowCount(int value)
{
  m_tableWidget->setRowCount(value);
}

int CDiagramArea::nbSeparators() const
{
  return m_tableWidget->nbSeparators();
}

//-------------------------------------------------------------------------

CTableDiagram::CTableDiagram(QWidget *parent)
  : QTableWidget(parent)
  , m_nbSeparators(0)
{
  setContentsMargins(4, 4, 4, 4);
  setRowCount(1);
  setColumnCount(1);
  m_fixedColumnCount = false;
  m_fixedRowCount = false;

  verticalHeader()->hide();
  horizontalHeader()->hide();
  setStyleSheet("QTableView::item { border: 0px; padding: 4px;}");
  setShowGrid(true);
}

void CTableDiagram::setColumnCount(int value)
{
  m_fixedColumnCount = true;
  QTableWidget::setColumnCount(value);
}

void CTableDiagram::setRowCount(int value)
{
  m_fixedRowCount = true;
  QTableWidget::setRowCount(value);
}

void CTableDiagram::addCellWidget(int index, QWidget *widget)
{
  int row = 1, col = 1;
  if (m_fixedColumnCount)
    {
      row = (index-1)/columnCount() + nbSeparators();
      col = (index-1)%columnCount();
    }
  if (m_fixedRowCount)
    {
      row = (index-1)%rowCount();
      col = (index-1)/rowCount() + nbSeparators();
    }

  // dynamically expand the QTableWidget
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

  setCellWidget(row, col, widget);
  resizeRowsToContents();
  resizeColumnsToContents();
}

QList<CDiagramWidget*> CTableDiagram::diagrams() const
{
  QList<CDiagramWidget*> list;
  for (int i=0; i < rowCount(); ++i)
    for (int j=0; j < columnCount(); ++j)
      if (CDiagramWidget *diagram = qobject_cast< CDiagramWidget* >(cellWidget(i, j)))
	list << diagram;

  return list;
}

void CTableDiagram::setTypeFilter(const CDiagram::ChordType & type)
{
  for (int i=0; i < rowCount(); ++i)
    for (int j=0; j < columnCount(); ++j)
      if (CDiagramWidget *diagram = qobject_cast< CDiagramWidget* >(cellWidget(i, j)))
	if (diagram->type() != type)
	  {
	    hideRow(i);
	    break;
	  }

  updateSeparatorsVisibility();
}

void CTableDiagram::setNameFilter(const QString & name)
{
  if (name.isEmpty())
    clearFilters();

  foreach (CDiagramWidget *diagram, diagrams())
    if (!diagram->chord().contains(name))
      {
	diagram->setVisible(false);
	m_hiddenItems << diagram;
      }

  updateSeparatorsVisibility();
}

void CTableDiagram::setImportantFilter(bool onlyImportantDiagrams)
{
  for (int j=0; j < columnCount(); ++j)
    for (int i=0; i < rowCount(); ++i)
      if (CDiagramWidget *diagram = qobject_cast< CDiagramWidget* >(cellWidget(i, j)))
	{
	  if (diagram->isImportant() != onlyImportantDiagrams)
	    hideColumn(j);
	  else
	    showColumn(j);
	  break;
	}

  updateSeparatorsVisibility();
}

void CTableDiagram::setStringsFilter(const QString & strings)
{
  if (strings.isEmpty())
    clearFilters();

  foreach (CDiagramWidget *diagram, diagrams())
    if (!diagram->strings().contains(strings))
      {
	diagram->setVisible(false);
	m_hiddenItems << diagram;
      }

  updateSeparatorsVisibility();
}

void CTableDiagram::clearFilters()
{
  m_hiddenItems.clear();

  foreach (CDiagramWidget *diagram, diagrams())
    diagram->setVisible(true);

  for (int i=0; i < rowCount(); ++i)
    showRow(i);
  for (int j=0; j < columnCount(); ++j)
    showColumn(j);
}

void CTableDiagram::addSeparator(const QString & str)
{
  QTableWidgetItem *separator = new QTableWidgetItem(str);
  setRowCount(rowCount()+1);
  m_fixedRowCount=false;
  if (rowCount() == 2)
    setItem(0, 0, separator);
  else
    setItem(rowCount()-1, 0, separator);
  ++m_nbSeparators;
}

int CTableDiagram::nbSeparators() const
{
  return m_nbSeparators;
}

void CTableDiagram::updateSeparatorsVisibility()
{
//  for (int position = rowCount() - 1; position > 0; )
//    {
//      QTableWidgetItem *current = item(position, 0);
//      // look backward for the position of the previous visible item
//      int prev = position -1;
//      if (current && !current->text().isEmpty())
//	{
//	  qDebug() << "current separator : " << current->text() << " on line " << position;
//	  QTableWidgetItem *prevItem = item(prev, 0);
//	  while (prev > 0 && prevItem && !prevItem->text().isEmpty() || )
//	    {
//	      qDebug() << "hiding row " << prev << " containing : " << prevItem->text();
//	      hideRow(prev);
//	      --prev;
//	    }
//	}
//      position = prev;
//    }
}


void CTableDiagram::paintEvent(QPaintEvent *event)
{
  QTableWidget::paintEvent(event);
  // workaround for QTBUG-22490
  // https://bugreports.qt-project.org/browse/QTBUG-22490
  // once fixed, we can remove the m_hiddenItems list
  foreach (QWidget *w, m_hiddenItems)
    w->setVisible(false);
}
