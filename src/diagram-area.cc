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

#include <QGridLayout>
#include <QBoxLayout>
#include <QPushButton>
#include <QList>
#include <QApplication>
#include <QKeyEvent>
#include <QLabel>


CDiagramArea::CDiagramArea(QWidget *parent)
  : QWidget(parent)
  , m_layout (new QGridLayout)
  , m_isReadOnly(false)
  , m_columnCount(0)
  , m_nbDiagrams(0)
  , m_nbSeparators(0)
{
  m_layout->setContentsMargins(4, 4, 4, 4);

  QBoxLayout *addButtonLayout = new QVBoxLayout;
  m_addDiagramButton = new QPushButton;
  m_addDiagramButton->setFlat(true);
  m_addDiagramButton->setToolTip(tr("Add a new diagram"));
  m_addDiagramButton->setIcon(QIcon::fromTheme("list-add", QIcon(":/icons/tango/48x48/actions/list-add.png")));
  connect(m_addDiagramButton, SIGNAL(clicked()), this, SLOT(addDiagram()));
  addButtonLayout->addStretch();
  addButtonLayout->addWidget(m_addDiagramButton);

  QBoxLayout *mainLayout = new QHBoxLayout;
  mainLayout->addLayout(m_layout);
  mainLayout->addLayout(addButtonLayout);
  mainLayout->addStretch();
  setLayout(mainLayout);

  setFocusPolicy(Qt::StrongFocus);
}

CDiagramWidget * CDiagramArea::addDiagram()
{
  CDiagramWidget *diagram = new CDiagramWidget("\\gtab{}{0:}", CDiagram::GuitarChord);
  diagram->setReadOnly(isReadOnly());
  connect(diagram, SIGNAL(diagramCloseRequested()), SLOT(removeDiagram()));
  connect(diagram, SIGNAL(changed()), SLOT(onDiagramChanged()));
  connect(diagram, SIGNAL(clicked()), SLOT(onDiagramClicked()));
  if (diagram->editChord())
    {
      ++m_nbDiagrams;
      m_layout->addWidget(diagram, m_nbDiagrams/columnCount() + m_nbSeparators, m_layout->columnCount()+1, 1, 1);
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
  ++m_nbDiagrams;
  m_layout->addWidget(diagram, (m_nbDiagrams-1)/columnCount() + m_nbSeparators, (m_nbDiagrams-1)%(columnCount()), 1, 1);
  connect(diagram, SIGNAL(diagramCloseRequested()), SLOT(removeDiagram()));
  connect(diagram, SIGNAL(changed()), SLOT(onDiagramChanged()));
  connect(diagram, SIGNAL(clicked()), SLOT(onDiagramClicked()));
  return diagram;
}

void CDiagramArea::removeDiagram()
{
  if (CDiagramWidget *diagram = qobject_cast< CDiagramWidget* >(QObject::sender()))
    {
      m_layout->removeWidget(diagram);
      --m_nbDiagrams;
      disconnect(diagram,0,0,0);
      diagram->setParent(0);
      diagram->deleteLater();
      onDiagramChanged();
    }
}

QList<CDiagramWidget*> CDiagramArea::diagrams() const
{
  QList<CDiagramWidget*>  list;
  for (int i=0; i < m_layout->count(); ++i)
    if (CDiagramWidget *diagram = qobject_cast< CDiagramWidget* >(m_layout->itemAt(i)->widget()))
      list << diagram;
  return list;
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
      for (int i=0; i < m_layout->count(); ++i)
	{
	  CDiagramWidget *temp = qobject_cast< CDiagramWidget* >(m_layout->itemAt(i)->widget());
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
  for (int i=0; i < m_layout->count(); ++i)
    if (CDiagramWidget *diagram = qobject_cast< CDiagramWidget* >(m_layout->itemAt(i)->widget()))
      diagram->setReadOnly(value);
}

int CDiagramArea::columnCount() const
{
  if (m_columnCount == 0)
    return 99;
  else
    return m_columnCount ;
}

void CDiagramArea::setColumnCount(int value)
{
  m_columnCount = value;
}

void CDiagramArea::setTypeFilter(const CDiagram::ChordType & type)
{
  foreach (CDiagramWidget *diagram, diagrams())
    if (diagram->type() != type)
      diagram->setVisible(false);

  updateSeparatorsVisibility();
}

void CDiagramArea::setNameFilter(const QString & name)
{
  if (name.isEmpty())
    clearFilters();

  foreach (CDiagramWidget *diagram, diagrams())
    if (!diagram->chord().contains(name))
      diagram->setVisible(false);

  updateSeparatorsVisibility();
}

void CDiagramArea::setImportantFilter(bool onlyImportantDiagrams)
{
  foreach (CDiagramWidget *diagram, diagrams())
    if (diagram->isImportant() != onlyImportantDiagrams)
      diagram->setVisible(false);
    else
      diagram->setVisible(true);

  updateSeparatorsVisibility();
}

void CDiagramArea::setStringsFilter(const QString & strings)
{
  if (strings.isEmpty())
    clearFilters();

  foreach (CDiagramWidget *diagram, diagrams())
    if (!diagram->strings().contains(strings))
      diagram->setVisible(false);

  updateSeparatorsVisibility();
}

void CDiagramArea::clearFilters()
{
  for (int i=0; i < m_layout->count(); ++i)
    m_layout->itemAt(i)->widget()->setVisible(true);
}

void CDiagramArea::addSeparator(const QString & str)
{
  QLabel *separator = new QLabel(QString("<b>%1</b>").arg(str));
  m_layout->addWidget(separator, m_layout->rowCount(), 0, 1, columnCount());
  ++m_nbSeparators;
}

void CDiagramArea::updateSeparatorsVisibility()
{
  for (int position=1; position < m_layout->count(); ++position)
    if (QLabel *currentSeparator = qobject_cast< QLabel* >(m_layout->itemAt(position)->widget()))
      {
	int prev = position -1; // look backward for the position of the previous visible item
	// we are either starting with a separator or the last item
	if (currentSeparator || position == m_layout->count()-1)
	  {
	    while (prev > 0 && !m_layout->itemAt(prev)->widget()->isVisible())
	      --prev;

	    // if previous visible element is a separator, it should be hidden
	    if (QLabel *prevSeparator = qobject_cast< QLabel* >(m_layout->itemAt(prev)->widget()))
	      prevSeparator->setVisible(false);
	  }
      }
}

