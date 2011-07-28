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

#include "preferences.hh"
#include "file-chooser.hh"
#include "songbook-panel.hh"
#include "main-window.hh"

#include <QtGui>

#include <QNetworkProxy>

// Config Dialog

ConfigDialog::ConfigDialog(CMainWindow* parent)
  : QDialog(parent)
  , m_parent(parent)
{
  m_contentsWidget = new QListWidget;
  m_contentsWidget->setViewMode(QListView::IconMode);
  m_contentsWidget->setIconSize(QSize(62, 62));
  m_contentsWidget->setMovement(QListView::Static);
  m_contentsWidget->setMaximumWidth(110);
  m_contentsWidget->setSpacing(12);
  m_contentsWidget->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::MinimumExpanding);

  m_pagesWidget = new QStackedWidget;
  m_pagesWidget->addWidget(new OptionsPage(this));
  m_pagesWidget->addWidget(new DisplayPage(this));
  m_pagesWidget->addWidget(new EditorPage(this));
  m_pagesWidget->addWidget(new NetworkPage(this));

  QPushButton *closeButton = new QPushButton(tr("Close"));

  createIcons();
  m_contentsWidget->setCurrentRow(0);

  connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

  QHBoxLayout *horizontalLayout = new QHBoxLayout;
  horizontalLayout->addWidget(m_contentsWidget);
  horizontalLayout->addWidget(m_pagesWidget, 1);

  QHBoxLayout *buttonsLayout = new QHBoxLayout;
  buttonsLayout->addStretch(1);
  buttonsLayout->addWidget(closeButton);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addLayout(horizontalLayout);
  mainLayout->addSpacing(12);
  mainLayout->addLayout(buttonsLayout);
  setLayout(mainLayout);

  setWindowTitle(tr("Preferences"));
}

CMainWindow* ConfigDialog::parent() const
{
  return m_parent;
}

void ConfigDialog::createIcons()
{
  QListWidgetItem *optionsButton = new QListWidgetItem(m_contentsWidget);
  optionsButton->setIcon(QIcon::fromTheme("preferences-system", QIcon(":/icons/tango/preferences-system")));
  optionsButton->setText(tr("Options"));
  optionsButton->setTextAlignment(Qt::AlignHCenter);
  optionsButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

  QListWidgetItem *displayButton = new QListWidgetItem(m_contentsWidget);
  displayButton->setIcon(QIcon::fromTheme("preferences-desktop-theme", QIcon(":/icons/tango/preferences-desktop-theme")));
  displayButton->setText(tr("Display"));
  displayButton->setTextAlignment(Qt::AlignHCenter);
  displayButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

  QListWidgetItem *editorButton = new QListWidgetItem(m_contentsWidget);
  editorButton->setIcon(QIcon::fromTheme("accessories-text-editor"));
  editorButton->setText(tr("Editor"));
  editorButton->setTextAlignment(Qt::AlignHCenter);
  editorButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

  QListWidgetItem *networkButton = new QListWidgetItem(m_contentsWidget);
  networkButton->setIcon(QIcon::fromTheme("preferences-system-network", QIcon(":/icons/tango/preferences-system-network")));
  networkButton->setText(tr("Network"));
  networkButton->setTextAlignment(Qt::AlignHCenter);
  networkButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

  connect(m_contentsWidget,
          SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
          this, SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));
}

void ConfigDialog::changePage(QListWidgetItem *current,
                              QListWidgetItem *previous)
{
  if (!current)
    current = previous;

  m_pagesWidget->setCurrentIndex(m_contentsWidget->row(current));
}

void ConfigDialog::closeEvent(QCloseEvent *event)
{
  for( int i = 0 ; i < m_pagesWidget->count() ; ++i )
    {
      m_pagesWidget->widget(i)->close();
    }
}

// Display Page

DisplayPage::DisplayPage(QWidget *parent)
  : QWidget(parent)
{
  QGroupBox *displayColumnsGroupBox = new QGroupBox(tr("Display Columns"));

  m_titleCheckBox = new QCheckBox(tr("Title"));
  m_artistCheckBox = new QCheckBox(tr("Artist"));
  m_pathCheckBox = new QCheckBox(tr("Path"));
  m_albumCheckBox = new QCheckBox(tr("Album"));
  m_lilypondCheckBox = new QCheckBox(tr("Lilypond"));
  m_langCheckBox = new QCheckBox(tr("Language"));

  QVBoxLayout *displayColumnsLayout = new QVBoxLayout;
  displayColumnsLayout->addWidget(m_titleCheckBox);
  displayColumnsLayout->addWidget(m_artistCheckBox);
  displayColumnsLayout->addWidget(m_pathCheckBox);
  displayColumnsLayout->addWidget(m_albumCheckBox);
  displayColumnsLayout->addWidget(m_lilypondCheckBox);
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
  m_titleCheckBox->setChecked(settings.value("title", true).toBool());
  m_artistCheckBox->setChecked(settings.value("artist", true).toBool());
  m_pathCheckBox->setChecked(settings.value("path", false).toBool());
  m_albumCheckBox->setChecked(settings.value("album", true).toBool());
  m_lilypondCheckBox->setChecked(settings.value("lilypond", false).toBool());
  m_langCheckBox->setChecked(settings.value("lang", false).toBool());
  m_compilationLogCheckBox->setChecked(settings.value("logs", false).toBool());
  settings.endGroup();
}

void DisplayPage::writeSettings()
{
  QSettings settings;
  settings.beginGroup("display");
  settings.setValue("title", m_titleCheckBox->isChecked());
  settings.setValue("artist", m_artistCheckBox->isChecked());
  settings.setValue("path", m_pathCheckBox->isChecked());
  settings.setValue("album", m_albumCheckBox->isChecked());
  settings.setValue("lilypond", m_lilypondCheckBox->isChecked());
  settings.setValue("lang", m_langCheckBox->isChecked());
  settings.setValue("logs", m_compilationLogCheckBox->isChecked());
  settings.endGroup();
}

void DisplayPage::closeEvent(QCloseEvent *event)
{
  writeSettings();
  event->accept();
}

// Option Page

OptionsPage::OptionsPage(ConfigDialog *p)
  : QWidget(p)
  , m_parent(p)
  , m_workingPath(0)
  , m_workingPathValid(0)
{
  m_workingPathValid = new QLabel;

  m_workingPath = new CFileChooser();
  m_workingPath->setFileMode(QFileDialog::Directory);
  m_workingPath->setOptions(QFileDialog::ShowDirsOnly);
  m_workingPath->setCaption(tr("Songbook path"));

  connect(m_workingPath, SIGNAL(pathChanged(const QString&)),
          this, SLOT(checkWorkingPath(const QString&)));

  readSettings();

  // working path
  QGroupBox *workingPathGroupBox
    = new QGroupBox(tr("Directory for Patacrep Songbook"));

  checkWorkingPath(m_workingPath->path());

  QLayout *workingPathLayout = new QVBoxLayout;
  workingPathLayout->addWidget(m_workingPath);
  workingPathLayout->addWidget(m_workingPathValid);
  workingPathGroupBox->setLayout(workingPathLayout);

  // songbook template
  QWidget* sbSettings = new QWidget;
  
  CSongbookPanel* panel = new CSongbookPanel(parent()->parent()->songbook());
  
  QDialogButtonBox * buttons = new QDialogButtonBox;
  buttons->addButton(new QPushButton(tr("Change settings")), QDialogButtonBox::AcceptRole);
  connect(buttons, SIGNAL(accepted()), panel, SLOT(settingsDialog()));

  QVBoxLayout * layout = new QVBoxLayout;
  layout->addWidget(panel);
  layout->addWidget(buttons);
  sbSettings->setLayout(layout);

  QGroupBox *songbookTemplateGroupBox
    = new QGroupBox(tr("Songbook PDF"));

  QLayout *songbookTemplateLayout = new QVBoxLayout;
  songbookTemplateLayout->addWidget(sbSettings);
  songbookTemplateGroupBox->setLayout(songbookTemplateLayout);

  // main layout
  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(workingPathGroupBox);
  mainLayout->addWidget(songbookTemplateGroupBox);
  mainLayout->addStretch(1);
  setLayout(mainLayout);
}

ConfigDialog* OptionsPage::parent() const
{
  return m_parent;
}

void OptionsPage::readSettings()
{
  QSettings settings;
  settings.beginGroup("library");
  m_workingPath->setPath(settings.value("workingPath", QDir::home().path()).toString());
  settings.endGroup();
}

void OptionsPage::writeSettings()
{
  QSettings settings;
  settings.beginGroup("library");
  settings.setValue("workingPath", m_workingPath->path());
  settings.endGroup();
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


// Editor Page

EditorPage::EditorPage(QWidget *parent)
  : QWidget(parent)
{
  m_font = QFont("Monospace",10);
  m_font.setStyleHint(QFont::TypeWriter, QFont::PreferAntialias);

  m_numberLinesCheckBox = new QCheckBox(tr("Display line numbers"));
  m_highlightCurrentLineCheckBox = new QCheckBox(tr("Highlight current line"));
  m_fontButton  = new QPushButton(QString("%1 %2").arg(m_font.family()).arg(QString::number(m_font.pointSize())), this);
  connect(m_fontButton, SIGNAL(clicked()), this, SLOT(selectFont()));

  QFormLayout* layout = new QFormLayout;
  layout->addRow(m_numberLinesCheckBox);
  layout->addRow(m_highlightCurrentLineCheckBox);
  layout->addRow(tr("Editor font:"), m_fontButton);
  setLayout(layout);

  readSettings();
}

void EditorPage::readSettings()
{
  QSettings settings;
  settings.beginGroup("editor");
  m_numberLinesCheckBox->setChecked(settings.value("lines", true).toBool());
  m_highlightCurrentLineCheckBox->setChecked(settings.value("highlight", true).toBool());
  m_font.fromString(settings.value("font", QString()).toString());
  settings.endGroup();
}

void EditorPage::writeSettings()
{
  QSettings settings;
  settings.beginGroup("editor");
  settings.value("lines", m_numberLinesCheckBox->isChecked());
  settings.value("highlight", m_highlightCurrentLineCheckBox->isChecked());
  settings.value("font", m_font.toString());
  settings.endGroup();
}

void EditorPage::selectFont()
{
  bool ok;
  m_font = QFontDialog::getFont(&ok, QFont("Monospace", 10), this);
  if(ok)
    {
      m_fontButton->setText(QString("%1 %2").arg(m_font.family()).arg(QString::number(m_font.pointSize())));
    }
}

void EditorPage::closeEvent(QCloseEvent *event)
{
  writeSettings();
  event->accept();
}

ConfigDialog* EditorPage::parent() const
{
  return m_parent;
}


// Network Page

NetworkPage::NetworkPage(QWidget *parent)
  : QWidget(parent)
  , m_hostname()
  , m_port()
  , m_user()
  , m_password()
{
  m_hostname = new QLineEdit;
  m_port = new QSpinBox;
  m_port->setRange(0, 65535);
  m_user = new QLineEdit;
  m_password = new QLineEdit;
  m_password->setEchoMode(QLineEdit::Password);

  readSettings();

  // check application
  QGroupBox *proxyGroupBox
    = new QGroupBox(tr("Proxy settings"));

  QFormLayout *proxyLayout = new QFormLayout;
  proxyLayout->addRow(tr("Hostname:"), m_hostname);
  proxyLayout->addRow(tr("Port:"), m_port);
  proxyLayout->addRow(tr("User:"), m_user);
  proxyLayout->addRow(tr("Password:"), m_password);
  proxyGroupBox->setLayout(proxyLayout);

  // main layout
  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(proxyGroupBox);
  mainLayout->addStretch(1);
  setLayout(mainLayout);
}

void NetworkPage::readSettings()
{
  QSettings settings;
  settings.beginGroup("proxy");
  m_hostname->setText(settings.value("hostname", QString()).toString());
  m_port->setValue(settings.value("port", 0).toInt());
  m_user->setText(settings.value("user", QString()).toString());
  m_password->setText(settings.value("password", QString()).toString());
  settings.endGroup();
}

void NetworkPage::writeSettings()
{
  QSettings settings;
  settings.beginGroup("proxy");
  settings.setValue("hostname", m_hostname->text());
  settings.setValue("port", m_port->value());
  settings.setValue("user", m_user->text());
  settings.setValue("password", m_password->text());
  settings.endGroup();

  QNetworkProxy proxy;
  if (m_hostname->text().isEmpty())
    {
      proxy.setType(QNetworkProxy::NoProxy);
    }
  else
    {
      proxy.setType(QNetworkProxy::HttpProxy);
      proxy.setHostName(m_hostname->text());
      proxy.setPort(m_port->value());
      proxy.setUser(m_user->text());
      proxy.setPassword(m_password->text());
    }
  QNetworkProxy::setApplicationProxy(proxy);
}

void NetworkPage::closeEvent(QCloseEvent *event)
{
  writeSettings();
  event->accept();
}

