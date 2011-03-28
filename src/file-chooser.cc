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
#include "file-chooser.hh"

#include <QFileDialog>

#include <QFileInfo>
#include <QLineEdit>
#include <QPushButton>
#include <QBoxLayout>

#include <QDebug>

CFileChooser::CFileChooser()
  : QWidget()
  , m_type(CFileChooser::OpenFileChooser)
  , m_filter()
  , m_caption()
  , m_directory(QDir::currentPath())
  , m_lineEdit(0)
  , m_button(0)
{
  m_lineEdit = new QLineEdit();

  m_button = new QPushButton(tr("Browse"));
  connect(m_button, SIGNAL(clicked()), SLOT(browse()));

  QLayout* layout = new QHBoxLayout;
  layout->addWidget(m_lineEdit);
  layout->addWidget(m_button);
  // disable layout's margin to have a proper "one widget" appeareance
  layout->setContentsMargins(0, 0, 0, 0);
  setLayout(layout);
}

CFileChooser::~CFileChooser()
{}

void CFileChooser::browse()
{
  QString path;
  switch (type())
    {
    case CFileChooser::OpenFileChooser:
      path = QFileDialog::getOpenFileName(NULL,
					  caption(),
					  directory().path(),
					  filter());
      break;
    case CFileChooser::SaveFileChooser:
      path = QFileDialog::getSaveFileName(NULL,
					  caption(),
					  directory().path(),
					  filter());
      break;
    case CFileChooser::DirectoryChooser:
      path = QFileDialog::getExistingDirectory(NULL,
					       caption(),
					       directory().path());
      break;
    default:
      qWarning() << "CFileChooser::browse : unknow type";
      break;
    }
  
  if (!path.isEmpty())
    setPath(path);
}

CFileChooser::TypeChooser CFileChooser::type() const
{
  return m_type;
}

void CFileChooser::setType(const CFileChooser::TypeChooser &type)
{
  m_type = type;
}

QString CFileChooser::filter() const
{
  return m_filter;
}

void CFileChooser::setFilter(const QString &filter)
{
  m_filter = filter;
}

QString CFileChooser::caption() const
{
  return m_caption;
}

void CFileChooser::setCaption(const QString &caption)
{
  m_caption = caption;
}

QDir CFileChooser::directory() const
{
  return m_directory;
}

void CFileChooser::setDirectory(const QString &directory)
{
  m_directory = QDir(directory);
}

void CFileChooser::setDirectory(const QDir &directory)
{
  m_directory = directory;
}

QString CFileChooser::path() const
{
  return m_lineEdit->text();
}

void CFileChooser::setPath(const QString &path)
{
  m_lineEdit->setText(path);

  QFileInfo fileInfo(path);
  if (fileInfo.isDir())
    setDirectory(path);
  else
    setDirectory(fileInfo.dir());

  emit(pathChanged(path));
}
