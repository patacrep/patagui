// Copyright (C) 2009-2011 Romain Goffe, Alexandre Dupas
//
// Songbook Creator is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// Songbook Creator is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
// MA  02110-1301, USA.
//******************************************************************************

/**
 * \file tab-widget.hh
 */

#ifndef __TAB_WIDGET_HH__
#define __TAB_WIDGET_HH__

#include <QTabWidget>
#include <QString>

/** \class CTabWidget "tab-widget.hh"
 * \brief CTabWidget is a class.
 */

class CTabWidget : public QTabWidget
{
  Q_OBJECT
  Q_ENUMS(SelectionBehavior)
  Q_PROPERTY(SelectionBehavior selectionBehaviorOnAdd
	     READ selectionBehaviorOnAdd
	     WRITE setSelectionBehaviorOnAdd)

public:
  enum SelectionBehavior { SelectCurrent, SelectNew };

  CTabWidget();
  virtual ~CTabWidget();

  int addTab(QWidget *widget, const QString &label);
  
  SelectionBehavior selectionBehaviorOnAdd() const;
  void setSelectionBehaviorOnAdd(SelectionBehavior behavior);

public slots:
  void closeTab(int index);
  void prev();
  void next();

protected:
  void updateTabBarVisibility();

private:
  SelectionBehavior m_selectionBehaviorOnAdd;
};

#endif  // __TAB_WIDGET_HH__
