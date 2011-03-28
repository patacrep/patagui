// Copyright (C) 2009-2011 Romain Goffe, Alexandre Dupas
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
 * \file file-chooser.hh
 */

#ifndef __FILE_CHOOSER_HH__
#define __FILE_CHOOSER_HH__

#include <QWidget>
#include <QString>
#include <QDir>

/** \class CFileChooser "file-chooser-widget.hh"
 *  \brief CFileChooser is a small widget composed
 *  of a lineEdit and a pushButton to pick up a file.
 */

class QLineEdit;
class QPushButton;

class CFileChooser : public QWidget
{
  Q_OBJECT
  Q_ENUMS(TypeChooser)

public:
  enum TypeChooser { OpenFileChooser, SaveFileChooser, DirectoryChooser };

  CFileChooser();
  ~CFileChooser();

  TypeChooser type() const;
  void setType(const TypeChooser &type);

  QString filter() const;
  void setFilter(const QString &filter);

  QString caption() const;
  void setCaption(const QString &caption);

  QDir directory() const;
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
  TypeChooser m_type;
  QString m_filter;
  QString m_caption;
  QDir m_directory;

  QLineEdit* m_lineEdit;
  QPushButton* m_button;
};

#endif  // __FILE_CHOOSER_HH__
