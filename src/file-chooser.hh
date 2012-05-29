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

#ifndef __FILE_CHOOSER_HH__
#define __FILE_CHOOSER_HH__

#include <QWidget>
#include <QString>
#include <QDir>
#include <QFileDialog>

/** \class CFileChooser "file-chooser-widget.hh"
 *  \brief CFileChooser is a small widget composed
 *  of a lineEdit and a pushButton to pick up a file.
 */

class QLineEdit;
class QPushButton;

class CFileChooser : public QWidget
{
  Q_OBJECT

public:
  CFileChooser(QWidget *parent=0);
  ~CFileChooser();

  void readSettings();
  void writeSettings();

  QFileDialog::Options options() const;
  void setOptions(const QFileDialog::Options &);

  QString filter() const;
  void setFilter(const QString &filter);

  QString caption() const;
  void setCaption(const QString &caption);

  QString directory() const;
  void setDirectory(const QString &directory);
  void setDirectory(const QDir &directory);

  QString path() const;

public slots:
  void setPath(const QString &path);

signals:
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
