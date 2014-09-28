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

#include "song-sort-filter-proxy-model.hh"

#include <QPainter>
#include <QMenu>
#include <QAction>

#include <QDebug>

CClearButton::CClearButton(QWidget *parent)
  : QToolButton(parent)
{
  setCursor(Qt::ArrowCursor);
  setFocusPolicy(Qt::NoFocus);
  setToolTip(tr("Clear"));
  setMinimumSize(22, 22);
  setVisible(false);

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
}

void CClearButton::textChanged(const QString &text)
{
  setVisible(!text.isEmpty());
}

void CClearButton::paintEvent(QPaintEvent *event)
{
  Q_UNUSED(event);
  QPainter painter(this);

  if (!m_icon.isNull())
    {
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

CMagButton::CMagButton(QWidget *parent)
  : QToolButton(parent)
{
  setCursor(Qt::ArrowCursor);
  setFocusPolicy(Qt::NoFocus);
  setToolTip(tr("Enter search request"));
  setMinimumSize(22, 22);
  setVisible(true);
}

void CMagButton::paintEvent(QPaintEvent *event)
{
  Q_UNUSED(event);
  QPainter painter(this);

  if (!m_icon.isNull())
    {
      int x = (width() - m_icon.width()) / 2 - 1;
      int y = (height() - m_icon.height()) / 2 - 1;
      painter.drawImage(x, y, m_icon);
      return;
    }

  // Fall back to boring circle X
  painter.setRenderHint(QPainter::Antialiasing, true);

  QPalette p = palette();

  QPen pen(QColor::fromRgb(85,85,85));
  pen.setWidthF(2.9);

  // draw circle
  painter.setPen(pen);
  int padding = width() / 4;
  int circleRadius = (width() - (padding * 2))*3/4;
  painter.drawEllipse(padding, padding, circleRadius, circleRadius);

  // draw
  painter.drawLine(padding+circleRadius, padding+circleRadius, width() - padding, width() - padding);
}

CLocaleButton::CLocaleButton(QWidget *parent)
  : QToolButton(parent)
{
  setCursor(Qt::ArrowCursor);
  setFocusPolicy(Qt::NoFocus);
  setToolTip(tr("Filter by language"));
  setMinimumSize(44, 22);
  setVisible(true);
  setIcon(QIcon::fromTheme("preferences-desktop-locale", QIcon(":/icons/tango/22x22/apps/preferences-desktop-locale.png")));
  setAutoRaise(false);
  setPopupMode(QToolButton::InstantPopup);

  QString style("QToolButton {"
                "border: none;"
		"pressed {border: 1px}"
                "}");

  setStyleSheet(style);
}

CFilterLineEdit::CFilterLineEdit(QWidget *parent)
  : LineEdit(parent)
  , m_menu(new QMenu)
  , m_filterModel(0)
{
  CClearButton *clearButton = new CClearButton(this);
  CMagButton *magButton = new CMagButton(this);
  CLocaleButton *localeButton = new CLocaleButton(this);
  QString style("QListView, QLineEdit {"
                "selection-color: white; "
                "border: 2px groove gray;"
                "border-radius: 13px;"
                "padding: 2px 2px;"
                "background-image: url(:/icons/xxx.png);"
                "background-position: top right;"
                "padding-right: 0px;"
                "}"
                "QLineEdit:focus {"
                "selection-color: white;   "
                "border: 2px groove gray;"
                "border-radius: 13px;"
                "padding: 2px 2px;"
                "background-image: url(:/icons/xxx.png);"
                "padding-right: 0px;"
                "}"
                "QLineEdit:edit-focus {"
                "selection-color: white;   "
                "border: 2px groove gray;"
                "border-radius: 13px;"
                "padding: 2px 2px;"
                "background-image: url(:/icons/xxx.png);"
                "padding-right: 0px;"
                "}");

  setStyleSheet(style);
  setAttribute(Qt::WA_MacShowFocusRect, 0);
  addWidget(magButton, LeftSide);

  connect(clearButton, SIGNAL(clicked()), this, SLOT(clear()));
  connect(this, SIGNAL(textChanged(const QString&)),
	  clearButton, SLOT(textChanged(const QString&)));
  addWidget(clearButton, RightSide);

  addWidget(localeButton, LeftSide);

  localeButton->setMenu(m_menu);
  connect(localeButton, SIGNAL(clicked()), localeButton, SLOT(showMenu()));

  QAction *action = new QAction(tr("english"), this);
  action->setStatusTip(tr("Select/Unselect songs in english"));
  action->setIcon(QIcon::fromTheme("flag-en", QIcon(":/icons/songbook/22x22/flags/flag-en.png")));
  action->setIconVisibleInMenu(true);
  connect(action, SIGNAL(triggered()), SLOT(filterLanguageEnglish()));
  addAction(action);

  action = new QAction(tr("french"), this);
  action->setStatusTip(tr("Select/Unselect songs in french"));
  action->setIcon(QIcon::fromTheme("flag-fr", QIcon(":/icons/songbook/22x22/flags/flag-fr.png")));
  action->setIconVisibleInMenu(true);
  connect(action, SIGNAL(triggered()), SLOT(filterLanguageFrench()));
  addAction(action);

  action = new QAction(tr("spanish"), this);
  action->setStatusTip(tr("Select/Unselect songs in spanish"));
  action->setIcon(QIcon::fromTheme("flag-es", QIcon(":/icons/songbook/22x22/flags/flag-es.png")));
  action->setIconVisibleInMenu(true);
  connect(action, SIGNAL(triggered()), SLOT(filterLanguageSpanish()));
  addAction(action);

  action = new QAction(tr("portuguese"), this);
  action->setStatusTip(tr("Select/Unselect songs in portuguese"));
  action->setIcon(QIcon::fromTheme("flag-pt", QIcon(":/icons/songbook/22x22/flags/flag-pt.png")));
  action->setIconVisibleInMenu(true);
  connect(action, SIGNAL(triggered()), SLOT(filterLanguagePortuguese()));
  addAction(action);

  action = new QAction(tr("italian"), this);
  action->setStatusTip(tr("Select/Unselect songs in italian"));
  action->setIcon(QIcon::fromTheme("flag-pt", QIcon(":/icons/songbook/22x22/flags/flag-it.png")));
  action->setIconVisibleInMenu(true);
  connect(action, SIGNAL(triggered()), SLOT(filterLanguageItalian()));
  addAction(action);

  updateTextMargins();
  setInactiveText(tr("Filter"));
}

CFilterLineEdit::~CFilterLineEdit()
{}

void CFilterLineEdit::addAction(QAction* action)
{
  m_menu->addAction(action);
}

void CFilterLineEdit::setFilterModel(CSongSortFilterProxyModel *filterModel)
{
  m_filterModel = filterModel;
  connect(this, SIGNAL(textChanged(const QString&)),
	  filterModel, SLOT(setFilterString(const QString&)));
}

void CFilterLineEdit::filterLanguageEnglish()
{
  setText(text() + " :en");
}

void CFilterLineEdit::filterLanguageFrench()
{
  setText(text() + " :fr");
}

void CFilterLineEdit::filterLanguageSpanish()
{
  setText(text() + " :es");
}

void CFilterLineEdit::filterLanguagePortuguese()
{
  setText(text() + " :pt");
}

void CFilterLineEdit::filterLanguageItalian()
{
  setText(text() + " :it");
}
