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

#ifndef __FILE_CHOOSER_HH__
#define __FILE_CHOOSER_HH__

#include <QWidget>
#include <QString>
#include <QDir>
#include <QFileDialog>

class QLineEdit;
class QPushButton;

/*!
  \file file-chooser.hh
  \class CFileChooser
  \brief CFileChooser is a small widget composed of a QLineEdit and a QPushButton to pick up a file.

  The QLineEdit object displays a path to a file and the "browse"
  QPushButton is connected to a QFileDialog.

  \image html file-chooser.png
*/
class CFileChooser : public QWidget
{
  Q_OBJECT

public:
  /// Constructor.
  CFileChooser(QWidget *parent=0);

  /// Destructor.
  ~CFileChooser();

  /*!
    Tries to restore the path previously selected.
    \sa writeSettings
  */
  void readSettings();

  /*!
    Saves the selected path.
    \sa readSettings
  */
  void writeSettings();

  /*!
    Returns the options of the QFileDialog .
    \sa setOptions
  */
  QFileDialog::Options options() const;

  /*!
    Sets the options of the QFileDialog .
    \sa options
  */
  void setOptions(const QFileDialog::Options &);

  /*!
    Returns the filter of the QFileDialog.
    \sa setFilter
  */
  QString filter() const;

  /*!
    Sets the filter of the QFileDialog.
    \sa filter
  */
  void setFilter(const QString &filter);

  /*!
    Returns the caption of the QFileDialog.
    \sa setCaption
  */
  QString caption() const;

  /*!
    Sets the caption of the QFileDialog.
    \sa caption
  */
  void setCaption(const QString &caption);

  /*!
    Returns the base directory from which the QFileDialog is opened.
    \sa setDirectory
  */
  QString directory() const;

  /*!
    Sets the base directory from which the QFileDialog is opened.
    \sa directory
  */
  void setDirectory(const QString &directory);

  /*!
    Sets the base directory from which the QFileDialog is opened.
    \sa directory
  */
  void setDirectory(const QDir &directory);

  /*!
    Returns the path from the QLineEdit.
    \sa setPath
  */
  QString path() const;

public slots:
  /*!
    Sets the path for the QLineEdit.
    \sa path
  */
  void setPath(const QString &path);

signals:
  /*!
    This signal is emitted when the path is changed in the QLineEdit.
    \sa path, setPath
  */
  void pathChanged(const QString &path);

private slots:
  void browse();


private:
  QLineEdit* m_lineEdit;
  QPushButton* m_button;
  QString m_caption;
  QString m_directory;
  QString m_filter;
  QFileDialog::Options m_options;
};

#endif  // __FILE_CHOOSER_HH__
