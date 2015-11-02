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

#ifndef __LABEL_HH__
#define __LABEL_HH__

#include <QLabel>

/**
 * \file label.hh
 * \class Label
 * \brief Label is a custom QLabel with a fixed width and elide mode.
 */
class Label : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(Qt::TextElideMode elideMode READ elideMode WRITE setElideMode)

public:
    /// Constructor.
    Label(QWidget *parent = 0);

    /// Destructor.
    ~Label();

    /// Getter on the elide mode used by the label.
    /// Default is elide on the right.
    /// @return the elide mode.
    Qt::TextElideMode elideMode() const;

    /// Setter on the elide mode used by the label.
    /// @param mode the elide mode.
    void setElideMode(Qt::TextElideMode mode);

protected:
    void paintEvent(QPaintEvent *event);

private:
    Qt::TextElideMode m_elideMode;
    QString m_textCache;
};

#endif // __LABEL_HH__
