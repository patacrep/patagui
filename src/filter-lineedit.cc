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
#include "filter-lineedit.hh"

#include <QPainter>

CClearButton::CClearButton(QWidget *parent)
  : QAbstractButton(parent)
{
  setCursor(Qt::ArrowCursor);
  setFocusPolicy(Qt::NoFocus);
  setToolTip(tr("Clear"));
  setMinimumSize(22, 22);
  setVisible(false);

#if QT_VERSION >= 0x040600
  // First check for a style icon
  if (m_icon.isNull())
    {
      QLatin1String iconName = (layoutDirection() == Qt::RightToLeft)
	? QLatin1String("edit-clear-locationbar-ltr")
	: QLatin1String("edit-clear-locationbar-rtl");
      QIcon icon = QIcon::fromTheme(iconName);
      if (!icon.isNull())
	m_icon = icon.pixmap(16, 16).toImage();
    }
#endif
}

void CClearButton::textChanged(const QString &text)
{
  setVisible(!text.isEmpty());
}

void CClearButton::paintEvent(QPaintEvent *event)
{
  Q_UNUSED(event);
  QPainter painter(this);
  
  if (!m_icon.isNull()) {
    int x = (width() - m_icon.width()) / 2 - 1;
    int y = (height() - m_icon.height()) / 2 - 1;
    painter.drawImage(x, y, m_icon);
    return;
  }
  
  // Fall back to boring circle X
  painter.setRenderHint(QPainter::Antialiasing, true);
  
  QPalette p = palette();
  QColor circleColor = isDown() ? p.color(QPalette::Dark) : p.color(QPalette::Mid);
  QColor xColor = p.color(QPalette::Window);
  
  // draw circle
  painter.setBrush(circleColor);
  painter.setPen(circleColor);
  int padding = width() / 5;
  int circleRadius = width() - (padding * 2);
  painter.drawEllipse(padding, padding, circleRadius, circleRadius);
  
  // draw X
  painter.setPen(xColor);
  padding *= 2;
  painter.drawLine(padding, padding, width() - padding, width() - padding);
  painter.drawLine(padding, height() - padding, width() - padding, padding);
}

CFilterLineEdit::CFilterLineEdit(QWidget *parent)
  : LineEdit(parent)
{
  CClearButton* clearButton = new CClearButton(this);
  connect(clearButton, SIGNAL(clicked()), this, SLOT(clear()));
  connect(this, SIGNAL(textChanged(const QString&)),
	  clearButton, SLOT(textChanged(const QString&)));
  addWidget(clearButton, RightSide);

  updateTextMargins();
  setInactiveText(tr("Filter"));
}

CFilterLineEdit::~CFilterLineEdit()
{}
