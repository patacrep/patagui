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
#include "song-item-delegate.hh"

#include "library.hh"

#include <QApplication>
#include <QStyle>
#include <QPainter>
#include <QLocale>
#include <QPixmapCache>

#include <QDebug>

CSongItemDelegate::CSongItemDelegate(QObject *parent)
  : QStyledItemDelegate(parent)
{
  // lilypond symbol
  QPixmapCache::insert("lilypond-checked", QIcon::fromTheme("audio-x-generic", QIcon(":/icons/tango/22x22/mimetypes/audio-x-generic.png")).pixmap(22,22));

  // cover missing
  QPixmapCache::insert("cover-missing-small", QIcon::fromTheme("image-missing", QIcon(":/icons/tango/22x22/status/image-missing.png")).pixmap(22, 22));
  QPixmapCache::insert("cover-missing-full", QIcon::fromTheme("image-missing", QIcon(":/icons/tango/scalable/status/image-missing.svg")).pixmap(128, 128));
  
  // language flags
  QPixmapCache::insert("fr_FR", QIcon::fromTheme("flag-fr", QIcon(":/icons/tango/scalable/places/flag-fr.svg")).pixmap(22,22));
  QPixmapCache::insert("en_US", QIcon::fromTheme("flag-en", QIcon(":/icons/tango/scalable/places/flag-en.svg")).pixmap(22,22));
  QPixmapCache::insert("es_ES", QIcon::fromTheme("flag-es", QIcon(":/icons/tango/scalable/places/flag-es.svg")).pixmap(22,22));
}

CSongItemDelegate::~CSongItemDelegate()
{}

void CSongItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  QStyleOptionViewItemV4 opt = option;
  opt.state &= ~QStyle::State_HasFocus;

  switch (index.column())
    {
    case 2:
      {
        if (opt.state & QStyle::State_Selected)
          painter->fillRect(opt.rect, opt.palette.highlight());

        if (index.model()->data(index, CLibrary::LilypondRole).toBool())
          {
            QPixmap pixmap;
            if (QPixmapCache::find("lilypond-checked", &pixmap))
              {
                QApplication::style()->drawItemPixmap(painter,
                                                      opt.rect,
                                                      Qt::AlignCenter,
                                                      pixmap);
              }
            else
              {
                QApplication::style()->drawItemText(painter,
                                                    opt.rect,
                                                    Qt::AlignCenter,
                                                    opt.palette,
                                                    true,
                                                    tr("yes"));
              }
          }
      }
      break;
    case 4:
      {
        QPalette::ColorRole textColor = QPalette::NoRole;
        if (opt.state & QStyle::State_Selected)
          {
            painter->fillRect(opt.rect, opt.palette.highlight());
            textColor = QPalette::HighlightedText;
          }

        // draw the cover
        QPixmap pixmap;
        QPixmapCache::find("cover-missing-small", &pixmap);
        if (qVariantCanConvert< QPixmap >(index.model()->data(index, CLibrary::CoverSmallRole)))
          {
            pixmap = qVariantValue< QPixmap >(index.model()->data(index, CLibrary::CoverSmallRole));
          }
        QRect coverRectangle(opt.rect.left(), opt.rect.top() + 2,
                             32, opt.rect.height() - 4);
        QApplication::style()->drawItemPixmap(painter,
                                              coverRectangle,
                                              Qt::AlignCenter,
                                              pixmap);

        // draw the album title
        QRect albumRectangle = opt.rect;
        albumRectangle.setTopLeft(coverRectangle.topRight());
        QApplication::style()->drawItemText(painter,
                                            albumRectangle,
                                            Qt::AlignLeft | Qt::AlignVCenter,
                                            opt.palette,
                                            true,
                                            index.model()->data(index, CLibrary::AlbumRole).toString(),
                                            textColor);
      }
      break;
    case 5:
      {
        QLocale::Language lang = qVariantValue< QLocale::Language >(index.model()->data(index, CLibrary::LanguageRole));
        QPixmap pixmap;
        if (opt.state & QStyle::State_Selected)
          painter->fillRect(opt.rect, opt.palette.highlight());

        if (QPixmapCache::find(QLocale(lang).name(), &pixmap))
          {
            QApplication::style()->drawItemPixmap(painter,
                                                  opt.rect,
                                                  Qt::AlignCenter,
                                                  pixmap);
          }
        else
          {
            QApplication::style()->drawItemText(painter,
                                                opt.rect,
                                                Qt::AlignCenter,
                                                opt.palette,
                                                true,
                                                QLocale::languageToString(lang));
          }
      }
      break;
    default:
      QStyledItemDelegate::paint(painter, opt, index);
      break;
    }
}

QSize CSongItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  switch (index.column())
    {
    case 2:
    case 5:
      return QSize(32, 32);
      break;
    }
  return QStyledItemDelegate::sizeHint(option, index);
}
