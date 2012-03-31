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
 * \file file-chooser.hh
 */

#ifndef __PROGRESS_BAR_HH__
#define __PROGRESS_BAR_HH__

#include <QProgressBar>

class QProgressBar;
class QToolButton;

/** 
 * \file progress-bar.hh
 * \class CProgressBar
 * \brief CProgressBar is a progress embedded into the CMainWindow's statusBar.
 *  This progressBar provides a cancel button that allows to cancel the action.
 */


class CProgressBar : public QWidget
{
  Q_OBJECT

public:
  CProgressBar(QWidget* parent=0);
  ~CProgressBar();

  //  QProgressBar * progressBar() const;

public:
  void setRange(int start, int stop);
  void setTextVisible(bool value);

public slots:
  void show();
  void hide();
  void setValue(int value);

signals:
  void canceled();

private slots:
  void cancel();

private:
  QProgressBar *m_progressBar;
  QToolButton *m_cancelButton;
};

#endif  // __FILE_CHOOSER_HH__
