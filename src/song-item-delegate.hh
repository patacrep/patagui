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

#ifndef __SONG_ITEM_DELEGATE_HH__
#define __SONG_ITEM_DELEGATE_HH__

#include <QStyledItemDelegate>

/*!
  \file song-item-delegate.hh
  \class CSongItemDelegate
  \brief CSongItemDelegate draws items from LibraryView.

  Items are rows corresponding to a song and are composed of several
  columns for title, artist, album, language  etc.
*/
class CSongItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    /// Constructor.
    CSongItemDelegate(QObject *parent = 0);

    /// Destructor.
    ~CSongItemDelegate();

    /*!
    Draws the song item at position \a index.
  */
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    /*!
    Returns a fixed size hint for columns 2 and 5 (lilypond and language columns).
  */
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // __SONG_ITEM_DELEGATE_HH__
