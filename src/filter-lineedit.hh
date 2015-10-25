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

/**
 * \file filter-lineedit.hh
 *
 * Filter lineedit with embedded clear button.
 *
 */
#ifndef __FILTER_LINEEDIT_HH__
#define __FILTER_LINEEDIT_HH__

#include <QToolButton>
#include "utils/lineedit.hh"

/*!
  \file filter-lineedit.hh
  \class ClearButton
  \brief ClearButton is the clear button that resets the content of a
  FilterLineEdit
  \image html mag-button.png
*/
class ClearButton : public QToolButton
{
    Q_OBJECT

    public:
    /// Constructor.
    ClearButton(QWidget *parent = 0);

    protected slots:
    void textChanged(const QString &text);

    protected:
    void paintEvent(QPaintEvent *event);

    private:
    QImage m_icon;
};

/*!
  \file filter-lineedit.hh
  \class MagButton
  \brief MagButton is the magnify button in a FilterLineEdit widget
  \image html mag-button.png
*/
class MagButton : public QToolButton
{
    Q_OBJECT

    public:
    /// Constructor.
    MagButton(QWidget *parent = 0);

    protected:
    void paintEvent(QPaintEvent *event);

    private:
    QImage m_icon;
};

/*!
  \file filter-lineedit.hh
  \class LocaleButton
  \brief LocaleButton is the "flag" button in a FilterLineEdit widget
  that triggers a popup to filter songs by language
*/
class LocaleButton : public QToolButton
{
    Q_OBJECT

    public:
    /// Constructor.
    LocaleButton(QWidget *parent = 0);
};

class QAction;
class SongSortFilterProxyModel;

/*!
  \file filter-lineedit.hh
  \class FilterLineEdit
  \brief FilterLineEdit is a QLineEdit widget that allows one to filter results
  in the songs library

  The filter updates the songs library view so that it only displays songs that
  match
  the user input.

  The filter only applies on artist/title/album columns as provided
  by the CLibrary::completionModel().

  A FilterLineEdit widget contains a MagButton on the left that allows one to
  access quick filters functions (such as search by songs' language) and a
  ClearButton
  on the right that resets its content (only dispayed when there is some user
  input).

  \image html filter.png

*/
class FilterLineEdit : public LineEdit
{
    Q_OBJECT

    public slots:
    /// Only display songs written in English.
    void filterLanguageEnglish();
    /// Only display songs written in French.
    void filterLanguageFrench();
    /// Only display songs written in Spanish.
    void filterLanguageSpanish();
    /// Only display songs written in Portuguese.
    void filterLanguagePortuguese();
    /// Only display songs written in Italian.
    void filterLanguageItalian();

    public:
    /// Constructor.
    FilterLineEdit(QWidget *parent = 0);
    /// Destructor.
    ~FilterLineEdit();

    /// Add an action to the menu that is displayed
    /// when clicking on the MagButton on the left.
    void addAction(QAction *action);

    /// Defines the model that should be filtered.
    /// @param filterModel the proxy model of the songs library
    void setFilterModel(SongSortFilterProxyModel *filterModel);

    private:
    QMenu *m_menu;

    SongSortFilterProxyModel *m_filterModel;
};

#endif // __FILTER_LINEEDIT_HH__
