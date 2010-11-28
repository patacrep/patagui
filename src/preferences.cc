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

// Config Dialog

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

void ConfigDialog::createIcons()
{
  QListWidgetItem *optionsButton = new QListWidgetItem(contentsWidget);
  //optionsButton->setIcon(QIcon::fromTheme("preferences-system"));
  optionsButton->setText(tr("Options"));
  optionsButton->setTextAlignment(Qt::AlignHCenter);
  optionsButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

  QListWidgetItem *displayButton = new QListWidgetItem(contentsWidget);
  //displayButton->setIcon(QIcon::fromTheme("preferences-columns"));
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

// Display Page

DisplayPage::DisplayPage(QWidget *parent)
  : QWidget(parent)
{
  QGroupBox *displayColumnsGroupBox = new QGroupBox(tr("Display Columns"));

  m_artistCheckBox = new QCheckBox(tr("Artist"));
  m_titleCheckBox = new QCheckBox(tr("Title"));
  m_pathCheckBox = new QCheckBox(tr("Path"));
  m_albumCheckBox = new QCheckBox(tr("Album"));
  m_lilypondCheckBox = new QCheckBox(tr("Lilypond"));
  m_coverCheckBox = new QCheckBox(tr("Cover"));
  m_langCheckBox = new QCheckBox(tr("Language"));

  QVBoxLayout *displayColumnsLayout = new QVBoxLayout;
  displayColumnsLayout->addWidget(m_artistCheckBox);
  displayColumnsLayout->addWidget(m_titleCheckBox);
  displayColumnsLayout->addWidget(m_pathCheckBox);
  displayColumnsLayout->addWidget(m_albumCheckBox);
  displayColumnsLayout->addWidget(m_lilypondCheckBox);
  displayColumnsLayout->addWidget(m_coverCheckBox);
  displayColumnsLayout->addWidget(m_langCheckBox);
  displayColumnsGroupBox->setLayout(displayColumnsLayout);

  QGroupBox *displayWindowsGroupBox = new QGroupBox(tr("Display windows"));
  m_compilationLogCheckBox = new QCheckBox(tr("Compilation log"));

  QVBoxLayout *displayWindowsLayout = new QVBoxLayout;
  displayWindowsLayout->addWidget(m_compilationLogCheckBox);
  displayWindowsGroupBox->setLayout(displayWindowsLayout);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(displayColumnsGroupBox);
  mainLayout->addWidget(displayWindowsGroupBox);
  mainLayout->addStretch(1);
  setLayout(mainLayout);

  readSettings();
}

void DisplayPage::readSettings()
{
  QSettings settings;
  settings.beginGroup("display");
  m_artistCheckBox->setChecked(settings.value("artist", true).toBool());
  m_titleCheckBox->setChecked(settings.value("title", true).toBool());
  m_pathCheckBox->setChecked(settings.value("path", false).toBool());
  m_albumCheckBox->setChecked(settings.value("album", true).toBool());
  m_lilypondCheckBox->setChecked(settings.value("lilypond", false).toBool());
  m_coverCheckBox->setChecked(settings.value("cover", true).toBool());
  m_langCheckBox->setChecked(settings.value("lang", false).toBool());
  m_compilationLogCheckBox->setChecked(settings.value("log", false).toBool());
  settings.endGroup();
}

void DisplayPage::writeSettings()
{
  QSettings settings;
  settings.beginGroup("display");
  settings.setValue("artist", m_artistCheckBox->isChecked());
  settings.setValue("title", m_titleCheckBox->isChecked());
  settings.setValue("path", m_pathCheckBox->isChecked());
  settings.setValue("album", m_albumCheckBox->isChecked());
  settings.setValue("lilypond", m_lilypondCheckBox->isChecked());
  settings.setValue("cover", m_coverCheckBox->isChecked());
  settings.setValue("lang", m_langCheckBox->isChecked());
  settings.setValue("log", m_compilationLogCheckBox->isChecked());
  settings.endGroup();
}

void DisplayPage::closeEvent(QCloseEvent *event)
{
  writeSettings();
  event->accept();
}

// Option Page

OptionsPage::OptionsPage(QWidget *parent)
  : QWidget(parent)
{
  m_workingPath = new QLineEdit;
  m_workingPathValid = new QLabel;
  readSettings();
  QSettings settings;
  QString workingDir = settings.value("workingPath", QString("%1/songbook/").arg(QDir::home().path())).toString();

  // working path
  QGroupBox *workingPathGroupBox
    = new QGroupBox(tr("Directory for Patacrep Songbook"));

  QPushButton *browseWorkingPathButton = new QPushButton(tr("Browse"));
  connect(browseWorkingPathButton, SIGNAL(clicked()),
          this, SLOT(browse()));
  connect(m_workingPath, SIGNAL(textChanged(const QString&)),
          this, SLOT(checkWorkingPath(const QString&)));
  checkWorkingPath(workingDir);

  QGridLayout *workingPathLayout = new QGridLayout;
  workingPathLayout->addWidget(m_workingPath,0,0,1,1);
  workingPathLayout->addWidget(browseWorkingPathButton,0,1,1,1);
  workingPathLayout->addWidget(m_workingPathValid,1,0,2,1);
  workingPathGroupBox->setLayout(workingPathLayout);

  // check application
  QGroupBox *checkApplicationGroupBox
    = new QGroupBox(tr("Check application dependencies"));

  QPushButton *checkApplicationButton = new QPushButton(tr("Check"));
  connect(checkApplicationButton, SIGNAL(clicked()),
          this, SLOT(checkApplication()));
  m_lilypondLabel = new QLabel(tr("<a href=\"http://lilypond.org/\">lilypond</a>: <font color=orange>%1</font>"));
  m_gitLabel = new QLabel(tr("<a href=\"http://git-scm.com/\">git</a>: <font color=orange>%1</font>"));

  QBoxLayout *checkApplicationLayout = new QVBoxLayout;
  checkApplicationLayout->addWidget(m_lilypondLabel);
  checkApplicationLayout->addWidget(m_gitLabel);
  checkApplicationLayout->addWidget(checkApplicationButton);
  checkApplicationGroupBox->setLayout(checkApplicationLayout);

  // main layout
  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(workingPathGroupBox);
  mainLayout->addWidget(checkApplicationGroupBox);
  mainLayout->addStretch(1);
  setLayout(mainLayout);

  checkApplication();
}

void OptionsPage::browse()
{
  QString path = QFileDialog::getExistingDirectory(this,
                                                   tr("Songbook path"),
                                                   m_workingPath->text());

  if (!path.isEmpty())
    {
      m_workingPath->setText(path);
    }
}

void OptionsPage::readSettings()
{
  QSettings settings;
  m_workingPath->setText(settings.value("workingPath", QString("%1/songbook/").arg(QDir::home().path())).toString());
}

void OptionsPage::writeSettings()
{
  QSettings settings;
  settings.setValue("workingPath", m_workingPath->text());
}

void OptionsPage::closeEvent(QCloseEvent *event)
{
  writeSettings();
  event->accept();
}

void OptionsPage::checkWorkingPath(const QString &path)
{
  QDir directory(path);

  bool error = true;
  bool warning = true;

  QString message;

  if (!directory.exists())
    {
      message = tr("the directory does not exist");
    }
  else if (!directory.exists("makefile"))
    {
      message = tr("makefile not found");
    }
  else if (!directory.exists("songbook.py"))
    {
      message = tr("songbook software (songbook.py) not found");
    }
  else if (!directory.exists("songs"))
    {
      message = tr("songs/ directory not found");
    }
  else if (!directory.exists("img"))
    {
      message = tr("img/ directory not found");
    }
  else if (!directory.exists("lilypond"))
    {
      error = false;
      message = tr("lilypond/ directory not found");
    }
  else if (!directory.exists("utils"))
    {
      error = false;
      message = tr("utils/ directory not found");
    }
  else
    {
      error = false;
      warning = false;
      message = tr("The directory is valid");
    }
  
  QString mask("<font color=%1>%2%3.</font>");
  if (error)
    {
      mask = mask.arg("red").arg("Error: ");
    }
  else if (warning)
    {
      mask = mask.arg("orange").arg("Warning: ");
    }
  else
    {
      mask = mask.arg("green").arg("");
    }
  m_workingPathValid->setText(mask.arg(message));
}


void OptionsPage::checkApplication()
{
  QProcess *process;

  process = new QProcess(m_gitLabel);
  connect(process, SIGNAL(error(QProcess::ProcessError)),
          this, SLOT(processError(QProcess::ProcessError)));

  process->start("git", QStringList() << "--version");
  if (process->waitForFinished())
    {
      QRegExp rx("git version ([^\n]+)");
      rx.indexIn(process->readAllStandardOutput().data());
      m_gitLabel->setText(m_gitLabel->text().replace("orange","green").arg(rx.cap(1)));
    }

  process = new QProcess(m_lilypondLabel);
  connect(process, SIGNAL(error(QProcess::ProcessError)),
          this, SLOT(processError(QProcess::ProcessError)));

  process->start("lilypond", QStringList() << "--version");
  if (process->waitForFinished())
    {
      QRegExp rx("GNU LilyPond ([^\n]+)");
      rx.indexIn(process->readAllStandardOutput().data());
      m_lilypondLabel->setText(m_lilypondLabel->text().replace("orange","green").arg(rx.cap(1)));
    }
}

void OptionsPage::processError(QProcess::ProcessError error)
{
  QProcess *process = qobject_cast< QProcess* >(QObject::sender());
  if (process)
    {
      QLabel *label =  qobject_cast< QLabel* >(process->parent());
      if (label)
        label->setText(label->text().arg("not found"));
    }
}
