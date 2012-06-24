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
#include <QTableWidget>
#include <QString>
#include <QList>

#include "diagram.hh"


class CDiagramWidget;

class CTableDiagram : public QTableWidget
{
  Q_OBJECT

public:
  CTableDiagram(QWidget *parent=0);

  int nbSeparators() const;

  QList<CDiagramWidget*> diagrams() const;

  virtual void setColumnCount(int value);
  virtual void setRowCount(int value);

public slots:
  void setTypeFilter(const CDiagram::ChordType & type);
  void setNameFilter(const QString & name);
  void setStringsFilter(const QString & strings);
  void setImportantFilter(bool onlyImportant);
  void clearFilters();
  void addSeparator(const QString & label);

  void addCellWidget(int index, QWidget *widget);

protected:
  virtual void paintEvent(QPaintEvent *event);

private:
  void updateSeparatorsVisibility();

private:
  int m_fixedColumnCount;
  int m_fixedRowCount;
  int m_nbSeparators;
  QList<QWidget*> m_hiddenItems;
};


class QPushButton;

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

  int rowCount() const;
  void setRowCount(int value);

  int nbSeparators() const;

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
  bool m_isReadOnly;
  CTableDiagram *m_tableWidget;
  QPushButton *m_addDiagramButton;
  int m_nbDiagrams;
};


#endif //__DIAGRAM_AREA_HH__
