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
#include "song-panel.hh"

#include "library.hh"

#include <QGridLayout>
#include <QLabel>

#include "label.hh"

#include <QDebug>

CSongPanel::CSongPanel(QWidget *parent)
  : QWidget(parent)
  , m_library()
  , m_currentIndex()
  , m_titleLabel()
  , m_artistLabel()
  , m_albumLabel()
  , m_coverLabel()
{
  m_artistLabel = new CLabel;
  m_artistLabel->setElideMode(Qt::ElideRight);
  m_artistLabel->setFixedWidth(175);
  m_titleLabel = new CLabel;
  m_titleLabel->setElideMode(Qt::ElideRight);
  m_titleLabel->setFixedWidth(175);
  m_albumLabel = new CLabel;
  m_albumLabel->setElideMode(Qt::ElideRight);
  m_albumLabel->setFixedWidth(175);
  m_coverLabel = new QLabel;
  m_coverLabel->setAlignment(Qt::AlignTop);
  QGridLayout *layout = new QGridLayout;
  layout->addWidget(m_coverLabel,0,0,3,1);
  layout->addWidget(new QLabel(tr("<b>Title:</b>")),0,1,1,1, Qt::AlignLeft);
  layout->addWidget(m_titleLabel,0,2,1,1);
  layout->addWidget(new QLabel(tr("<b>Artist:</b>")),1,1,1,1, Qt::AlignLeft);
  layout->addWidget(m_artistLabel,1,2,1,1);
  layout->addWidget(new QLabel(tr("<b>Album:</b>")),2,1,1,1, Qt::AlignLeft);
  layout->addWidget(m_albumLabel,2,2,1,1);
  layout->setColumnStretch(2,1);

  setLayout(layout);
}

CSongPanel::~CSongPanel()
{}

QAbstractItemModel * CSongPanel::library() const
{
  return m_library;
}

void CSongPanel::setLibrary(QAbstractItemModel *library)
{
  m_library = library;
  setCurrentIndex(QModelIndex());
}

QModelIndex CSongPanel::currentIndex() const
{
  return m_currentIndex;
}

void CSongPanel::setCurrentIndex(const QModelIndex &currentIndex)
{
  m_currentIndex = currentIndex;
  update();
}

void CSongPanel::update()
{
  if (!library())
    return;

  m_titleLabel->setText(library()->data(currentIndex(), CLibrary::TitleRole).toString());
  m_artistLabel->setText(library()->data(currentIndex(), CLibrary::ArtistRole).toString());
  m_albumLabel->setText(library()->data(currentIndex(), CLibrary::AlbumRole).toString());

  QPixmap pixmap = library()->data(currentIndex(), CLibrary::CoverFullRole).value< QPixmap >();
  if(!pixmap.isNull())
    pixmap = pixmap.scaled(128,128,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
  m_coverLabel->setPixmap(pixmap);
}
