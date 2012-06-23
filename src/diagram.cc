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
#include "diagram.hh"
#include "diagram-editor.hh"

#include <QApplication>
#include <QToolBar>
#include <QAction>
#include <QLabel>
#include <QPainter>
#include <QBoxLayout>
#include <QMouseEvent>
#include <QDebug>
#include <QPushButton>
#include <QSpacerItem>

QRegExp CDiagram::reChord("\\\\[ug]tab[\\*]?\\{([^\\}]+)");
QRegExp CDiagram::reFret("\\\\[ug]tab[\\*]?\\{.+\\{(\\d):");
QRegExp CDiagram::reStringsFret(":([^\\}]+)");
QRegExp CDiagram::reStringsNoFret("\\\\[ug]tab[\\*]?\\{.+\\{([^\\}]+)");

CDiagram::CDiagram(const QString & chord, const ChordType & type, QWidget *parent)
  : QWidget(parent)
  , m_type(type)
{
  fromString(chord);
  setBackgroundRole(QPalette::Base);
  setAutoFillBackground(true);
}

CDiagram::~CDiagram()
{
}

QSize CDiagram::minimumSizeHint() const
{
  return QSize(100, 60);
}

QSize CDiagram::sizeHint() const
{
  return QSize(100, 60);
}

QString CDiagram::toString()
{
  QString str;
  switch(m_type)
    {
    case GuitarChord:
      str.append("\\gtab");
      break;
    case UkuleleChord:
      str.append("\\utab");
      break;
    default:
      qWarning() << tr("CDiagram::toString unsupported chord type");
    }

  if (isImportant())
    str.append("*");

  //the chord name such as Am
  str.append( QString("{%1}{").arg(chord()) );
  //the fret
  str.append(QString("%2").arg(fret()));
  //the strings
  if (!fret().isEmpty())
    str.append(":");
  str.append(QString("%3}").arg(strings()));

  return str;
}

void CDiagram::fromString(const QString & str)
{
  if (str.contains("gtab"))
    m_type = GuitarChord;
  else if (str.contains("utab"))
    m_type = UkuleleChord;
  else
    qWarning() << tr("CDiagram::fromString unsupported chord type");

  setImportant(str.contains("*"));

  reChord.indexIn(str);
  setChord(reChord.cap(1));

  reFret.indexIn(str);
  setFret(reFret.cap(1));

  if (fret().isEmpty())
    {
      QString copy(str);
      reStringsNoFret.indexIn(copy.replace("~:",""));
      setStrings(reStringsNoFret.cap(1));
    }
  else
    {
      reStringsFret.indexIn(str);
      setStrings(reStringsFret.cap(1));
    }
}

void CDiagram::paintEvent(QPaintEvent *)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setPen(Qt::black);

  int cellWidth = 12, cellHeight = 12;
  int width = (strings().length() - 1)*cellWidth;
  int padding = 13;

  //draw horizontal lines
  int max = 4;
  foreach (QChar c, strings())
    if (c.digitValue() > max)
      max = c.digitValue();

  Q_ASSERT(max < 10);
  for (int i=0; i<max+1; ++i)
    {
      painter.drawLine(padding, i*cellHeight+padding, width+padding, i*cellHeight+padding);
    }

  int height = max*cellHeight;
  //draw a vertical line for each string
  for (int i=0; i<strings().length(); ++i)
    {
      painter.drawLine(i*cellWidth+padding, padding, i*cellWidth+padding, height+padding);
    }

  //draw played strings
  for (int i=0; i<strings().length(); ++i)
    {
      QRect stringRect(0, 0, cellWidth-4, cellHeight-4);
      int value = strings()[i].digitValue();
      if (value == -1)
	{
	  stringRect.moveTo( (i*cellWidth)+cellWidth/2.0 +3, 3 );
	  painter.setFont(QFont("Arial", 9));
	  painter.drawText(stringRect, Qt::AlignCenter, "X");
	}
      else
	{
	  stringRect.moveTo( (i*cellWidth)+cellWidth/2.0 +3, value*cellHeight+3 );
	  if (value == 0)
	    painter.drawEllipse(stringRect);
	  else
	    fillEllipse(&painter, stringRect, QBrush(QColor(Qt::black)));
	}
    }

  //draw fret
  QRect fretRect(padding-(cellWidth-2), padding+cellHeight/2.0, cellWidth-4, cellHeight);
  painter.setFont(QFont("Arial", 9));
  painter.drawText(fretRect, Qt::AlignCenter, fret());
}

void CDiagram::fillEllipse(QPainter* painter, const QRect & rect, const QBrush & brush)
{
  QPainterPath path;
  path.addEllipse(rect.topLeft().x(), rect.topLeft().y(), rect.width(), rect.height());;
  painter->fillPath(path, brush);
}

QString CDiagram::chord() const
{
  return m_chord;
}

void CDiagram::setChord(const QString & str)
{
  m_chord = str;
}

QString CDiagram::fret() const
{
  return m_fret;
}

void CDiagram::setFret(const QString & str)
{
  m_fret = str;
}

QString CDiagram::strings() const
{
  return m_strings;
}

void CDiagram::setStrings(const QString & str)
{
  m_strings = str;
}

CDiagram::ChordType CDiagram::type() const
{
  return m_type;
}

void CDiagram::setType(const CDiagram::ChordType & type)
{
  m_type = type;
}

bool CDiagram::isImportant() const
{
  return m_important;
}

void CDiagram::setImportant(bool value)
{
  m_important = value;
}

//----------------------------------------------------------------------------

CDiagramWidget::CDiagramWidget(const QString & gtab,
			       const CDiagram::ChordType & type,
			       QWidget *parent)
  : QWidget(parent)
  , m_diagram(new CDiagram(gtab, type))
  , m_chordName(new QLabel)
  , m_selected(false)
  , m_isReadOnly(false)
{
  setBackgroundRole(QPalette::Base);
  setAutoFillBackground(true);
  setMaximumWidth(100);
  setMaximumHeight(120);
  setToolTip(m_diagram->toString());
  setContextMenuPolicy(Qt::ActionsContextMenu);

  QAction* action = new QAction(tr("Edit"), parent);
  action->setIcon(QIcon::fromTheme("accessories-text-editor", QIcon(":/icons/tango/16x16/actions/accessories-text-editor.png")));
  action->setStatusTip(tr("Edit the chord"));
  connect(action, SIGNAL(triggered()), this, SLOT(editChord()));
  addAction(action);

  action = new QAction(tr("Delete"), parent);
  action->setIcon(QIcon::fromTheme("user-trash", QIcon(":/icons/tango/16x16/actions/user-trash.png")));
  action->setStatusTip(tr("Remove this chord"));
  connect(action, SIGNAL(triggered()), this, SLOT(removeChord()));
  addAction(action);

  QBoxLayout* layout = new QVBoxLayout;
  layout->setContentsMargins(6, 6, 6, 6);
  layout->addWidget(m_chordName);
  layout->addWidget(m_diagram);
  setLayout(layout);

  connect(this, SIGNAL(changed()), SLOT(updateBackground()));
  connect(this, SIGNAL(changed()), SLOT(updateChordName()));
  emit changed();
}

CDiagramWidget::~CDiagramWidget()
{}

bool CDiagramWidget::editChord()
{
  if (isReadOnly())
    return false;

  CDiagramEditor editor(this);
  editor.setDiagram(m_diagram);

  if (editor.exec() == QDialog::Accepted)
    {
      m_diagram->setType(editor.chordType());
      m_diagram->setChord(editor.chordName());
      m_diagram->setStrings(editor.chordStrings());
      m_diagram->setFret(editor.chordFret());
      m_diagram->setImportant(editor.isChordImportant());
      setToolTip(m_diagram->toString());
      emit changed();
      return true;
    }
  return false;
}

void CDiagramWidget::removeChord()
{
  if (isReadOnly())
    return;

  emit diagramCloseRequested();
}

void CDiagramWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
  Q_UNUSED(event);
  editChord();
}

void CDiagramWidget::mousePressEvent(QMouseEvent *event)
{
  Q_UNUSED(event);
  emit clicked();
}

void CDiagramWidget::updateBackground()
{
  if (m_diagram->type() == CDiagram::GuitarChord)
    setPalette(QPalette(QColor(114,159,207)));
  else if (m_diagram->type() == CDiagram::UkuleleChord)
    setPalette(QPalette(QColor(173,127,168)));

  if ( m_diagram->isImportant() )
    setBackgroundRole(QPalette::Mid);
  else
    setBackgroundRole(QPalette::Button);

  if ( isSelected() )
    setBackgroundRole(QPalette::Highlight);
}

void CDiagramWidget::updateChordName()
{
  m_chordName->setText(QString("<font size=\"2\"><b>%1</b></font>")
		       .arg(m_diagram->chord().replace("&", QChar(0x266D))));
  m_chordName->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  m_chordName->setMaximumHeight(18);
  m_chordName->setStyleSheet("QLabel{ border-radius: 4px; background-color: palette(mid); }");
}

bool CDiagramWidget::isSelected() const
{
  return m_selected;
}

void CDiagramWidget::setSelected(bool value)
{
  if ( value != m_selected )
    {
      m_selected = value;
      emit changed();
    }
}

QString CDiagramWidget::toString()
{
  return m_diagram->toString();
}

CDiagram::ChordType CDiagramWidget::type() const
{
  return m_diagram->type();
}

QString CDiagramWidget::chord() const
{
  return m_diagram->chord();
}

QString CDiagramWidget::fret() const
{
  return m_diagram->fret();
}

QString CDiagramWidget::strings() const
{
  return m_diagram->strings();
}

bool CDiagramWidget::isImportant() const
{
  return m_diagram->isImportant();
}

bool CDiagramWidget::isReadOnly() const
{
  return m_isReadOnly;
}

void CDiagramWidget::setReadOnly(bool value)
{
  m_isReadOnly = value;
  foreach (QAction *action, actions())
    {
      action->setEnabled(!value);
      action->setVisible(!value);
    }
}

//----------------------------------------------------------------------------

CDiagramArea::CDiagramArea(QWidget *parent)
  : QWidget(parent)
  , m_layout (new QHBoxLayout)
  , m_isReadOnly(false)
{
  m_layout->setContentsMargins(4, 4, 4, 4);

  QBoxLayout *addButtonLayout = new QVBoxLayout;
  QPushButton *addDiagramButton = new QPushButton;
  addDiagramButton->setFlat(true);
  addDiagramButton->setToolTip(tr("Add a new diagram"));
  addDiagramButton->setIcon(QIcon::fromTheme("list-add", QIcon(":/icons/tango/48x48/actions/list-add.png")));
  connect(addDiagramButton, SIGNAL(clicked()), this, SLOT(addDiagram()));
  addButtonLayout->addStretch();
  addButtonLayout->addWidget(addDiagramButton);

  QBoxLayout *mainLayout = new QHBoxLayout;
  mainLayout->addLayout(m_layout);
  mainLayout->addLayout(addButtonLayout);
  mainLayout->addStretch();
  setLayout(mainLayout);

  setFocusPolicy(Qt::StrongFocus);
}

CDiagramWidget * CDiagramArea::addDiagram()
{
  CDiagramWidget *diagram = new CDiagramWidget("\\gtab{}{0:}", CDiagram::GuitarChord);
  diagram->setReadOnly(isReadOnly());
  connect(diagram, SIGNAL(diagramCloseRequested()), SLOT(removeDiagram()));
  connect(diagram, SIGNAL(changed()), SLOT(onDiagramChanged()));
  connect(diagram, SIGNAL(clicked()), SLOT(onDiagramClicked()));
  if (diagram->editChord())
    {
      m_layout->addWidget(diagram);
      emit(contentsChanged());
    }
  else
    {
      delete diagram;
      diagram = 0;
    }
  return diagram;
}

CDiagramWidget * CDiagramArea::addDiagram(const QString & chord, const CDiagram::ChordType & type)
{
  CDiagramWidget *diagram = new CDiagramWidget(chord, type);
  m_layout->addWidget(diagram);
  diagram->setReadOnly(isReadOnly());
  connect(diagram, SIGNAL(diagramCloseRequested()), SLOT(removeDiagram()));
  connect(diagram, SIGNAL(changed()), SLOT(onDiagramChanged()));
  connect(diagram, SIGNAL(clicked()), SLOT(onDiagramClicked()));
  return diagram;
}

void CDiagramArea::removeDiagram()
{
  if (CDiagramWidget *diagram = qobject_cast< CDiagramWidget* >(QObject::sender()))
    {
      m_layout->removeWidget(diagram);
      disconnect(diagram,0,0,0);
      diagram->setParent(0);
      onDiagramChanged();
    }
}

QList<CDiagramWidget*> CDiagramArea::diagrams() const
{
  QList<CDiagramWidget*>  list;
  for (int i=0; i < m_layout->count(); ++i)
    if (CDiagramWidget *diagram = qobject_cast< CDiagramWidget* >(m_layout->itemAt(i)->widget()))
      list << diagram;
  return list;
}

void CDiagramArea::onDiagramChanged()
{
  emit(contentsChanged());
}

void CDiagramArea::onDiagramClicked()
{
  CDiagramWidget *diagram = qobject_cast< CDiagramWidget* >(QObject::sender());
  diagram->setSelected(!diagram->isSelected());

  // multiple selection via Ctrl is allowed in non read only mode
  if (isReadOnly() || QApplication::keyboardModifiers() != Qt::ControlModifier)
    {
      for (int i=0; i < m_layout->count(); ++i)
	{
	  CDiagramWidget *temp = qobject_cast< CDiagramWidget* >(m_layout->itemAt(i)->widget());
	  if (temp && temp != diagram)
	    temp->setSelected(false);
	}
    }

  emit(diagramClicked(diagram));
}

void CDiagramArea::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Delete)
    {
      bool changed = false;
      for (int i=0; i < m_layout->count(); ++i)
        if (CDiagramWidget *diagram = qobject_cast< CDiagramWidget* >(m_layout->itemAt(i)->widget()))
	  if (diagram->isSelected())
            {
	      changed = true;
	      diagram->deleteLater();
            }
      if (changed)
	onDiagramChanged();
    }
}

bool CDiagramArea::isReadOnly() const
{
  return m_isReadOnly;
}

void CDiagramArea::setReadOnly(bool value)
{
  m_isReadOnly = value;
  m_addDiagramButton->setVisible(!value);
  for (int i=0; i < m_layout->count(); ++i)
    if (CDiagramWidget *diagram = qobject_cast< CDiagramWidget* >(m_layout->itemAt(i)->widget()))
      diagram->setReadOnly(value);
}

