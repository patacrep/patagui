// Copyright (C) 2009-2011, Romain Goffe <romain.goffe@gmail.com>
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

/**
 * \file library-view.hh
 *
 * Class for displaying the songlibrary.
 *
 */
#ifndef __LIBRARY_VIEW_HH__
#define __LIBRARY_VIEW_HH__

#include <QTableView>

class CMainWindow;

class CLibraryView : public QTableView
{
  Q_OBJECT

public:
  CLibraryView(CMainWindow *parent);
  ~CLibraryView();

  void readSettings();
  void writeSettings();

  void resizeColumns();

public slots:
  void update();

private:
  void createActions();
  CMainWindow * parent() const;
};

#endif // __LIBRARY_VIEW_HH__
