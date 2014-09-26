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

#include "song-sort-filter-proxy-model.hh"

#include "library.hh"
#include "songbook.hh"

#include <QDebug>

CSongSortFilterProxyModel::CSongSortFilterProxyModel(QObject *parent)
  : QSortFilterProxyModel(parent)
  , m_onlySelected(false)
  , m_onlyNotSelected(false)
  , m_filterString()
  , m_languageFilter()
  , m_negativeLanguageFilter()
  , m_keywordFilter()
{}

CSongSortFilterProxyModel::~CSongSortFilterProxyModel()
{}

void CSongSortFilterProxyModel::setFilterString(const QString &filterString)
{
  m_filterString = filterString;

  clearLanguageFilter();
  clearNegativeLanguageFilter();
  clearKeywordFilter();

  m_onlySelected = false;
  m_onlyNotSelected = false;

  QString filter = m_filterString;
  if (filter.contains("!:selection"))
    {
      m_onlyNotSelected = true;
      filter.remove("!:selection");
    }
  else if (filter.contains(":selection"))
    {
      m_onlySelected = true;
      filter.remove(":selection");
    }
  else if (!filter.contains(":se")) //:se(lection) would be removed
    {
      // parse the :keyword parameters and create the appropriate filter
      QRegExp langFilter("!?:(\\w{2})\\s?");
      int pos = 0;
      while ((pos = langFilter.indexIn(m_filterString, pos)) != -1)
	{
	  QString language = langFilter.cap(1);
	  QLocale locale(language);
	  if (langFilter.cap(0).startsWith("!"))
	    {
	      insertNegativeLanguageFilter(locale.language());
	    }
	  else
	    {
	      insertLanguageFilter(locale.language());
	    }
	  pos += langFilter.matchedLength();
	}

      filter.remove(langFilter);
    }
  m_keywordFilter << filter.split(" ");
  invalidateFilter();
}

QString CSongSortFilterProxyModel::filterString() const
{
  return m_filterString;
}

bool CSongSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
  QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

  bool accept = true;
  if (!m_keywordFilter.isEmpty())
    {
      foreach (QString keyword, m_keywordFilter)
        {
	  if (keyword.startsWith("!"))
            {
              keyword.remove("!");
              if (sourceModel()->data(index, CLibrary::TitleRole).toString().contains(keyword, Qt::CaseInsensitive)
                  || sourceModel()->data(index, CLibrary::ArtistRole).toString().contains(keyword, Qt::CaseInsensitive)
                  || sourceModel()->data(index, CLibrary::AlbumRole).toString().contains(keyword, Qt::CaseInsensitive))
                {
                  accept = false;
                }
            }
          else if (!sourceModel()->data(index, CLibrary::TitleRole).toString().contains(keyword, Qt::CaseInsensitive)
                   && !sourceModel()->data(index, CLibrary::ArtistRole).toString().contains(keyword, Qt::CaseInsensitive)
                   && !sourceModel()->data(index, CLibrary::AlbumRole).toString().contains(keyword, Qt::CaseInsensitive))
            {
              accept = false;
            }
        }
    }

  if (!m_negativeLanguageFilter.isEmpty())
    accept = accept && !m_negativeLanguageFilter.contains(sourceModel()->data(index, CLibrary::LanguageRole).value< QLocale::Language >());

  if (!m_languageFilter.isEmpty())
    accept = accept && m_languageFilter.contains(sourceModel()->data(index, CLibrary::LanguageRole).value< QLocale::Language >());

  if (m_onlySelected)
    accept = accept && qobject_cast< CSongbook* >(sourceModel())->isChecked(index);

  if (m_onlyNotSelected)
    accept = accept && !qobject_cast< CSongbook* >(sourceModel())->isChecked(index);

  return accept;
}

void CSongSortFilterProxyModel::checkAll()
{
  int rows = rowCount();
  CSongbook *songbook = qobject_cast< CSongbook* >(sourceModel());
  for (int i = 0; i < rows; ++i)
    {
      songbook->setChecked(mapToSource(index(i,0)), true);
    }
}

void CSongSortFilterProxyModel::uncheckAll()
{
  int rows = rowCount();
  CSongbook *songbook = qobject_cast< CSongbook* >(sourceModel());
  for (int i = 0; i < rows; ++i)
    {
      songbook->setChecked(mapToSource(index(i,0)), false);
    }
}

void CSongSortFilterProxyModel::toggleAll()
{
  int rows = rowCount();
  CSongbook *songbook = qobject_cast< CSongbook* >(sourceModel());
  for (int i = 0; i < rows; ++i)
    {
      songbook->toggle(mapToSource(index(i,0)));
    }
}

void CSongSortFilterProxyModel::insertLanguageFilter(const QLocale::Language &language)
{
  m_languageFilter.insert(language);
}

void CSongSortFilterProxyModel::removeLanguageFilter(const QLocale::Language &language)
{
  m_languageFilter.remove(language);
}

void CSongSortFilterProxyModel::clearLanguageFilter()
{
  m_languageFilter.clear();
}

const QSet< QLocale::Language > & CSongSortFilterProxyModel::languageFilter() const
{
  return m_languageFilter;
}

void CSongSortFilterProxyModel::insertNegativeLanguageFilter(const QLocale::Language &language)
{
  m_negativeLanguageFilter.insert(language);
}

void CSongSortFilterProxyModel::removeNegativeLanguageFilter(const QLocale::Language &language)
{
  m_negativeLanguageFilter.remove(language);
}

void CSongSortFilterProxyModel::clearNegativeLanguageFilter()
{
  m_negativeLanguageFilter.clear();
}

const QSet< QLocale::Language > & CSongSortFilterProxyModel::negativeLanguageFilter() const
{
  return m_negativeLanguageFilter;
}

void CSongSortFilterProxyModel::clearKeywordFilter()
{
  m_keywordFilter.clear();
}

const QStringList & CSongSortFilterProxyModel::keywordFilter() const
{
  return m_keywordFilter;
}
