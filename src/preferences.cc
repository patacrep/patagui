// Copyright (C) 2009 Romain Goffe, Alexandre Dupas
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

#include "preferences.hh"

#include <QtGui>

#include <iostream>

ConfigDialog::ConfigDialog()
  : QDialog()
{
  contentsWidget = new QListWidget;
  contentsWidget->setViewMode(QListView::IconMode);
  contentsWidget->setIconSize(QSize(96, 84));
  contentsWidget->setMovement(QListView::Static);
  contentsWidget->setMaximumWidth(128);
  contentsWidget->setSpacing(12);
  
  pagesWidget = new QStackedWidget;
  pagesWidget->addWidget(new OptionsPage);
  pagesWidget->addWidget(new DisplayPage);
  
  QPushButton *closeButton = new QPushButton(tr("Close"));
  
  createIcons();
  contentsWidget->setCurrentRow(0);
  
  connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
  
  QHBoxLayout *horizontalLayout = new QHBoxLayout;
  horizontalLayout->addWidget(contentsWidget);
  horizontalLayout->addWidget(pagesWidget, 1);
  
  QHBoxLayout *buttonsLayout = new QHBoxLayout;
  buttonsLayout->addStretch(1);
  buttonsLayout->addWidget(closeButton);
  
  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addLayout(horizontalLayout);
  mainLayout->addStretch(1);
  mainLayout->addSpacing(12);
  mainLayout->addLayout(buttonsLayout);
  setLayout(mainLayout);

  setWindowTitle(tr("Preferences"));
}

QSize ConfigDialog::sizeHint() const
{
  return QSize(700,300);
}

void ConfigDialog::createIcons()
{
  QListWidgetItem *optionsButton = new QListWidgetItem(contentsWidget);
  optionsButton->setIcon(QIcon(":/icons/preferences-system.png"));
  optionsButton->setText(tr("Options"));
  optionsButton->setTextAlignment(Qt::AlignHCenter);
  optionsButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

  QListWidgetItem *displayButton = new QListWidgetItem(contentsWidget);
  displayButton->setIcon(QIcon(":/icons/preferences-columns.png"));
  displayButton->setText(tr("Display"));
  displayButton->setTextAlignment(Qt::AlignHCenter);
  displayButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
  
  connect(contentsWidget,
          SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
          this, SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));
}

void ConfigDialog::changePage(QListWidgetItem *current, 
                              QListWidgetItem *previous)
{
  if (!current)
    current = previous;
  
  pagesWidget->setCurrentIndex(contentsWidget->row(current));
}

void ConfigDialog::closeEvent(QCloseEvent *event)
{
  for( int i = 0 ; i < pagesWidget->count() ; ++i )
    {
      pagesWidget->widget(i)->close();
    }
}


DisplayPage::DisplayPage(QWidget *parent)
  : QWidget(parent)
{
  QGroupBox *displayColumnsGroupBox = new QGroupBox(tr("Display Columns"));
  
  artistCheckBox = new QCheckBox(tr("Artist"));
  titleCheckBox = new QCheckBox(tr("Title"));
  pathCheckBox = new QCheckBox(tr("Path"));
  albumCheckBox = new QCheckBox(tr("Album"));
  lilypondCheckBox = new QCheckBox(tr("Lilypond"));
  coverCheckBox = new QCheckBox(tr("Cover"));

  QVBoxLayout *displayColumnsLayout = new QVBoxLayout;
  displayColumnsLayout->addWidget(artistCheckBox);
  displayColumnsLayout->addWidget(titleCheckBox);
  displayColumnsLayout->addWidget(pathCheckBox);
  displayColumnsLayout->addWidget(albumCheckBox);
  displayColumnsLayout->addWidget(lilypondCheckBox);
  displayColumnsLayout->addWidget(coverCheckBox);
  displayColumnsGroupBox->setLayout(displayColumnsLayout);
  
  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(displayColumnsGroupBox);
  mainLayout->addStretch(1);
  setLayout(mainLayout);

  readSettings();
}

void DisplayPage::readSettings()
{
  QSettings settings;
  settings.beginGroup("display");
  artistCheckBox->setChecked(settings.value("artist", true).toBool());
  titleCheckBox->setChecked(settings.value("title", true).toBool());
  pathCheckBox->setChecked(settings.value("path", false).toBool());
  albumCheckBox->setChecked(settings.value("album", true).toBool());
  lilypondCheckBox->setChecked(settings.value("lilypond", false).toBool());
  coverCheckBox->setChecked(settings.value("cover", true).toBool());
  settings.endGroup();
}

void DisplayPage::writeSettings()
{
  QSettings settings;
  settings.beginGroup("display");
  settings.setValue("artist", artistCheckBox->isChecked());
  settings.setValue("title", titleCheckBox->isChecked());
  settings.setValue("path", pathCheckBox->isChecked());
  settings.setValue("album", albumCheckBox->isChecked());
  settings.setValue("lilypond", lilypondCheckBox->isChecked());
  settings.setValue("cover", coverCheckBox->isChecked());
  settings.endGroup();
}

void DisplayPage::closeEvent(QCloseEvent *event)
{
  writeSettings();
  event->accept();
}


OptionsPage::OptionsPage(QWidget *parent)
  : QWidget(parent)
{
  QSettings settings;

  QGroupBox *workingPathGroupBox = new QGroupBox(tr("Directory for Patacrep Songbook"));

  QPushButton *browseWorkingPathButton = new QPushButton(tr("Browse"));
  connect(browseWorkingPathButton, SIGNAL(clicked()), this, SLOT(browse()));
  workingPath = new QLineEdit(QString());

  QGroupBox *bookTypeGroupBox = new QGroupBox(tr("Book Type"));

  QButtonGroup *bookTypeGroup = new QButtonGroup();
  chordbookRadioButton = new QRadioButton(tr("Chordbook"));
  bookTypeGroup->addButton(chordbookRadioButton);
  lyricbookRadioButton = new QRadioButton(tr("Lyricbook"));
  bookTypeGroup->addButton(lyricbookRadioButton);
  
  QGroupBox *chordbookOptionsGroupBox = new QGroupBox(tr("Chordbook Options"));
  
  diagramCheckBox = new QCheckBox(tr("Chord Diagram"));
  lilypondCheckBox = new QCheckBox(tr("Lilypond"));
  tablatureCheckBox = new QCheckBox(tr("Tablature"));

  QHBoxLayout *workingPathLayout = new QHBoxLayout;
  workingPathLayout->addWidget(workingPath,50);
  workingPathLayout->addWidget(browseWorkingPathButton);
  workingPathGroupBox->setLayout(workingPathLayout);

  QVBoxLayout *bookTypeLayout = new QVBoxLayout;
  bookTypeLayout->addWidget(chordbookRadioButton);
  bookTypeLayout->addWidget(lyricbookRadioButton);
  bookTypeGroupBox->setLayout(bookTypeLayout);

  QVBoxLayout *chordbookOptionsLayout = new QVBoxLayout;
  chordbookOptionsLayout->addWidget(diagramCheckBox);
  chordbookOptionsLayout->addWidget(lilypondCheckBox);
  chordbookOptionsLayout->addWidget(tablatureCheckBox);
  chordbookOptionsGroupBox->setLayout(chordbookOptionsLayout);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(workingPathGroupBox);
  mainLayout->addWidget(bookTypeGroupBox);
  mainLayout->addWidget(chordbookOptionsGroupBox);
  mainLayout->addStretch(1);
  setLayout(mainLayout);

  readSettings();
}

void OptionsPage::browse()
{
  QString directory = QFileDialog::getExistingDirectory(this,
                                                        tr("Find Files"), 
                                                        workingPath->text());
  
  //Last char proof ! -> useless/to remove
  while(directory.endsWith("/"))
    {
      std::cout<<"OptionsPage warning"<<std::endl;
      directory.remove(directory.lastIndexOf("/"),1);
    }
  
  if (!directory.isEmpty())
    workingPath->setText(directory);
}

void OptionsPage::readSettings()
{
  QSettings settings;
  workingPath->setText(settings.value("workingPath", QString("%1/").arg(QDir::currentPath())).toString());

  settings.beginGroup("options");
  chordbookRadioButton->setChecked(settings.value("chordbook", true).toBool());
  lyricbookRadioButton->setChecked(settings.value("lyricbook", false).toBool());
  diagramCheckBox->setChecked(settings.value("chordDiagram", true).toBool());
  lilypondCheckBox->setChecked(settings.value("lilypond", false).toBool());
  tablatureCheckBox->setChecked(settings.value("tablature", true).toBool());
  settings.endGroup();
}

void OptionsPage::writeSettings()
{
  QSettings settings;
  settings.setValue("workingPath", workingPath->text());

  settings.beginGroup("options");
  settings.setValue("chordbook", chordbookRadioButton->isChecked());
  settings.setValue("lyricbook", lyricbookRadioButton->isChecked());
  settings.setValue("chordDiagram", diagramCheckBox->isChecked());
  settings.setValue("lilypond", lilypondCheckBox->isChecked());
  settings.setValue("tablature", tablatureCheckBox->isChecked());
  settings.endGroup();  
}

void OptionsPage::closeEvent(QCloseEvent *event)
{
  writeSettings();
  event->accept();
}
