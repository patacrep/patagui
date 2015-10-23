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

#ifndef __LIBRARY_VIEW_HH__
#define __LIBRARY_VIEW_HH__

#include <QTableView>

class MainWindow;

/**
 * \file library-view.hh
 * \class CLibraryView
 * \brief CLibraryView is the class that displays the songs library.
 *
 * The songs library is displayed as a table where each row represents a song.
 * Columns can be displayed/hidden through the display section in the preferences.
 *
 * \image html library-view.png
 *
 */
class CLibraryView : public QTableView
{
    Q_OBJECT

public:
    /// Constructor
    CLibraryView(MainWindow *parent);
    /// Destructor
    ~CLibraryView();

    /// Load user settings
    void readSettings();
    /// Save user settings
    void writeSettings();

    /// Resize the library columns
    void resizeColumns();

public slots:
    /// Update the view, sorting songs by artist name, then titles
    void update();

private:
    void createActions();
    MainWindow * parent() const;
};

#endif // __LIBRARY_VIEW_HH__
