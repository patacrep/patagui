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
#include "songbook-panel.hh"
#include "songbook.hh"
#include "label.hh"

#include <QDialog>
#include <QLabel>
#include <QLayout>
#include <QFormLayout>
#include <QBoxLayout>
#include <QScrollArea>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QDebug>

CSongbookPanel::CSongbookPanel(CSongbook* parent)
  : QWidget()
  , m_songbook(parent)
  , m_titleLabel(new QLabel)
  , m_authorsLabel(new QLabel)
  , m_styleLabel(new QLabel)
  , m_picture(new QPixmap)
{
  QWidget* form = new QWidget;
  QFormLayout* formLayout = new QFormLayout;
  formLayout->addRow(tr("<b>Title:</b>"), m_titleLabel);
  formLayout->addRow(tr("<b>Authors:</b>"), m_authorsLabel);
  formLayout->addRow(tr("<b>Style:</b>"), m_styleLabel);
  form->setLayout(formLayout);

  QLabel* pic = new QLabel;
  if(!m_picture->isNull())
    pic->setPixmap(m_picture->scaled(256,256,Qt::KeepAspectRatio, Qt::SmoothTransformation));

  QHBoxLayout* mainLayout = new QHBoxLayout;
  mainLayout->addWidget(pic);
  mainLayout->addWidget(form);
  
  setLayout(mainLayout);
  update();
  connect(this, SIGNAL(songbookChanged()), this, SLOT(update()));
}

CSongbookPanel::~CSongbookPanel()
{}

void CSongbookPanel::update()
{
  m_titleLabel->setText(songbook()->title());
  m_authorsLabel->setText(songbook()->authors());
  m_styleLabel->setText(songbook()->style());
  m_picture = songbook()->picture();
}

CSongbook * CSongbookPanel::songbook() const
{
  return m_songbook;
}

void CSongbookPanel::setSongbook(CSongbook *songbook)
{
  m_songbook = songbook;
  emit(songbookChanged());
}


void CSongbookPanel::settingsDialog()
{
  qDebug() << "settingsDialog";
  QDialog *dialog = new QDialog(this);
  dialog->setWindowTitle(tr("Songbook settings"));
  QVBoxLayout *layout = new QVBoxLayout;

  QScrollArea *songbookScrollArea = new QScrollArea();
  songbookScrollArea->setMinimumWidth(400);
  songbookScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  songbookScrollArea->setWidget(settingsWidget());

  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
  connect( buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()) );

  QPushButton *button = buttonBox->addButton(QDialogButtonBox::Reset);
  connect( button, SIGNAL(clicked()), songbook(), SLOT(reset()) );

  layout->addWidget(songbookScrollArea);
  layout->addWidget(buttonBox);
  dialog->setLayout(layout);
  dialog->show();
}

QWidget* CSongbookPanel::settingsWidget()
{
  if(!songbook())
    return new QLabel(tr("No songbook found"));

  QWidget * settings = new QWidget;

  QBoxLayout *templateLayout = new QHBoxLayout;

  QComboBox* combobox = new QComboBox;
  combobox->addItems(songbook()->templates());
  combobox->setCurrentIndex(songbook()->templates().indexOf("patacrep.tmpl"));
  connect(combobox, SIGNAL(currentIndexChanged(const QString &)),
	  songbook(), SLOT(setTmpl(const QString &)));

  templateLayout->addWidget(new QLabel(tr("Template:")));
  templateLayout->addWidget(combobox);

  songbook()->changeTemplate();

  QBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addLayout(templateLayout);
  mainLayout->addWidget(songbook()->propertyEditor());

  settings->setLayout(mainLayout);
  return settings;
}
