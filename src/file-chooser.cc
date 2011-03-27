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
#include <QDir>
#include <QFileDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QDebug>
#include <QBoxLayout>

CFileChooser::CFileChooser(const TypeChooser & type)
  : QWidget()
  ,m_readOnly(false)
  ,m_type(type)
  ,m_filter()
  ,m_text()
  ,m_windowTitle(QString(tr("Choose a file")))
  ,m_defaultLocation(QDir::currentPath())
  ,m_lineEdit(new QLineEdit)
  ,m_button(new QPushButton(tr("Browse")))
{
  connect(m_button, SIGNAL(clicked()), 
	  this, SLOT(browse()));

  QLayout* layout = new QHBoxLayout;
  layout->addWidget(m_lineEdit);
  layout->addWidget(m_button);
  setLayout(layout);
}

CFileChooser::~CFileChooser()
{}

void CFileChooser::browse()
{
  QString str;
  if (type() == FileChooser)
    str = QFileDialog::getOpenFileName(NULL, windowTitle(), defaultLocation(), filter());
  else if (type() == DirectoryChooser)
    str = QFileDialog::getExistingDirectory(NULL, windowTitle(), defaultLocation());
  else
    qWarning() << "CFileChooser::browse : unknow type";

  if (!str.isEmpty())
    m_lineEdit->setText(str);
}

QString CFileChooser::filter() const
{
  return m_filter;
}

void CFileChooser::setFilter(QString str)
{
  if (type() == DirectoryChooser)
    qWarning() << "CFileChooser::setFilter : directories do not support filters";

  m_filter = str;
}

QString CFileChooser::text() const
{
  return m_lineEdit->text();
}

void CFileChooser::setText(QString str)
{
  m_lineEdit->setText(str);
}

QString CFileChooser::windowTitle() const
{
  return m_windowTitle;
}

void CFileChooser::setWindowTitle(QString str)
{
  m_windowTitle = str;
}

QString CFileChooser::defaultLocation() const
{
  return m_defaultLocation;
}

void CFileChooser::setDefaultLocation(QString str)
{
  m_defaultLocation = str;
}

CFileChooser::TypeChooser CFileChooser::type() const
{
  return m_type;
}

void CFileChooser::setType(CFileChooser::TypeChooser type)
{
  m_type = type;
}

QLineEdit* CFileChooser::lineEdit() const
{
  return m_lineEdit;
}

QPushButton* CFileChooser::button() const
{
  return m_button;
}
