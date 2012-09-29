// Copyright (C) 2009-2011, Romain Goffe <romain.goffe@gmail.com>
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
//******************************************************************************

#ifndef __TAB_WIDGET_HH__
#define __TAB_WIDGET_HH__

#include <QTabWidget>
#include <QTabBar>
#include <QString>

/**
 * \file tab-widget.hh
 * \class CTabWidget
 * \brief CTabWidget is a widget representing a tab in the mainwindow
 *
 * \image html tab-widget.png
 *
 * Note that a tab-widget is a generic widget that is a plain
 * container but from the application point of view, two kinds of tabs
 * should be distinguished :
 *
 * \li The library tab displays the list of
 * songs that may be used for a songbook. There is only one single
 * library tab.
 * \li A song-editor tab displays the contents of a song
 * and acts as a specialized text editor for sg files. There may be a
 * tab for each available song.
 *
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

  /// Constructor.
  CTabWidget(QWidget *parent=0);

  /// Destructor.
  virtual ~CTabWidget();

  /// Add a new tab.
  /// The tab's label is the title of the parent widget's window.
  /// @param widget the parent widget
  int addTab(QWidget *widget);

  /// Add a new tab with a label.
  /// @param widget the parent widget
  /// @param label the tab label
  int addTab(QWidget *widget, const QString &label);

  /// Getter on the selection behavior when the tab is added.
  /// This defines if the focus should be set on the tab
  /// when it is added.
  /// @return the selection behavior mode
  SelectionBehavior selectionBehaviorOnAdd() const;

  /// Setter on the selection behavior when the tab is added.
  /// This defines if the focus should be set on the tab
  /// when it is added.
  /// @param behavior the selection behavior mode
  void setSelectionBehaviorOnAdd(SelectionBehavior behavior);

public slots:
  /// Close the current tab.
  void closeTab();

  /// Close the tab at a given index.
  /// @param index a tab index
  void closeTab(int index);

  /// The current tab becomes the next tab on the right.
  void prev();

  /// The current tab becomes the previous tab on the left.
  void next();

  /// Change the tab label.
  /// @param text the new label
  void changeTabText(const QString &text);

protected:
  /// Hide or display the tab bar.
  /// The tab bar is hidden when there is only the library tab
  /// to be displayed (no song-editor tabs).
  void updateTabBarVisibility();

private:
  SelectionBehavior m_selectionBehaviorOnAdd; ///> focus policy on new tabs
};

class CTabBar : public QTabBar
{
  Q_OBJECT

public:
  /// Constructor.
  CTabBar(QWidget *parent=0);
  /// Destructor.
  ~CTabBar();

protected:
  virtual void mouseReleaseEvent(QMouseEvent *event);
};

#endif  // __TAB_WIDGET_HH__
