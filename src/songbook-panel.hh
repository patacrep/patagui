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
#ifndef __SONGBOOK_PANEL_HH__
#define __SONGBOOK_PANEL_HH__

#include <QWidget>

class CSongbook;
class QLabel;
class QPixmap;

class CSongbookPanel : public QWidget
{
  Q_OBJECT

public:
  CSongbookPanel();
  ~CSongbookPanel();

  CSongbook * songbook() const;
  QWidget* settingsWidget();

public slots:
  void settingsDialog();
  void setSongbook(CSongbook *songbook);
  void update(bool invalid = false);

signals:
  void songbookChanged();

private:
  CSongbook *m_songbook;

  QLabel* m_titleLabel;
  QLabel* m_authorsLabel;
  QLabel* m_styleLabel;
  QLabel* m_pictureLabel;
  QPixmap* m_picture;

  QWidget* m_settingsWidget;
};

#endif // __SONGBOOK_PANEL_HH__
