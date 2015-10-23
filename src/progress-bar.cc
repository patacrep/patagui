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
#include "progress-bar.hh"

#include <QProgressBar>
#include <QToolButton>
#include <QBoxLayout>

ProgressBar::ProgressBar(QWidget* parent)
    : QWidget(parent)
    , m_progressBar(new QProgressBar(this))
    , m_cancelButton(new QToolButton(this))
    , m_cancelable(true)
{
    m_cancelButton->setIcon(QIcon::fromTheme("process-stop", QIcon(":/icons/tango/32x32/actions/process-stop.png")));

    connect(m_cancelButton, SIGNAL(clicked()), this, SLOT(cancel()));

    QLayout* layout = new QHBoxLayout;
    layout->addWidget(m_progressBar);
    layout->addWidget(m_cancelButton);
    setLayout(layout);
    setContentsMargins(0, 0, 0, 0);

    hide();
}

ProgressBar::~ProgressBar()
{
    delete m_progressBar;
    delete m_cancelButton;
}

void ProgressBar::cancel()
{
    hide();
    emit(canceled());
}

void ProgressBar::hide()
{
    m_progressBar->hide();
    m_cancelButton->hide();
}

void ProgressBar::show()
{
    m_progressBar->show();
    if (isCancelable())
        m_cancelButton->show();
}

void ProgressBar::setTextVisible(bool value)
{
    m_progressBar->setTextVisible(value);
}

void ProgressBar::setRange(int start, int stop)
{
    m_progressBar->setRange(start, stop);
}

void ProgressBar::setValue(int value)
{
    m_progressBar->setValue(value);
}

bool ProgressBar::isCancelable() const
{
    return m_cancelable;
}

void ProgressBar::setCancelable(bool value)
{
    m_cancelable = value;
}
