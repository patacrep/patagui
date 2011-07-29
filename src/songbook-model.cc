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
#include "songbook-model.hh"

#include "library.hh"

#include <QDebug>

CSongbookModel::CSongbookModel(QObject *parent)
  : CIdentityProxyModel(parent)
  , m_selectedSongs()
{}

CSongbookModel::~CSongbookModel()
{
  m_selectedSongs.clear();
}

void CSongbookModel::selectAll()
{
  for (int i = 0; i < m_selectedSongs.size(); ++i)
    {
      m_selectedSongs[i] = true;
    }
  emit(dataChanged(index(0,0),index(m_selectedSongs.size()-1,0)));
}

void CSongbookModel::unselectAll()
{
  for (int i = 0; i < m_selectedSongs.size(); ++i)
    {
      m_selectedSongs[i] = false;
    }
  emit(dataChanged(index(0,0),index(m_selectedSongs.size()-1,0)));
}

void CSongbookModel::invertSelection()
{
  for (int i = 0; i < m_selectedSongs.size(); ++i)
    {
      m_selectedSongs[i] = !m_selectedSongs[i];
    }
  emit(dataChanged(index(0,0),index(m_selectedSongs.size()-1,0)));
}

int CSongbookModel::selectedCount() const
{
  int count = 0;
  for (int i = 0; i < m_selectedSongs.size(); ++i)
    {
      if (m_selectedSongs[i])
	count++;
    }
  return count;
}

QStringList CSongbookModel::selectedPaths() const
{
  QStringList songPaths;

  for (int i = 0; i < m_selectedSongs.size(); ++i)
    {
      if (m_selectedSongs[i])
	songPaths << data(index(i,0), CLibrary::PathRole).toString();
    }
  return songPaths;
}

void CSongbookModel::selectLanguages(const QStringList &languages)
{
  for (int i = 0; i < m_selectedSongs.size(); ++i)
    {
      m_selectedSongs[i] = false;
      if (languages.contains(data(index(i,0), CLibrary::LanguageRole).toString()))
	m_selectedSongs[i] = true;
    }
  emit(dataChanged(index(0,0),index(m_selectedSongs.size()-1,0)));
}

bool CSongbookModel::selectPaths(QStringList &paths)
{
  bool ok = true;
  
  if (paths.count() == 0)
    unselectAll();

  for (int i = 0; i < m_selectedSongs.size(); ++i)
    {
      m_selectedSongs[i] = false;
      if (paths.contains(data(index(i,0), CLibrary::PathRole).toString()))
	m_selectedSongs[i] = true;
      else
	ok = false;
    }
  emit(dataChanged(index(0,0),index(m_selectedSongs.size()-1,0)));
  return ok;
}

QVariant CSongbookModel::data(const QModelIndex &index, int role) const
{
  if (index.column() == 0 && role == Qt::CheckStateRole)
    {
      return (m_selectedSongs[index.row()] ? Qt::Checked : Qt::Unchecked);
    }
  return CIdentityProxyModel::data(index, role);
}

Qt::ItemFlags CSongbookModel::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return Qt::NoItemFlags;
  return Qt::ItemIsUserCheckable | CIdentityProxyModel::flags(index);
}

bool CSongbookModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (index.column() == 0 && role == Qt::CheckStateRole)
    {
      m_selectedSongs[index.row()] = value.toBool();
      emit(dataChanged(index, index));
      return true;
    }
  return CIdentityProxyModel::setData(index, value, role);
}

void CSongbookModel::sourceModelAboutToBeReset()
{
  m_selectedPaths = selectedPaths();
  beginResetModel();
}

void CSongbookModel::sourceModelReset()
{
  m_selectedSongs.clear();
  for (int i = 0; i < sourceModel()->rowCount(); ++i)
    {
      m_selectedSongs << false;
    }
  selectPaths(m_selectedPaths);
  endResetModel();
}
