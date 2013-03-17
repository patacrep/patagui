// Copyright (C) 2009-2012, Romain Goffe <romain.goffe@gmail.com>
// Copyright (C) 2009-2012, Alexandre Dupas <alexandre.dupas@gmail.com>
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

#include "search-widget.hh"

#include <QPushButton>
#include <QLineEdit>
#include <QBoxLayout>
#include <QSettings>
#include <QTextDocument>
#include <QPlainTextEdit>

#include <QDebug>

CSearchWidget::CSearchWidget(QWidget *parent)
  : QFrame(parent)
  , m_editor(0)
  , m_findLineEdit(new QLineEdit(this))
  , m_findPrevButton(new QPushButton(this))
  , m_findNextButton(new QPushButton(this))
{
  if (QPlainTextEdit *editor = qobject_cast< QPlainTextEdit* >(parent))
    setTextEditor(editor);

  m_findLineEdit->setMinimumWidth(200);

  m_findPrevButton->setFlat(true);
  m_findPrevButton->setMaximumWidth(20);
  m_findPrevButton->setIcon(QIcon::fromTheme("go-up", QIcon(":/icons/tango/48x48/actions/go-up.png")));
  connect(m_findPrevButton, SIGNAL(clicked()), SLOT(find()));

  m_findNextButton->setFlat(true);
  m_findNextButton->setMaximumWidth(20);
  m_findNextButton->setIcon(QIcon::fromTheme("go-down", QIcon(":/icons/tango/48x48/actions/go-down.png")));
  connect(m_findNextButton, SIGNAL(clicked()), SLOT(find()));

  QPushButton *closeButton = new QPushButton(this);
  closeButton->setFlat(true);
  closeButton->setMaximumWidth(20);
  closeButton->setIcon(QIcon::fromTheme("window-close", QIcon(":/icons/tango/48x48/actions/window-close.png")));
  connect(closeButton, SIGNAL(clicked()), SLOT(close()));

  QBoxLayout *layout = new QHBoxLayout;
  layout->addWidget(m_findLineEdit, 1);
  layout->addWidget(m_findPrevButton);
  layout->addWidget(m_findNextButton);
  layout->addWidget(closeButton);

  setLayout(layout);

  setAutoFillBackground(true);
  setFrameStyle(QFrame::StyledPanel);
  setLineWidth(1);
  setContentsMargins(1, 1, 1, 1);
  setVisible(false);

  readSettings();
}

CSearchWidget::~CSearchWidget()
{
  delete m_findLineEdit;
  delete m_findPrevButton;
  delete m_findNextButton;
}

void CSearchWidget::setFocus()
{
  m_findLineEdit->setFocus();
}

void CSearchWidget::keyPressEvent(QKeyEvent *event)
{
  switch (event->key())
    {
    case Qt::Key_Enter:
    case Qt::Key_Return:
      find();
      return;
    case Qt::Key_Escape:
      event->ignore();
      return;
    default:
      return;
    }
}

void CSearchWidget::readSettings()
{
  QSettings settings;
  settings.beginGroup("find-replace");
  m_findLineEdit->setText(settings.value("quick-find", QString()).toString());
  settings.endGroup();
}

void CSearchWidget::writeSettings()
{
  QSettings settings;
  settings.beginGroup("find-replace");
  settings.setValue("quick-find", m_findLineEdit->text());
  settings.endGroup();
}

void CSearchWidget::find()
{
  if (!m_editor)
    return;

  QTextDocument::FindFlags options = 0;
  QString expr = m_findLineEdit->text();

  QPushButton *button = qobject_cast< QPushButton* >(sender());
  if (button == m_findPrevButton)
    options |= QTextDocument::FindBackward;

  if (!m_editor->find(expr, options))
    m_editor->setStatusTip(tr("\"%1\" not found").arg(expr));
}

void CSearchWidget::setTextEditor(QPlainTextEdit *editor)
{
  m_editor = editor;
}
