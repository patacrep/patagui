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

#include <QUrl>
#include <QDeclarativeView>
#include <QDeclarativeEngine>
#include <QDeclarativeComponent>
#include <QDeclarativeContext>
#include <QDeclarativeItem>

#include <QDebug>

CDiagramArea::CDiagramArea(QWidget *parent)
  : QWidget(parent)
  , m_isReadOnly(false)
  , m_addDiagramButton(new QPushButton)
  , m_chordId(0)
{
  QBoxLayout *addButtonLayout = new QVBoxLayout;
  m_addDiagramButton->setFlat(true);
  m_addDiagramButton->setToolTip(tr("Add a new diagram"));
  m_addDiagramButton->setIcon(QIcon::fromTheme("list-add", QIcon(":/icons/tango/48x48/actions/list-add.png")));
  connect(m_addDiagramButton, SIGNAL(clicked()), this, SLOT(newDiagram()));
  addButtonLayout->addStretch();
  addButtonLayout->addWidget(m_addDiagramButton);

  m_diagramView = new QDeclarativeView;
  m_diagramView->setResizeMode(QDeclarativeView::SizeRootObjectToView);

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
  m_addDiagramButton->setVisible(!isReadOnly());
}

void CDiagramArea::newDiagram()
{
  editDiagram(QModelIndex());
}

void CDiagramArea::editDiagram(QModelIndex index)
{
  Q_ASSERT(!isReadOnly());

  bool newChord = !index.isValid();

  CDiagramEditor dialog(this);
  if (dialog.exec() == QDialog::Accepted)
    {
      if (newChord)
	qWarning() << "Todo: Add diagram";
      else
	qWarning() << "Todo: Update diagram";

      emit(contentsChanged());
    }
}

void CDiagramArea::addDiagram(const QString & chord)
{
  m_diagramModel.append(new CChord(chord, m_chordId++));
  QDeclarativeContext *ctxt = m_diagramView->rootContext();
  ctxt->setContextProperty("chordModel", QVariant::fromValue(m_diagramModel));
  m_diagramView->setSource(QUrl::fromLocalFile("qml/Grid.qml"));
}

void CDiagramArea::removeDiagram(QModelIndex index)
{
  if (!index.isValid() || isReadOnly())
    return;
  qWarning() << "Todo: Remove diagram";
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

QList<QObject*> CDiagramArea::chords() const
{
  return m_diagramModel;
}
