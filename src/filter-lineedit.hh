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

/**
 * \file filter-lineedit.hh
 *
 * Filter lineedit with embedded clear button.
 *
 */
#ifndef __FILTER_LINEEDIT_HH__
#define __FILTER_LINEEDIT_HH__

#include <QAbstractButton>
#include <QToolButton>
#include "utils/lineedit.hh"

class CClearButton : public QAbstractButton
{
  Q_OBJECT
  
public:
  CClearButton(QWidget *parent = 0);

public slots:
  void textChanged(const QString &text);

protected:
  void paintEvent(QPaintEvent *event);

private:
  QImage m_icon;
};

class CMagButton : public QToolButton
{
  Q_OBJECT

public:
  CMagButton(QWidget *parent = 0);

protected:
  void paintEvent(QPaintEvent *event);

private:
  QImage m_icon;
};

class QAction;

class CFilterLineEdit : public LineEdit
{
  Q_OBJECT

public:
  CFilterLineEdit(QWidget *parent = 0);
  ~CFilterLineEdit();

  void addAction(QAction* action);

private:
  QMenu* m_menu;

};

#endif // __FILTER_LINEEDIT_HH__
