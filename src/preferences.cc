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
  // pagesWidget->addWidget(new SongbookAppearancePage);
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

  m_artistCheckBox = new QCheckBox(tr("Artist"));
  m_titleCheckBox = new QCheckBox(tr("Title"));
  m_pathCheckBox = new QCheckBox(tr("Path"));
  m_albumCheckBox = new QCheckBox(tr("Album"));
  m_lilypondCheckBox = new QCheckBox(tr("Lilypond"));
  m_coverCheckBox = new QCheckBox(tr("Cover"));

  QVBoxLayout *displayColumnsLayout = new QVBoxLayout;
  displayColumnsLayout->addWidget(m_artistCheckBox);
  displayColumnsLayout->addWidget(m_titleCheckBox);
  displayColumnsLayout->addWidget(m_pathCheckBox);
  displayColumnsLayout->addWidget(m_albumCheckBox);
  displayColumnsLayout->addWidget(m_lilypondCheckBox);
  displayColumnsLayout->addWidget(m_coverCheckBox);
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
  m_artistCheckBox->setChecked(settings.value("artist", true).toBool());
  m_titleCheckBox->setChecked(settings.value("title", true).toBool());
  m_pathCheckBox->setChecked(settings.value("path", false).toBool());
  m_albumCheckBox->setChecked(settings.value("album", true).toBool());
  m_lilypondCheckBox->setChecked(settings.value("lilypond", false).toBool());
  m_coverCheckBox->setChecked(settings.value("cover", true).toBool());
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
  m_workingPath = new QLineEdit(QString());
  connect(m_workingPath, SIGNAL(textChanged(const QString&)), this, SLOT(checkWorkingPath(const QString&)));
  m_workingPathValid = new QLabel;
  checkWorkingPath(workingDir);

  QGridLayout *workingPathLayout = new QGridLayout;
  workingPathLayout->addWidget(m_workingPath,0,0,1,1);
  workingPathLayout->addWidget(browseWorkingPathButton,0,1,1,1);
  workingPathLayout->addWidget(m_workingPathValid,1,0,2,1);
  workingPathGroupBox->setLayout(workingPathLayout);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(workingPathGroupBox);
  mainLayout->addStretch(1);
  setLayout(mainLayout);

  readSettings();
}

void OptionsPage::browse()
{
  QString directory = QFileDialog::getExistingDirectory(this,
                                                        tr("Find Files"),
                                                        m_workingPath->text());

  if (!directory.isEmpty())
    {
      m_workingPath->setText(directory);
      checkWorkingPath(directory);
    }
}

void OptionsPage::readSettings()
{
  QSettings settings;
  m_workingPath->setText(settings.value("workingPath", QString("%1/").arg(QDir::currentPath())).toString());
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

void OptionsPage::checkWorkingPath(const QString & path)
{
  m_isValid = false;
  QDir directory(path);
  if (!directory.exists())
    {
      m_workingPathValid->setText("<font color=red>Invalid songbook directory: directory does not exist.</font>");
      return;
    }

  if (!directory.entryList(QDir::Files | QDir::Readable).contains("makefile"))
    {
      m_workingPathValid->setText("<font color=red>Invalid songbook directory: makefile not found.</font>");
      return;
    }

  if (!directory.entryList(QDir::Files | QDir::Readable).contains("mybook.tex"))
    {
      m_workingPathValid->setText("<font color=red>Invalid songbook directory: mybook.tex not found.</font>");
      return;
    }

  // subdirectories
  QDir songs( QString("%1/songs").arg(path) );
  QDir utils( QString("%1/utils").arg(path) );
  QDir lilypond( QString("%1/lilypond").arg(path) );
  QDir img( QString("%1/img").arg(path) );

  if (!songs.exists())
    {
      m_workingPathValid->setText("<font color=red>Invalid songbook directory: subdirectory songs/ not found.</font>");
      return;
    }

  if (!img.exists())
    {
      m_workingPathValid->setText("<font color=red>Invalid songbook directory: subdirectory img/ not found.</font>");
      return;
    }

  m_isValid = true;

  if (!lilypond.exists())
    {
      m_workingPathValid->setText("<font color=orange>Warning: subdirectory lilypond/ not found.</font>");
      return;
    }

  if (!utils.exists())
    {
      m_workingPathValid->setText("<font color=orange>Warning: subdirectory utils/ not found.</font>");
      return;
    }

  m_workingPathValid->setText("<font color=green>The songbook directory is valid.</font>");
}

void OptionsPage::checkLilypondVersion(int AState)
{
  if (AState==Qt::Checked)
    {
      m_lilypondCheck = new QProcess(this);
      connect(m_lilypondCheck, SIGNAL(error(QProcess::ProcessError)),
	      this, SLOT(processError(QProcess::ProcessError)));

      QStringList argsLily;
      argsLily << "--version";
      m_lilypondCheck->start("lilypond", argsLily);
      m_lilypondCheck->waitForFinished();
      QRegExp rx("GNU([^\n]+)");
      rx.indexIn(m_lilypondCheck->readAllStandardOutput().data());
      m_lilypondLabel->setText(QString("<font color=green>Found:%1</font>").arg(rx.cap(1)));
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

void OptionsPage::lyricBookMode(bool tmp)
{
  QObject *object = QObject::sender();
  if (QRadioButton* lyricBook = qobject_cast< QRadioButton* >(object))
    {
      // m_diagramCheckBox->setEnabled(!lyricBook->isChecked());
      // m_lilypondCheckBox->setEnabled(!lyricBook->isChecked());
      // m_tablatureCheckBox->setEnabled(!lyricBook->isChecked());
    }
}
