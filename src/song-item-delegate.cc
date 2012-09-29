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
}

CSongItemDelegate::~CSongItemDelegate()
{}

void CSongItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  QStyleOptionViewItemV4 opt(option);
  opt.state &= ~QStyle::State_MouseOver;
  opt.state &= ~QStyle::State_HasFocus;

  QPalette::ColorRole textColor = QPalette::NoRole;
  opt.state &= ~QStyle::State_HasFocus;
  if (opt.state & QStyle::State_Selected)
    {
      if (opt.state & QStyle::State_Active)
	{
	  painter->fillRect(opt.rect, opt.palette.highlight());
	  textColor = QPalette::HighlightedText;
	}
      else
	{
#if defined(Q_OS_WIN32)
	  painter->fillRect(opt.rect, opt.palette.button());
#endif
	}
    }

  switch (index.column())
    {
    case 2:
      {
        if (index.model()->data(index, CLibrary::LilypondRole).toBool())
          {
            QPixmap pixmap;
            if (!QPixmapCache::find("lilypond-checked", &pixmap))
              {
		pixmap = QIcon::fromTheme("audio-x-generic", QIcon(":/icons/tango/22x22/mimetypes/audio-x-generic.png")).pixmap(22,22);
		QPixmapCache::insert("lilypond-checked", pixmap);
	      }
	    QApplication::style()->drawItemPixmap(painter,
						  opt.rect,
						  Qt::AlignCenter,
						  pixmap);
          }
      }
      break;
    case 4:
      {
        // draw the cover
        QPixmap pixmap;
        if (!QPixmapCache::find("cover-missing-small", &pixmap))
	  {
	    pixmap = QIcon::fromTheme("image-missing", QIcon(":/icons/tango/22x22/status/image-missing.png")).pixmap(22, 22);
	    QPixmapCache::insert("cover-missing-small", pixmap);
	  }
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
	QString locale = QLocale(lang).name();
        QPixmap pixmap;
        if (!QPixmapCache::find(locale, &pixmap))
          {
	    pixmap = QIcon::fromTheme(QString("flag-%1").arg(locale.split('_').first()),
				      QIcon(QString(":/icons/songbook/22x22/flags/flag-%1.png").arg(locale.split('_').first()))).pixmap(22,22);
	    QPixmapCache::insert(locale, pixmap);
	  }
	QApplication::style()->drawItemPixmap(painter,
					      opt.rect,
					      Qt::AlignCenter,
					      pixmap);
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
