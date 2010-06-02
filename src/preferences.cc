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
#include "header.hh"
#include "custom.hh"

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
  pagesWidget->addWidget(new SongbookAppearancePage);
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

  QListWidgetItem *songbookAppearanceButton = new QListWidgetItem(contentsWidget);
  songbookAppearanceButton->setIcon(QIcon(":/icons/preferences-system.png"));
  songbookAppearanceButton->setText(tr("Songbook\nAppearance"));
  songbookAppearanceButton->setTextAlignment(Qt::AlignHCenter);
  songbookAppearanceButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

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
  QString workingDir = settings.value("workingPath", QString("%1/").arg(QDir::currentPath())).toString();

  QGroupBox *workingPathGroupBox = new QGroupBox(tr("Directory for Patacrep Songbook"));

  QPushButton *browseWorkingPathButton = new QPushButton(tr("Browse"));
  connect(browseWorkingPathButton, SIGNAL(clicked()), this, SLOT(browse()));
  workingPath = new QLineEdit(QString());
  connect(workingPath, SIGNAL(textChanged(const QString&)), this, SLOT(checkWorkingPath(const QString&)));
  m_workingPathValid = new QLabel;
  checkWorkingPath(workingDir);

  QGroupBox *songbookOptionsGroupBox = new QGroupBox(tr("Main options"));

  QButtonGroup *bookTypeGroup = new QButtonGroup();
  chordbookRadioButton = new QRadioButton(tr("Chordbook"));
  bookTypeGroup->addButton(chordbookRadioButton);
  lyricbookRadioButton = new QRadioButton(tr("Lyricbook"));
  bookTypeGroup->addButton(lyricbookRadioButton);
  
  diagramCheckBox = new QCheckBox(tr("Chord Diagram"));
  lilypondCheckBox = new QCheckBox(tr("Lilypond"));
  tablatureCheckBox = new QCheckBox(tr("Tablature"));
  m_lilypondLabel = new QLabel;
  connect(lilypondCheckBox, SIGNAL(stateChanged(int)),this,SLOT(checkLilypondVersion(int)));

  QGridLayout *workingPathLayout = new QGridLayout;
  workingPathLayout->addWidget(workingPath,0,0,1,1);
  workingPathLayout->addWidget(browseWorkingPathButton,0,1,1,1);
  workingPathLayout->addWidget(m_workingPathValid,1,0,2,1);
  workingPathGroupBox->setLayout(workingPathLayout);

  QGridLayout *songbookOptionsLayout = new QGridLayout;
  songbookOptionsLayout->addWidget(chordbookRadioButton,0,0,1,1);
  songbookOptionsLayout->addWidget(lyricbookRadioButton,1,0,1,1);
  songbookOptionsLayout->addWidget(diagramCheckBox,0,1,1,1);
  songbookOptionsLayout->addWidget(lilypondCheckBox,1,1,1,1);
  songbookOptionsLayout->addWidget(tablatureCheckBox,2,1,1,1);
  songbookOptionsLayout->addWidget(m_lilypondLabel,3,0,2,1);
  songbookOptionsGroupBox->setLayout(songbookOptionsLayout);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(workingPathGroupBox);
  mainLayout->addWidget(songbookOptionsGroupBox);
  mainLayout->addStretch(1);
  setLayout(mainLayout);

  readSettings();
}

void OptionsPage::browse()
{
  QString directory = QFileDialog::getExistingDirectory(this,
                                                        tr("Find Files"), 
                                                        workingPath->text());
  
  if (!directory.isEmpty())
    {
      workingPath->setText(directory);
      checkWorkingPath(directory);
    }
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

void OptionsPage::checkWorkingPath(const QString & path)
{
  isValid = false;
  QDir directory(path);
  if(!directory.exists())
    {
      m_workingPathValid->setText("<font color=red>Invalid songbook directory: directory does not exist.</font>");
      return;
    }

  if(!directory.entryList(QDir::Files | QDir::Readable).contains("makefile"))
    {
      m_workingPathValid->setText("<font color=red>Invalid songbook directory: makefile not found.</font>");
      return;
    }

  if(!directory.entryList(QDir::Files | QDir::Readable).contains("mybook.tex"))
    {
      m_workingPathValid->setText("<font color=red>Invalid songbook directory: mybook.tex not found.</font>");
      return;
    }

  // subdirectories
  QDir songs( QString("%1/songs").arg(path) );
  QDir utils( QString("%1/utils").arg(path) );
  QDir lilypond( QString("%1/lilypond").arg(path) );
  QDir img( QString("%1/img").arg(path) );

  if(!songs.exists())
    {
      m_workingPathValid->setText("<font color=red>Invalid songbook directory: subdirectory songs/ not found.</font>");
      return;
    }

  if(!img.exists())
    {
      m_workingPathValid->setText("<font color=red>Invalid songbook directory: subdirectory img/ not found.</font>");
      return;
    }

  isValid = true;

  if(!lilypond.exists())
    {
      m_workingPathValid->setText("<font color=orange>Warning: subdirectory lilypond/ not found.</font>");
      return;
    }

  if(!utils.exists())
    {
      m_workingPathValid->setText("<font color=orange>Warning: subdirectory utils/ not found.</font>");
      return;
    }

  m_workingPathValid->setText("<font color=green>The songbook directory is valid.</font>");
}

void OptionsPage::checkLilypondVersion(int AState)
{
  if(AState==Qt::Checked)
    {
      m_lilypondCheck = new QProcess(this);
      m_grep = new QProcess(this);

      connect(m_lilypondCheck, SIGNAL(error(QProcess::ProcessError)), 
	      this, SLOT(processError(QProcess::ProcessError)));
      connect(m_grep, SIGNAL(readyReadStandardOutput()), 
	      this, SLOT(readProcessOut()));
      
      QStringList argsLily;
      argsLily << "--version";
      QStringList argsGrep;
      argsGrep << "GNU LilyPond";

      m_lilypondCheck->setStandardOutputProcess(m_grep);
      m_lilypondCheck->start("lilypond", argsLily);
      m_grep->start("grep", argsGrep);
    }
  else
    {
      m_lilypondLabel->setText("");
    }
}

void OptionsPage::processError(QProcess::ProcessError error)
{
  m_lilypondLabel->setText(tr("<font color=orange>Warning: <a href=\"http://lilypond.org\">Lilypond</a> not found</font>"));
}

void OptionsPage::readProcessOut()
{
  QString res = m_grep->readAllStandardOutput().data();
  m_lilypondLabel->setText(QString("<font color=green>Found: %1</font>").arg(res));
}




SongbookAppearancePage::SongbookAppearancePage(QWidget *parent)
  : QWidget(parent)
{
  QSettings settings;

  QGroupBox* frontPageOptionsGroupBox = new QGroupBox(tr("Front Page Options"));
  QLabel* ltitle     = new QLabel(tr("Title:"));
  QLabel* lsubtitle  = new QLabel(tr("Subtitle:"));
  QLabel* lauthor    = new QLabel(tr("Author:"));
  QLabel* lversion   = new QLabel(tr("Version:"));
  QLabel* lmail      = new QLabel(tr("Mail:"));
  QLabel* lpicture   = new QLabel(tr("Picture:"));
  QLabel* lcopyright = new QLabel(tr("Copyright:"));

  QString workingDir = settings.value("workingPath", QString("%1/").arg(QDir::currentPath())).toString();  
  CHeader header(workingDir);
  header.retrieveFields();
  m_title = new QLineEdit(header.title());
  m_subtitle = new QLineEdit(header.subtitle());
  m_author = new QLineEdit(header.author());
  m_version = new QLineEdit(header.version());
  m_mail = new QLineEdit(header.mail());
  m_picture =new QLineEdit(QString("%1/img/%2.jpg").arg(workingDir).arg(header.picture()));
  m_copyright = new QLineEdit(header.copyright());
  m_picture->setReadOnly(true);
    
  QToolButton *browsePictureButton = new QToolButton;
  browsePictureButton->setIcon(QIcon(":/icons/document-load.png"));
  connect(browsePictureButton, SIGNAL(clicked()),
	  this, SLOT(browseHeaderPicture()) );
  
  QGroupBox* customOptionsGroupBox = new QGroupBox(tr("Custom options"));
  QLabel* lboxColor = new QLabel(tr("Boxes color:"));
  m_colorLabel = new QLabel;
  m_color = new QColor(209,228,174);
  m_colorLabel->setText(m_color->name());
  m_colorLabel->setPalette(QPalette(*m_color));
  m_colorLabel->setAutoFillBackground(true);
  QLabel* lfontSize = new QLabel(tr("Font size:"));
  m_sliderFontSize = new QSlider(Qt::Horizontal);
  m_sliderFontSize->setRange(0,4);
  m_sliderFontSize->setPageStep(1);
  m_sliderFontSize->setSingleStep(1);
  m_sliderFontSize->setTickPosition(QSlider::TicksBelow);
  m_sliderFontSize->setValue(2);
  QLabel* lsmall = new QLabel(tr("small"));
  QLabel* llarge = new QLabel(tr("large"));
  
  QPushButton *pickColorButton = new QPushButton(tr("Change"));
  connect(pickColorButton, SIGNAL(clicked()), this, SLOT(pickColor()));

  QToolButton *resetColorButton = new QToolButton;
  resetColorButton->setIcon(QIcon(":/icons/edit-clear.png"));
  connect(resetColorButton, SIGNAL(clicked()),
	  this, SLOT(resetColor()) );

  QGridLayout *frontPageLayout = new QGridLayout();
  frontPageLayout->addWidget(ltitle,0,0,1,1);
  frontPageLayout->addWidget(m_title,0,1,1,3);
  frontPageLayout->addWidget(lsubtitle,1,0,1,1);
  frontPageLayout->addWidget(m_subtitle,1,1,1,3);
  frontPageLayout->addWidget(lauthor,2,0,1,1);
  frontPageLayout->addWidget(m_author,2,1,1,3);
  frontPageLayout->addWidget(lversion,3,0,1,1);
  frontPageLayout->addWidget(m_version,3,1,1,3);
  frontPageLayout->addWidget(lmail,4,0,1,1);
  frontPageLayout->addWidget(m_mail,4,1,1,3);
  frontPageLayout->addWidget(lpicture,5,0,1,1);
  frontPageLayout->addWidget(m_picture,5,1,1,2);
  frontPageLayout->addWidget(browsePictureButton,5,3,1,1);
  frontPageLayout->addWidget(lcopyright,6,0,1,1);
  frontPageLayout->addWidget(m_copyright,6,1,1,3);
  frontPageOptionsGroupBox->setLayout(frontPageLayout);

  QGridLayout *customOptionsLayout = new QGridLayout();
  customOptionsLayout->addWidget(lboxColor,0,0,1,1);
  customOptionsLayout->addWidget(m_colorLabel,0,1,1,3);
  customOptionsLayout->addWidget(pickColorButton,0,4,1,1);
  customOptionsLayout->addWidget(resetColorButton,0,5,1,1);
  customOptionsLayout->addWidget(lfontSize,1,0,1,1);
  customOptionsLayout->addWidget(lsmall,1,1,1,1);
  customOptionsLayout->addWidget(m_sliderFontSize,1,2,1,1);
  customOptionsLayout->addWidget(llarge,1,3,1,1);
  customOptionsGroupBox->setLayout(customOptionsLayout);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(frontPageOptionsGroupBox);
  mainLayout->addWidget(customOptionsGroupBox);
  mainLayout->addStretch(1);
  setLayout(mainLayout);

  readSettings();
}

void SongbookAppearancePage::pickColor()
{
  m_color = new QColor(QColorDialog::getColor(QColor(209,228,174), this));
  if (m_color->isValid())
    {
      m_colorLabel->setText(m_color->name());
      m_colorLabel->setPalette(QPalette(*m_color));
      m_colorLabel->setAutoFillBackground(true);
    }
}

void SongbookAppearancePage::resetColor()
{
  if(m_color) delete m_color;
  m_color = new QColor(209,228,174);
  m_colorLabel->setText(m_color->name());
  m_colorLabel->setPalette(QPalette(*m_color));
  m_colorLabel->setAutoFillBackground(true);
  
}

void SongbookAppearancePage::browseHeaderPicture()
{
  //todo: right now, only .jpg is supported since it's hardcoded in dockWidgets
  //problem is that in mybook.tex, there's just the basename so its extension 
  //should be guessed from somewhere else.
  QString filename = QFileDialog::getOpenFileName(this, tr("Open Image File"),
						  "/home",
						  tr("Images (*.jpg)"));
  if (!filename.isEmpty())
    m_picture->setText(filename);
}

void SongbookAppearancePage::readSettings()
{
  QSettings settings;
  settings.beginGroup("options");
  QColor color(209,228,174);
  m_color = new QColor(settings.value("color", color).value<QColor>());
  m_sliderFontSize->setValue(settings.value("fontsize", 2).toInt());
  settings.endGroup();

  //todo: put somewhere else
  if(m_color)
    {
      m_colorLabel->setText(m_color->name());
      m_colorLabel->setPalette(QPalette(*m_color));
      m_colorLabel->setAutoFillBackground(true);
    }
}

void SongbookAppearancePage::writeSettings()
{
  QSettings settings;

  settings.beginGroup("options");
  settings.setValue("color", *m_color);
  settings.setValue("fontsize", m_sliderFontSize->value());
  settings.endGroup();
}

void SongbookAppearancePage::closeEvent(QCloseEvent *event)
{
  writeSettings();
  QSettings settings;
  QString workingDir = settings.value("workingPath", QString("%1/").arg(QDir::currentPath())).toString();
  checkWorkingPath(workingDir);
  if(isValid)
    {
      updateHeader(); //modify mybook.tex with front page settings
      updateCustom(); //modify crepbook.tex with custom settings
    }
  event->accept();
}

void SongbookAppearancePage::updateHeader()
{
  QSettings settings;
  QString workingDir = settings.value("workingPath", QString("%1/").arg(QDir::currentPath())).toString();
  CHeader header(workingDir);
  header.setTitle(m_title->text());
  header.setSubtitle(m_subtitle->text());
  header.setAuthor(m_author->text());
  header.setVersion(m_version->text());
  header.setMail(m_mail->text());
  header.setPicture(m_picture->text());
  header.setCopyright(m_copyright->text());
}

void SongbookAppearancePage::updateCustom()
{
  QSettings settings;
  QString workingDir = settings.value("workingPath", QString("%1/").arg(QDir::currentPath())).toString();  
  CCustom custom(workingDir);
  custom.setColorBox(m_colorLabel->text());
  custom.setFontSize(m_sliderFontSize->sliderPosition());
}

void SongbookAppearancePage::checkWorkingPath(const QString & path)
{
  isValid = false;
  QDir directory(path);
  if(!directory.exists())
    {
      return;
    }

  if(!directory.entryList(QDir::Files | QDir::Readable).contains("makefile"))
    {
      return;
    }

  if(!directory.entryList(QDir::Files | QDir::Readable).contains("mybook.tex"))
    {
      return;
    }

  // subdirectories
  QDir songs( QString("%1/songs").arg(path) );
  QDir utils( QString("%1/utils").arg(path) );
  QDir lilypond( QString("%1/lilypond").arg(path) );
  QDir img( QString("%1/img").arg(path) );

  if(!songs.exists())
    {
      return;
    }

  if(!img.exists())
    {
      return;
    }

  isValid = true;
}
