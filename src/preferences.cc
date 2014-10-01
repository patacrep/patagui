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

#include <QBoxLayout>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFontDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QSettings>
#include <QSpinBox>
#include <QStackedWidget>

#ifdef ENABLE_LIBRARY_DOWNLOAD
#include <QNetworkProxy>
#endif // ENABLE_LIBRARY_DOWNLOAD

#include <QtGroupBoxPropertyBrowser>

#include "main-window.hh"
#include "songbook.hh"
#include "library.hh"
#include "file-chooser.hh"

#include <QDebug>

// Config Dialog

ConfigDialog::ConfigDialog(QWidget* parent)
    : QDialog(parent)
{
    m_contentsWidget = new QListWidget(this);
    m_contentsWidget->setViewMode(QListView::IconMode);
    m_contentsWidget->setIconSize(QSize(62, 62));
    m_contentsWidget->setMovement(QListView::Static);
    m_contentsWidget->setSpacing(12);
    m_contentsWidget->setFixedWidth(110);

    m_pagesWidget = new QStackedWidget(this);
    m_pagesWidget->addWidget(new OptionsPage(this));
    m_pagesWidget->addWidget(new SongbookPage(this));
    m_pagesWidget->addWidget(new DisplayPage(this));
    m_pagesWidget->addWidget(new EditorPage(this));
#ifdef ENABLE_LIBRARY_DOWNLOAD
    m_pagesWidget->addWidget(new NetworkPage(this));
#endif // ENABLE_LIBRARY_DOWNLOAD

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttons, SIGNAL(rejected()), this, SLOT(close()));

    createIcons();
    m_contentsWidget->setCurrentRow(0);

    QBoxLayout *horizontalLayout = new QHBoxLayout;
    horizontalLayout->addWidget(m_contentsWidget);
    horizontalLayout->addWidget(m_pagesWidget, 1);

    QBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(horizontalLayout);
    mainLayout->addSpacing(12);
    mainLayout->addWidget(buttons);

    setLayout(mainLayout);
    setWindowTitle(tr("Preferences"));
    resize(600,600);
}

CMainWindow* ConfigDialog::parent() const
{
    CMainWindow *p = qobject_cast<CMainWindow*>(QDialog::parent());
    if (!p) qWarning() << tr("ConfigDialog::parent() invalid parent");
    return p;
}

void ConfigDialog::createIcons()
{
    QListWidgetItem *optionsButton = new QListWidgetItem(m_contentsWidget);
    optionsButton->setIcon(QIcon::fromTheme("preferences-system", QIcon(":/icons/tango/48x48/categories/preferences-system.png")));
    optionsButton->setText(tr("Options"));
    optionsButton->setTextAlignment(Qt::AlignHCenter);
    optionsButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *songbookButton = new QListWidgetItem(m_contentsWidget);
    songbookButton->setIcon(QIcon(":/icons/songbook/256x256/book.png"));
    songbookButton->setText(tr("Songbook"));
    songbookButton->setTextAlignment(Qt::AlignHCenter);
    songbookButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *displayButton = new QListWidgetItem(m_contentsWidget);
    displayButton->setIcon(QIcon::fromTheme("preferences-desktop", QIcon(":/icons/tango/48x48/categories/preferences-desktop.png")));
    displayButton->setText(tr("Display"));
    displayButton->setTextAlignment(Qt::AlignHCenter);
    displayButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *editorButton = new QListWidgetItem(m_contentsWidget);
    editorButton->setIcon(QIcon::fromTheme("accessories-text-editor", QIcon(":/icons/tango/48x48/apps/accessories-text-editor.png")));
    editorButton->setText(tr("Editor"));
    editorButton->setTextAlignment(Qt::AlignHCenter);
    editorButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

#ifdef ENABLE_LIBRARY_DOWNLOAD
    QListWidgetItem *networkButton = new QListWidgetItem(m_contentsWidget);
    networkButton->setIcon(QIcon::fromTheme("preferences-system-network", QIcon(":/icons/tango/48x48/categories/preferences-system-network.png")));
    networkButton->setText(tr("Network"));
    networkButton->setTextAlignment(Qt::AlignHCenter);
    networkButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
#endif // ENABLE_LIBRARY_DOWNLOAD

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
    Q_UNUSED(event);
    for ( int i = 0 ; i < m_pagesWidget->count() ; ++i )
    {
        m_pagesWidget->widget(i)->close();
    }
}


// Page

Page::Page(QWidget *parent)
    : QScrollArea(parent)
    , m_content(new QWidget)
{
}

ConfigDialog * Page::parent() const
{
    ConfigDialog *p = qobject_cast<ConfigDialog*>(QScrollArea::parent());
    if (!p) qWarning() << tr("Page::parent() invalid parent");
    return p;
}

void Page::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void Page::readSettings()
{}

void Page::writeSettings()
{}

void Page::setLayout(QLayout *layout)
{
    m_content->setLayout(layout);
    setWidget(m_content);
}


// Display Page

DisplayPage::DisplayPage(QWidget *parent)
    : Page(parent)
{
    QGroupBox *displayApplicationGroupBox = new QGroupBox(tr("Application"));
    m_statusBarCheckBox = new QCheckBox(tr("Status bar"));
    m_toolBarCheckBox = new QCheckBox(tr("Tool bar"));
    m_compilationLogCheckBox = new QCheckBox(tr("Compilation log"));

    QGroupBox *displayColumnsGroupBox = new QGroupBox(tr("Library"));
    m_titleCheckBox = new QCheckBox(tr("Title"));
    m_artistCheckBox = new QCheckBox(tr("Artist"));
    m_pathCheckBox = new QCheckBox(tr("Path"));
    m_albumCheckBox = new QCheckBox(tr("Album"));
    m_lilypondCheckBox = new QCheckBox(tr("Lilypond"));
    m_websiteCheckBox = new QCheckBox(tr("Artist website"));
    m_langCheckBox = new QCheckBox(tr("Language"));

    QVBoxLayout *displayApplicationLayout = new QVBoxLayout;
    displayApplicationLayout->addWidget(m_statusBarCheckBox);
    displayApplicationLayout->addWidget(m_toolBarCheckBox);
    displayApplicationLayout->addWidget(m_compilationLogCheckBox);
    displayApplicationGroupBox->setLayout(displayApplicationLayout);

    QVBoxLayout *displayColumnsLayout = new QVBoxLayout;
    displayColumnsLayout->addWidget(m_titleCheckBox);
    displayColumnsLayout->addWidget(m_artistCheckBox);
    displayColumnsLayout->addWidget(m_pathCheckBox);
    displayColumnsLayout->addWidget(m_albumCheckBox);
    displayColumnsLayout->addWidget(m_lilypondCheckBox);
    displayColumnsLayout->addWidget(m_websiteCheckBox);
    displayColumnsLayout->addWidget(m_langCheckBox);
    displayColumnsGroupBox->setLayout(displayColumnsLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(displayApplicationGroupBox);
    mainLayout->addWidget(displayColumnsGroupBox);
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
    m_websiteCheckBox->setChecked(settings.value("website", false).toBool());
    m_langCheckBox->setChecked(settings.value("lang", true).toBool());
    m_compilationLogCheckBox->setChecked(settings.value("logs", false).toBool());
    m_statusBarCheckBox->setChecked(settings.value("statusBar", true).toBool());
    m_toolBarCheckBox->setChecked(settings.value("toolBar", true).toBool());
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
    settings.setValue("website", m_websiteCheckBox->isChecked());
    settings.setValue("lang", m_langCheckBox->isChecked());
    settings.setValue("logs", m_compilationLogCheckBox->isChecked());
    settings.setValue("statusBar", m_statusBarCheckBox->isChecked());
    settings.setValue("toolBar", m_toolBarCheckBox->isChecked());
    settings.endGroup();
}


// Option Page

OptionsPage::OptionsPage(QWidget *parent)
    : Page(parent)
    , m_songbookPath(0)
    , m_songbookPathValid(new QLabel)
    , m_libraryPath(0)
    , m_libraryPathValid(new QLabel)
    , m_buildCommand(0)
    , m_cleanCommand(0)
    , m_cleanallCommand(0)
{
    m_songbookPath = new CFileChooser();
    m_songbookPath->setMinimumWidth(300);
    m_songbookPath->setOptions(QFileDialog::ShowDirsOnly);
    m_songbookPath->setCaption(tr("Songbook path"));

    m_libraryPath = new CFileChooser();
    m_libraryPath->setMinimumWidth(300);
    m_libraryPath->setOptions(QFileDialog::ShowDirsOnly);
    m_libraryPath->setCaption(tr("Library path"));

    connect(m_songbookPath, SIGNAL(pathChanged(const QString&)),
            this, SLOT(checkSongbookPath(const QString&)));

    connect(m_libraryPath, SIGNAL(pathChanged(const QString&)),
            this, SLOT(checkLibraryPath(const QString&)));

    m_buildCommand = new QLineEdit(this);
    m_cleanCommand = new QLineEdit(this);
    m_cleanallCommand = new QLineEdit(this);

    readSettings();

    // Global paths
    QGroupBox *pathGroupBox
            = new QGroupBox(tr("Directories"));

    checkSongbookPath(m_songbookPath->path());
    checkLibraryPath(m_libraryPath->path());

    QFormLayout *pathLayout = new QFormLayout;
    pathLayout->addRow(tr("Songbook:"), m_songbookPath);
    pathLayout->addRow(m_songbookPathValid);
    pathLayout->addRow(tr("Library:"), m_libraryPath);
    pathLayout->addRow(m_libraryPathValid);
    pathGroupBox->setLayout(pathLayout);

    // external tools
    QGroupBox *toolsGroupBox
            = new QGroupBox(tr("Songbook commands"));

    QFormLayout *toolsLayout = new QFormLayout;
    toolsLayout->addRow(tr("Build:"), m_buildCommand);
    toolsLayout->addRow(tr("Clean:"), m_cleanCommand);
    toolsLayout->addRow(tr("Cleanall:"), m_cleanallCommand);

    QPushButton *resetButton = new QPushButton(tr("Reset to defaults"), this);
    connect(resetButton, SIGNAL(clicked()), SLOT(resetCommands()));
    toolsLayout->addRow(resetButton);

    toolsGroupBox->setLayout(toolsLayout);

    // main layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(pathGroupBox);
    mainLayout->addWidget(toolsGroupBox);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
}

void OptionsPage::readSettings()
{
    QSettings settings;
    settings.beginGroup("general");
    m_songbookPath->setPath(settings.value("songbookPath", QDir::homePath()).toString());
    m_libraryPath->setPath(settings.value("libraryPath", m_songbookPath->path()).toString());
    settings.endGroup();

    settings.beginGroup("tools");
    m_buildCommand->setText(settings.value("buildCommand", PLATFORM_BUILD_COMMAND).toString());
    m_cleanCommand->setText(settings.value("cleanCommand", PLATFORM_CLEAN_COMMAND).toString());
    m_cleanallCommand->setText(settings.value("cleanallCommand", PLATFORM_CLEANALL_COMMAND).toString());
    settings.endGroup();
}

void OptionsPage::writeSettings()
{
    QSettings settings;
    settings.beginGroup("general");
    settings.setValue("songbookPath", m_songbookPath->path());
    settings.setValue("libraryPath", m_libraryPath->path());
    settings.endGroup();

    settings.beginGroup("tools");
    settings.setValue("buildCommand", m_buildCommand->text());
    settings.setValue("cleanCommand", m_cleanCommand->text());
    settings.setValue("cleanallCommand", m_cleanallCommand->text());
    settings.endGroup();
}

void OptionsPage::checkSongbookPath(const QString &path)
{
    QDir directory(path);

    bool error = true;
    bool warning = true;

    QString message;

    if (!directory.exists())
    {
        message = tr("the directory does not exist");
    }
    else if (!directory.exists("songbook"))
    {
        message = tr("songbook executable not found");
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
        mask = mask.arg("red").arg(tr("Error: "));
    }
    else if (warning)
    {
        mask = mask.arg("orange").arg(tr("Warning: "));
    }
    else
    {
        mask = mask.arg("green").arg("");
    }
    m_songbookPathValid->setText(mask.arg(message));
}

void OptionsPage::checkLibraryPath(const QString &path)
{
    m_libraryPathValid->setText(CLibrary::checkPath(path));
}

void OptionsPage::resetCommands()
{
    m_buildCommand->setText(PLATFORM_BUILD_COMMAND);
    m_cleanCommand->setText(PLATFORM_CLEAN_COMMAND);
    m_cleanallCommand->setText(PLATFORM_CLEANALL_COMMAND);
}

// Editor Page

EditorPage::EditorPage(QWidget *parent)
    : Page(parent)
    , m_numberLinesCheckBox(new QCheckBox(tr("Display line numbers")))
    , m_highlightCurrentLineCheckBox(new QCheckBox(tr("Highlight current line")))
    , m_colorEnvironmentsCheckBox(new QCheckBox(tr("Highlight environments")))
    , m_fontButton(new QPushButton)
{
    readSettings();

    if (m_fontstr.isEmpty())
    {
        m_font = QFont("Monospace",11);
        m_font.setStyleHint(QFont::TypeWriter, QFont::PreferAntialias);
    }

    updateFontButton();
    connect(m_fontButton, SIGNAL(clicked()), this, SLOT(selectFont()));

    QFormLayout* layout = new QFormLayout;
    layout->addRow(m_numberLinesCheckBox);
    layout->addRow(m_highlightCurrentLineCheckBox);
    layout->addRow(m_colorEnvironmentsCheckBox);
    layout->addRow(tr("Font:"), m_fontButton);

    setLayout(layout);
}

void EditorPage::readSettings()
{
    QSettings settings;
    settings.beginGroup("editor");
    m_numberLinesCheckBox->setChecked(settings.value("lines", true).toBool());
    m_highlightCurrentLineCheckBox->setChecked(settings.value("highlight", true).toBool());
    m_colorEnvironmentsCheckBox->setChecked(settings.value("color-environments", true).toBool());
    m_fontstr = settings.value("font", QString()).toString();
    if (!m_fontstr.isEmpty())
        m_font.fromString(m_fontstr);
    settings.endGroup();
}

void EditorPage::writeSettings()
{
    QSettings settings;
    settings.beginGroup("editor");
    settings.setValue("lines", m_numberLinesCheckBox->isChecked());
    settings.setValue("highlight", m_highlightCurrentLineCheckBox->isChecked());
    settings.setValue("color-environments", m_colorEnvironmentsCheckBox->isChecked());
    settings.setValue("font", m_font.toString());
    settings.endGroup();
}

void EditorPage::selectFont()
{
    bool ok;
    m_font = QFontDialog::getFont(&ok, m_font, this);
    if (ok) updateFontButton();
}

void EditorPage::updateFontButton()
{
    m_fontButton->setText(QString("%1 %2")
                          .arg(QFontInfo(m_font).family())
                          .arg(QString::number(QFontInfo(m_font).pointSize())));
}


#ifdef ENABLE_LIBRARY_DOWNLOAD
// Network Page

NetworkPage::NetworkPage(QWidget *parent)
    : Page(parent)
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
#endif // ENABLE_LIBRARY_DOWNLOAD

// Songbook Page

SongbookPage::SongbookPage(QWidget *p)
    : Page(p)
    , m_propertyEditor(new QtGroupBoxPropertyBrowser)
    , m_mainwindow(parent()->parent())
{
    if (!m_mainwindow)
    {
        qWarning() << tr("SongbookPage::SongbookPage invalid parent: can't find the mainwindow");
        return;
    }

    CSongbook *songbook = m_mainwindow->songbook();

    QComboBox* templateComboBox = new QComboBox;
    templateComboBox->addItems(songbook->library()->templates());

    int index = songbook->library()->templates().indexOf(songbook->tmpl());
    if (index == -1)
        index = songbook->library()->templates().indexOf("patacrep.tmpl");
    templateComboBox->setCurrentIndex(index);

    connect(templateComboBox, SIGNAL(currentIndexChanged(const QString &)),
            songbook, SLOT(setTmpl(const QString &)));
    connect(songbook, SIGNAL(wasModified(bool)), SLOT(updatePropertyEditor()));

    songbook->changeTemplate(songbook->tmpl());
    songbook->initializeEditor(m_propertyEditor);

    QGroupBox *songbookGroupBox = new QGroupBox(tr("Songbook"));

    QBoxLayout *templateLayout = new QHBoxLayout;
    templateLayout->addWidget(new QLabel(tr("Template:")));
    templateLayout->addWidget(templateComboBox);

    QBoxLayout *songbookLayout = new QVBoxLayout;
    songbookLayout->addLayout(templateLayout);
    songbookLayout->addWidget(m_propertyEditor);
    songbookGroupBox->setLayout(songbookLayout);

    // main layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(songbookGroupBox);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
}

void SongbookPage::updatePropertyEditor()
{
    if (m_mainwindow)
        m_mainwindow->songbook()->initializeEditor(m_propertyEditor);
    else
        qWarning() << tr("SongbookPage::updatePropertyEditor can't find the mainwindow");
}
