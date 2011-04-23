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
#ifndef __NOTIFICATION_HH__
#define __NOTIFICATION_HH__

#include <QDockWidget>
#include <QString>
#include "utils/utils.hh"

class CMainWindow;
class QTextEdit;
class QBoxLayout;

/** \brief Notification is a temporary widget to provide information
 */
class CNotify : public QDockWidget
{
  Q_OBJECT

public:
  CNotify(QWidget* parent);
  
  QString message() const;
  void setMessage(const QString &);

  SbPriority priority() const;
  void setPriority(const SbPriority &);

  void addAction(QAction* action);

  void changeBackground();

  CMainWindow* parent() const;

private:
  QTextEdit* m_textEdit;
  QBoxLayout* m_layout;
  SbPriority m_priority;
  
};

#endif // __NOTIFICATION_HH__
