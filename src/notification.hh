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

class MainWindow;
class QTextEdit;
class QBoxLayout;

/**
 * \file notification.hh
 * \class Notification
 * \brief Notification is a widget to provide exceptionnal information.
 *
 * \image html notification.png
 *
 * A notification is supposed to appear as a ribbon at the top of the main
 * window, giving the user a description of an unusal behavior with
 * possible actions to choose from.
 *
 */
class Notification : public QDockWidget
{
    Q_OBJECT

public:
    enum Priority { LowPriority, MediumPriority, HighPriority };

    /// Constructor
    Notification(QWidget *parent);

    /// Getter on the notification message.
    /// The message is the main information provided by the widget.
    /// Based on this message, the user can select appropriate actions.
    /// @return the message
    QString message() const;

    /// Setter on the notification message.
    /// The message is the main information provided by the widget.
    /// Based on this message, the user can select appropriate actions.
    /// @param message the message
    void setMessage(const QString &message);

    /// Getter on the notification priority.
    /// The priority indicates the level of importance of the notification.
    /// As a result, the icon and color background of the notification area may
    /// change
    /// to highlight this level of priority.
    /// @return the priority
    Priority priority() const;

    /// Setter on the notification priority.
    /// The priority indicates the level of importance of the notification.
    /// As a result, the icon and color background of the notification area may
    /// change
    /// to highlight this level of priority.
    /// @param level the priority level
    void setPriority(const Priority &level);

    /// Add an action to the notification.
    /// Actions result in QPushButtons on the right side,
    /// organised in a vertical layout.
    /// @param action the action
    void addAction(QAction *action);

    /// Getter on the parent widget.
    /// @return the parent widget
    MainWindow *parent() const;

private:
    /// Change the background color of the notification area
    /// based on the priority level.
    void changeBackground();

    QTextEdit *m_textEdit; ///< widget containing icon and message
    QBoxLayout *m_layout;  ///< notification layout
    Priority m_priority;   ///< notification priority level
};

#endif // __NOTIFICATION_HH__
