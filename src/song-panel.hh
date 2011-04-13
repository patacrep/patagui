// Copyright (C) 2011, Romain Goffe <romain.goffe@gmail.com>
// Copyright (C) 2011, Alexandre Dupas <alexandre.dupas@gmail.com>
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
#ifndef __SONG_PANEL_HH__
#define __SONG_PANEL_HH__

#include <QWidget>
#include <QModelIndex>

class QAbstractItemModel;
class QLabel;

class CLabel;

class CSongPanel : public QWidget
{
  Q_OBJECT

public:
  CSongPanel(QWidget *parent = 0);
  ~CSongPanel();

  QAbstractItemModel * library() const;
  void setLibrary(QAbstractItemModel *library);

  QModelIndex currentIndex() const;

public slots:
  void setCurrentIndex(const QModelIndex &index);
  void update();

private:
  QAbstractItemModel *m_library;
  QModelIndex m_currentIndex;

  CLabel *m_titleLabel;
  CLabel *m_artistLabel;
  CLabel *m_albumLabel;
  QLabel *m_coverLabel;
};

#endif // __SONG_PANEL_HH__
