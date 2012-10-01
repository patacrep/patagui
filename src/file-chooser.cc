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
#include "file-chooser.hh"

#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QLineEdit>
#include <QPushButton>
#include <QBoxLayout>
#include <QSettings>

CFileChooser::CFileChooser(QWidget *parent)
  : QWidget(parent)
  , m_lineEdit(0)
  , m_button(0)
  , m_caption(QCoreApplication::applicationName())
  , m_directory(QDir::homePath())
  , m_path("")
  , m_filter("")
  , m_options(0)
{
  m_lineEdit = new QLineEdit();
  connect(m_lineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(setPath(const QString &)));

  m_button = new QPushButton(tr("Browse"));
  connect(m_button, SIGNAL(clicked()), SLOT(browse()));

  QLayout *layout = new QHBoxLayout;
  layout->addWidget(m_lineEdit);
  layout->addWidget(m_button);
  // disable layout's margin to have a proper "one widget" appeareance
  layout->setContentsMargins(0, 0, 0, 0);
  setLayout(layout);
}

CFileChooser::~CFileChooser()
{
}

void CFileChooser::browse()
{
  QString selection;
  if (options() & QFileDialog::ShowDirsOnly)
    selection = QFileDialog::getExistingDirectory(this, caption(), directory());
  else
    selection = QFileDialog::getOpenFileName(this, caption(), directory(), filter(), 0, options());

  if (!selection.isEmpty())
    setPath(selection);
}

QFileDialog::Options CFileChooser::options() const
{
  return m_options;
}

void CFileChooser::setOptions(const QFileDialog::Options &opts)
{
  m_options = opts;
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

QString CFileChooser::directory() const
{
  return m_directory;
}

void CFileChooser::setDirectory(const QString &directory)
{
  m_directory = directory;
}

void CFileChooser::setDirectory(const QDir &directory)
{
  m_directory = directory.absolutePath();
}


QString CFileChooser::path() const
{
  return m_path;
}

void CFileChooser::setPath(const QString &path)
{
  if (QString::compare(m_path, path, Qt::CaseSensitive) == 0)
    return;

  m_path = path;
  m_lineEdit->setText(m_path);

  QFileInfo fileInfo(m_path);
  if (fileInfo.isDir())
    setDirectory(m_path);
  else
    setDirectory(fileInfo.dir());

  emit(pathChanged(m_path));
}
