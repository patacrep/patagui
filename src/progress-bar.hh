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

#ifndef __PROGRESS_BAR_HH__
#define __PROGRESS_BAR_HH__

#include <QProgressBar>

class QProgressBar;
class QToolButton;

/*!
  \file progress-bar.hh
  \class CProgressBar
  \brief CProgressBar is a custom cancelable progress bar.

  A CProgressBar object displays a QProgressBar with a "cancel" button
  if it is cancelable.

  If the "cancel" button is clicked, the signal canceled() is emitted
  and the CProgressBar object is hidden.

  \image html progress-bar.png
*/

class CProgressBar : public QWidget
{
  Q_OBJECT

public:
  /// Constructor.
  CProgressBar(QWidget* parent=0);

  /// Destructor.
  ~CProgressBar();

public:
  /*!
    Sets the range for the progress bar.
    \sa QProgressBar::setRange
  */
  void setRange(int start, int stop);

  /*!
    Sets the visibility of the text over the progress bar to \a value.
    \sa QProgressBar::setTextVisible
  */
  void setTextVisible(bool value);

  /*!
    Returns \a true if the action of the progress can be canceled; \a false otherwise.
    \sa setCancelable
  */
  bool isCancelable() const;

  /*!
    Sets the cancelable property of the progress bar to \a value.
    \sa isCancelable
  */
  void setCancelable(bool value);

public slots:
  /*!
    Shows the progress bar.
    The cancel button is shown if the progress bar is cancelable.
    \sa hide
  */
  void show();

  /*!
    Hides the progress bar.
    \sa show
  */
  void hide();

  /*!
    Sets the current progress of the progress bar.
    \sa QProgressBar::setValue
  */
  void setValue(int value);

signals:
  /*!
    This signal is emitted when the cancel button is clicked.
    \sa cancelable, setCancelable
  */
  void canceled();

private slots:
  void cancel();

private:
  QProgressBar *m_progressBar;
  QToolButton *m_cancelButton;
  bool m_cancelable;
};

#endif  // __FILE_CHOOSER_HH__
