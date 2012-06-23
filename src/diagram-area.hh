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

#include "diagram.hh"

#include <QWidget>
#include <QString>
#include <QList>

class QGridLayout;
class QPushButton;

class CDiagramWidget;

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

  CDiagramWidget * addDiagram(const QString & chord, const CDiagram::ChordType & type);
  QList<CDiagramWidget*> diagrams() const;

  bool isReadOnly() const;
  void setReadOnly(bool value);

  int columnCount() const;
  void setColumnCount(int value);

public slots:
  void setTypeFilter(const CDiagram::ChordType & type);
  void setNameFilter(const QString & name);
  void setStringsFilter(const QString & strings);
  void setImportantFilter(bool onlyImportant);
  void clearFilters();
  void addSeparator(const QString & label);

protected:
  virtual void keyPressEvent(QKeyEvent *event);

private slots:
  void onDiagramChanged();
  void onDiagramClicked();
  CDiagramWidget * addDiagram();
  void removeDiagram();

signals:
  void contentsChanged();
  void diagramClicked(CDiagramWidget * diagram);

private:
  void updateSeparatorsVisibility();

  QGridLayout *m_layout;
  bool m_isReadOnly;
  int m_columnCount;
  int m_nbDiagrams;
  int m_nbSeparators;
  QPushButton *m_addDiagramButton;
};

#endif //__DIAGRAM_AREA_HH__
