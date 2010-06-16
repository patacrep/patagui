// Copyright (C) 2010 Alexandre Dupas
//
// Songbook Creator is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// Songbook Creator is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
// MA  02110-1301, USA.
//******************************************************************************

/**
 * \file label.hh
 *
 * Class for label with elide support.
 *
 */
#ifndef __LABEL_HH__
#define __LABEL_HH__

#include <QLabel>

class CLabel : public QLabel
{
  Q_OBJECT
  Q_PROPERTY(Qt::TextElideMode elideMode READ elideMode WRITE setElideMode)

public:
  CLabel(QWidget *parent = 0);
  ~CLabel();

  Qt::TextElideMode elideMode() const;
  void setElideMode(Qt::TextElideMode mode);
  
protected:
    void paintEvent(QPaintEvent *event);

private:
  Qt::TextElideMode m_elideMode;
  QString m_textCache;
};

#endif // __LABEL_HH__
