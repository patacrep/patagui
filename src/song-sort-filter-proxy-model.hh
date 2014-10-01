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
#ifndef __SONG_SORT_FILTER_PROXY_MODEL_HH__
#define __SONG_SORT_FILTER_PROXY_MODEL_HH__

#include <QSortFilterProxyModel>
#include <QString>
#include <QSet>
#include <QLocale>
#include <QStringList>

/*!
  \file song-sort-filter-proxy-model.hh
  \class CSongSortFilterProxyModel
  \brief CSongSortFilterProxyModel implements custom sort and filter
  operations for CLibraryView.

  Allows one to filter the library. Song items are only displayed if
  the match the filter from their artist, title, or album fields.
*/
class CSongSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public slots:
    /*!
    Selects all filtered songs.
    \sa uncheckAll, toggleAll
  */
    void checkAll();

    /*!
    Unselects all filtered songs.
    \sa checkAll, toggleAll
  */
    void uncheckAll();

    /*!
    Invert selection for all filtered songs.
    \sa checkAll, uncheckAll
  */
    void toggleAll();

    /*!
    Filter the view according to \a filterString.
    A filter string may contain keywords starting with :
    or negative filters starting with !: (ie :fr or !:en)
  */
    void setFilterString(const QString &filterString);

    /*!
    Add a \a language filter. For instance, ":en" hides songs
    that are not written in english.
    \sa insertNegativeLanguageFilter, removeLanguageFilter, clearLanguageFilter
  */
    void insertLanguageFilter(const QLocale::Language &language);

    /*!
    Removes a \a language filter.
    \sa removeNegativeLanguageFilter, insertLanguageFilter, clearLanguageFilter
  */
    void removeLanguageFilter(const QLocale::Language &language);

    /*!
    Removes all language filters.
    \sa clearNegativeLanguageFilter, insertLanguageFilter, clearLanguageFilter
  */
    void clearLanguageFilter();

    /*!
    Add a negative \a language filter. For instance, "!:en" hides songs
    that are written in english.
    \sa insertLanguageFilter, removeNegativeLanguageFilter, clearNegativeLanguageFilter
  */
    void insertNegativeLanguageFilter(const QLocale::Language &language);

    /*!
    Removes a negative \a language filter.
    \sa removesLanguageFilter, insertNegativeLanguageFilter, clearNegativeLanguageFilter
  */
    void removeNegativeLanguageFilter(const QLocale::Language &language);

    /*!
    Removes all negative language filters.
    \sa clearLanguageFilter, insertNegativeLanguageFilter, removeNegativeLanguageFilter
  */
    void clearNegativeLanguageFilter();

    /*!
    Clears the filter, displaying all items in the library view.
   */
    void clearKeywordFilter();

public:
    /// Constructor.
    CSongSortFilterProxyModel(QObject *parent = 0);

    /// Destructor.
    ~CSongSortFilterProxyModel();

    /*!
    Returns the filter.
    \sa setFilterString
  */
    QString filterString() const;

    /*!
    Returns language filters.
    \sa insertLanguageFilter, removeLanguageFilter, clearLanguageFilter
  */
    const QSet< QLocale::Language > & languageFilter() const;

    /*!
    Returns negative language filters.
    \sa insertNegativeLanguageFilter, removeNegativeLanguageFilter, clearNegativeLanguageFilter
  */
    const QSet< QLocale::Language > & negativeLanguageFilter() const;

    /*!
    Returns the keywordFilter.
    \sa setKeywordFilter
  */
    const QStringList & keywordFilter() const;

protected:
    /*!
    Reimplements QSortFilterProxyModel::filterAcceptsRow
    to display rows matching filterString only for title, artist and album columns.
  */
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

private:
    bool m_onlySelected;
    bool m_onlyNotSelected;
    QString m_filterString;
    QSet< QLocale::Language > m_languageFilter;
    QSet< QLocale::Language > m_negativeLanguageFilter;
    QStringList m_keywordFilter;
};

#endif // __SONG_SORT_FILTER_PROXY_MODEL_HH__
