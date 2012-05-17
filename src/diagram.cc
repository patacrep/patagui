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

#include <QToolBar>
#include <QAction>
#include <QLabel>
#include <QPainter>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QDialog>
#include <QDialogButtonBox>
#include <QBoxLayout>
#include <QFormLayout>
#include <QMouseEvent>
#include <QDebug>

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
  return QSize(100, 75);
}

QSize CDiagram::sizeHint() const
{
  return QSize(100, 75);
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

  if(isImportant())
    str.append("*");

  //the chord name such as Am
  str.append( QString("{%1}{").arg(chord().replace(QChar(0x266D),"&")) );
  //the fret
  str.append(QString("%2").arg(fret()));
  //the strings
  if(!fret().isEmpty())
    str.append(":");
  str.append(QString("%3}").arg(strings()));

  return str;
}

void CDiagram::fromString(const QString & str)
{
  if(str.contains("gtab"))
    m_type = GuitarChord;
  else if(str.contains("utab"))
    m_type = UkuleleChord;
  else
    qWarning() << tr("CDiagram::fromString unsupported chord type");

  setImportant(str.contains("*"));

  reChord.indexIn(str);
  setChord(reChord.cap(1).replace("&",QChar(0x266D)));

  reFret.indexIn(str);
  setFret(reFret.cap(1));

  if(fret().isEmpty())
    {
      reStringsNoFret.indexIn(str);
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

  int cellWidth = 14, cellHeight = 14;
  int width = (strings().length() - 1)*cellWidth;
  int height = 4*cellHeight;
  int padding = 15;
  //draw a vertical line for each string
  for(int i=0; i<strings().length(); ++i)
    {
      painter.drawLine(i*cellWidth+padding, padding, i*cellWidth+padding, height+padding);
    }

  //draw 5 horizontal lines
  for(int i=0; i<5; ++i)
    {
      painter.drawLine(padding, i*cellHeight+padding, width+padding, i*cellHeight+padding);
    }

  //draw played strings
  for(int i=0; i<strings().length(); ++i)
    {
      QRect stringRect(0, 0, cellWidth-4, cellHeight-4);
      int value = strings()[i].digitValue();
      if(value == -1)
	{
	  stringRect.moveTo( (i*cellWidth)+cellWidth/2.0 +3, 3 );
	  painter.setFont(QFont("Arial", 9));
	  painter.drawText(stringRect, Qt::AlignCenter, "X");
	}
      else
	{
	  stringRect.moveTo( (i*cellWidth)+cellWidth/2.0 +3, value*cellHeight+3 );
	  if(value == 0)
	    painter.drawEllipse(stringRect);
	  else
	    fillEllipse(&painter, stringRect, QBrush(QColor(Qt::black)));
	}
    }

  //draw fret
  QRect fretRect(padding-(cellWidth-4), padding+cellHeight/2.0, cellWidth-4, cellHeight);
  painter.setFont(QFont("Arial", 11));
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

ChordType CDiagram::type() const
{
  return m_type;
}

void CDiagram::setType(const ChordType & type)
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

CDiagramWidget::CDiagramWidget(const QString & gtab, const ChordType & type, QWidget *parent)
  : QWidget(parent)
  , m_diagram(new CDiagram(gtab, type))
  , m_chordName(new QLabel)
  , m_selected(false)
{
  setBackgroundRole(QPalette::Base);
  setAutoFillBackground(true);
  setMinimumWidth(120);
  setMaximumWidth(120);
  setToolTip(m_diagram->toString());
  setContextMenuPolicy(Qt::ActionsContextMenu);

  updateBackground();
  updateChordName();

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
  layout->addWidget(m_chordName);
  layout->addWidget(m_diagram);
  setLayout(layout);

  connect(this, SIGNAL(changed()), SLOT(updateBackground()));
}

CDiagramWidget::~CDiagramWidget()
{}

bool CDiagramWidget::editChord()
{
  QDialog dialog(this);
  dialog.setWindowTitle(tr("Chord editor"));

  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
						     QDialogButtonBox::Cancel);
  connect(buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), &dialog, SLOT(close()));
  connect(this, SIGNAL(diagramChanged()), this, SLOT(updateChordName()));

  QGroupBox *instrumentGroupBox = new QGroupBox(tr("Instrument"));
  QRadioButton *guitar  = new QRadioButton(tr("Guitar"));
  QRadioButton *ukulele = new QRadioButton(tr("Ukulele"));

  guitar->setChecked(m_diagram->type() == GuitarChord);
  ukulele->setChecked(m_diagram->type() == UkuleleChord);

  QVBoxLayout *instrumentLayout = new QVBoxLayout;
  instrumentLayout->addWidget(guitar);
  instrumentLayout->addWidget(ukulele);
  instrumentLayout->addStretch(1);
  instrumentGroupBox->setLayout(instrumentLayout);

  QLineEdit *nameLineEdit = new QLineEdit;
  nameLineEdit->setToolTip(tr("The chord name such as A&m for A-flat minor"));
  nameLineEdit->setText(m_diagram->chord());

  QSpinBox *fretSpinBox = new QSpinBox;
  fretSpinBox->setToolTip(tr("Fret"));
  fretSpinBox->setRange(0,9);
  fretSpinBox->setValue(m_diagram->fret().toInt());

  QLineEdit *stringsLineEdit = new QLineEdit;
  stringsLineEdit->setMaxLength(6);
  stringsLineEdit->setToolTip(tr("Symbols for each string of the guitar from lowest pitch to highest:\n"
			      "  X: string is not to be played\n"
			      "  0: string is to be played open\n"
			      "  [1-9]: string is to be played on the given numbered fret."));
  stringsLineEdit->setText(m_diagram->strings());

  QCheckBox *importantCheckBox = new QCheckBox(tr("Important diagram"));
  importantCheckBox->setToolTip(tr("Mark this diagram as important."));
  importantCheckBox->setChecked(m_diagram->isImportant());

  QFormLayout *chordLayout = new QFormLayout;
  chordLayout->addRow(tr("Name:"), nameLineEdit);
  chordLayout->addRow(tr("Fret:"), fretSpinBox);
  chordLayout->addRow(tr("Strings:"), stringsLineEdit);

  QBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(instrumentGroupBox);
  layout->addLayout(chordLayout);
  layout->addWidget(importantCheckBox);
  layout->addWidget(buttonBox);
  dialog.setLayout(layout);

  if (dialog.exec() == QDialog::Accepted)
    {
      m_diagram->setType(guitar->isChecked() ? GuitarChord : UkuleleChord);
      m_diagram->setChord(nameLineEdit->text());
      m_diagram->setFret((fretSpinBox->value() == 0) ? "" : QString::number(fretSpinBox->value()));
      m_diagram->setStrings(stringsLineEdit->text());
      m_diagram->setImportant(importantCheckBox->isChecked());
      setToolTip(m_diagram->toString());
      updateBackground();
      update();
      emit diagramChanged();
      return true;
    }
  return false;
}

void CDiagramWidget::removeChord()
{
  emit diagramCloseRequested();
}

void CDiagramWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
  editChord();
}

void CDiagramWidget::mousePressEvent(QMouseEvent *event)
{
  setSelected(true);
  emit changed();
}

void CDiagramWidget::mouseReleaseEvent(QMouseEvent *event)
{
  setSelected(false);
  emit changed();
}

void CDiagramWidget::updateBackground()
{
  if (m_diagram->isImportant())
    setBackgroundRole(QPalette::Mid);
  else
    setBackgroundRole(QPalette::Base);

  if ( isSelected() )
    setBackgroundRole(QPalette::Highlight);
}

void CDiagramWidget::updateChordName()
{
  m_chordName->setText(QString("<b>%1</b>").arg(m_diagram->chord().replace("&", QChar(0x266D))));
  m_chordName->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  if (m_diagram->isImportant())
    m_chordName->setStyleSheet("margin: 2px 2px; border-radius: 6px; background-color: palette(dark);");
  else
    m_chordName->setStyleSheet("margin: 2px 2px; border-radius: 6px; background-color: palette(mid);");
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

ChordType CDiagramWidget::type() const
{
  return m_diagram->type();
}
