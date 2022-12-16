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
#include "main-window.hh"

#include <QApplication>
#include <QAction>
#include <QBoxLayout>
#include <QCheckBox>
#include <QCloseEvent>
#include <QCompleter>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QStandardPaths>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QListView>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QSettings>
#include <QStatusBar>
#include <QToolBar>
#include <QtConcurrent>
#include <QFuture>

#include "label.hh"
#include "library.hh"
#include "library-view.hh"
#include "songbook.hh"
#include "song-editor.hh"
#include "song-highlighter.hh"
#include "logs-highlighter.hh"
#include "filter-lineedit.hh"
#include "song-sort-filter-proxy-model.hh"
#include "tab-widget.hh"
#include "notification.hh"
#include "song-item-delegate.hh"
#include "preferences.hh"
#include "progress-bar.hh"
#include "import-dialog.hh"
#include "patacrep.hh"

#include <QDebug>
#include <QMetaMethod>

namespace // anonymous namespace
{
bool checkPdfLaTeX()
{
    QString message;
    QProcess process;
    process.start("pdflatex", QStringList() << "--version");
    if (!process.waitForFinished()) {
        QString platformSpecificMessage;
#if defined(Q_OS_WIN32)
        platformSpecificMessage = QObject::tr(
            "<ol><li>Download and install the <a "
            "href=\"http://miktex.org\"/>MikTeX</a> distribution for "
            "Windows.</li>"
            "<li>Verify that your PATH variable is correctly set.</li></ol>");
#elif defined(Q_OS_MAC)
        platformSpecificMessage =
            QObject::tr("<p>Download and install the <a "
                        "href=\"http://www.tug.org/mactex\">MacTeX</a> "
                        "distribution for Mac OS.</p>");
#else // Unix/Linux
        platformSpecificMessage =
            QObject::tr("<p>Download and install the following packages:</p>"
                        "<ol><li>texlive-base</li>"
                        "<li>texlive-latex-extra</li>"
                        "<li>texlive-latex-extra</li>"
                        "<li>texlive-fonts-extra</li>"
                        "<li>texlive-lang-french</li>"
                        "<li>texlive-lang-english</li>"
                        "<li>texlive-lang-spanish</li>"
                        "<li>texlive-lang-italian</li>"
                        "<li>texlive-lang-portuguese</li>"
                        "</ol>");
#endif
        message =
            QObject::tr(
                "<p>The following program cannot be found: <i>pdflatex</i>.</p>"
                "<p>A <a href=\"www.latex-project.org/\">LaTeX</a> "
                "distribution supporting <i>pdflatex</i> is required "
                "to produce the PDF document. Such a distribution is either "
                "not installed or misconfigured.</p>"
                "%1"
                "You can find more information in the "
                "<a href=\"http://www.patacrep.fr/data/documents/doc_%2.pdf\">"
                "documentation</a>.\n")
                .arg(platformSpecificMessage)
                .arg((QLocale::system().language() == QLocale::French) ? "fr"
                                                                       : "en");
        QMessageBox::warning(0, QObject::tr("Missing program"), message);
        return false;
    }
    return true;
}

bool checkPython()
{
    QString message;
    QProcess process;
    process.start("python", QStringList() << "--version");
    if (!process.waitForFinished()) {
        QString platformSpecificMessage;
#if defined(Q_OS_WIN32)
        platformSpecificMessage = QObject::tr(
            "<ol><li>Download and install <a "
            "href=\"http://www.python.org/download\">Python 2.X</a> for "
            "Windows.</li>"
            "<li>Verify that your PATH variable is correctly set.</li></ol>");
#elif defined(Q_OS_MAC)
        platformSpecificMessage = QObject::tr(
            "<ol><li>Download and install <a "
            "href=\"http://www.python.org/download\">Python 2.X</a> for Mac "
            "OS.</li>"
            "<li>Verify that your PATH variable is correctly set.</li></ol>");
#else // Unix/Linux
        platformSpecificMessage =
            QObject::tr("<p>Download and install the following packages: "
                        "<i>python</i></p>");
#endif
        message =
            QObject::tr(
                "<p>The following program cannot be found: <i>python</i>.</p>"
                "<p>A version of <a href=\"www.python.org/\">Python 2</a> is "
                "required "
                "to produce the PDF document.</p>"
                "%1"
                "You can find more information in the "
                "<a href=\"http://www.patacrep.fr/data/documents/doc_%2.pdf\">"
                "documentation</a>.\n")
                .arg(platformSpecificMessage)
                .arg((QLocale::system().language() == QLocale::French) ? "fr"
                                                                       : "en");
        QMessageBox::warning(0, QObject::tr("Missing program"), message);
        return false;
    }
    return true;
}

bool removeDirectoryRecursively(const QDir &directory)
{
    bool error = false;
    if (directory.exists()) {
        QFileInfoList entries = directory.entryInfoList(
            QDir::NoDotAndDotDot | QDir::Hidden | QDir::NoSymLinks |
            QDir::Dirs | QDir::Files);

        int count = entries.size();
        for (int index = 0; index < count && !error; ++index) {
            QFileInfo entryInfo = entries[index];
            QString path = entryInfo.absoluteFilePath();
            if (entryInfo.isDir()) {
                error = removeDirectoryRecursively(QDir(path));
            } else {
                QFile file(path);
                if (!file.remove()) {
                    qWarning()
                        << QObject::tr(
                               "Can't remove file: %1 from cache directory")
                               .arg(path);
                    error = true;
                }
            }
        }
        if (!directory.rmdir(directory.absolutePath())) {
            qWarning() << QObject::tr(
                              "Can't remove directory: %1 from cache directory")
                              .arg(directory.absolutePath());
            error = true;
        }
    }
    return error;
}
}

const QString MainWindow::_cachePath(QString("%1/patagui").arg(
    QStandardPaths::writableLocation(QStandardPaths::CacheLocation)));

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_view(new LibraryView(this))
    , m_songbook(new Songbook(this))
    , m_proxyModel(new SongSortFilterProxyModel(this))
    , m_tempFilesmodel(0)
    , m_mainWidget(new TabWidget(this))
    , m_progressBar(new ProgressBar(this))
    , m_noDataInfo(0)
    , m_noDatadirSet(0)
    , m_updateAvailable(0)
    , m_infoSelection(new QLabel(this))
    , m_log(new QDockWidget(tr("LaTeX compilation logs")))
    , m_isToolBarDisplayed(true)
    , m_isStatusBarDisplayed(true)
    , m_currentToolBar(0)
    , patacrep(new Patacrep(this))
    , m_songHighlighter(0)
{
    setWindowTitle("Patagui");
    setWindowIcon(QIcon(":/icons/songbook/256x256/patagui.png"));
    Library::instance()->setParent(this);

    connect(library(), SIGNAL(directoryChanged(const QDir &)),
            SLOT(noDataNotification(const QDir &)));
    connect(library(), SIGNAL(noDirectory()),
            SLOT(noSongbookDirectoryNotification()));

    connect(m_songbook, SIGNAL(wasModified(bool)),
            SLOT(setWindowModified(bool)));
    connect(
        m_songbook,
        SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
        SLOT(selectedSongsChanged(const QModelIndex &, const QModelIndex &)));

    // proxy model (sorting & filtering)
    m_proxyModel->setSourceModel(m_songbook);
    m_proxyModel->setSortLocaleAware(true);
    m_proxyModel->setDynamicSortFilter(true);
    m_proxyModel->setFilterKeyColumn(-1);

    // view
    m_view->setModel(m_proxyModel);
    m_view->setItemDelegate(new SongItemDelegate);
    m_view->resizeColumns();
    connect(library(), SIGNAL(wasModified()), m_view, SLOT(update()));

    // compilation log
    QPlainTextEdit *logs = new QPlainTextEdit;
    logs->setReadOnly(true);
    Q_UNUSED(new LogsHighlighter(logs->document()));
    m_log->setWidget(logs);
    addDockWidget(Qt::BottomDockWidgetArea, m_log);

    createActions();
    createMenus();
    createToolBar();

    // place elements into the main window
    m_mainWidget->setTabsClosable(true);
    m_mainWidget->setMovable(true);
    m_mainWidget->setSelectionBehaviorOnAdd(TabWidget::SelectNew);
    connect(m_mainWidget, SIGNAL(tabCloseRequested(int)), SLOT(closeTab(int)));
    connect(m_mainWidget, SIGNAL(currentChanged(int)), SLOT(changeTab(int)));
    m_mainWidget->addTab(m_view, tr("Library"));
    setCentralWidget(m_mainWidget);

    m_progressBar->setTextVisible(false);
    m_progressBar->setRange(0, 0);
    m_progressBar->hide();
    connect(progressBar(), SIGNAL(canceled()), this, SLOT(cancelProcess()));

    // status bar with an embedded label and progress bar
    statusBar()->addPermanentWidget(m_infoSelection);
    statusBar()->addPermanentWidget(m_progressBar);

    // make/make clean/make cleanall process
    connect(patacrep, SIGNAL(aboutToStart()), progressBar(), SLOT(show()));
    connect(patacrep, SIGNAL(aboutToStart()), statusBar(),
            SLOT(clearMessage()));
    connect(patacrep, SIGNAL(message(const QString &, int)), statusBar(),
            SLOT(showMessage(const QString &, int)));
    connect(patacrep, SIGNAL(message(const QString &, int)), log()->widget(),
            SLOT(appendPlainText(const QString &)));
    connect(patacrep, SIGNAL(finished()), progressBar(), SLOT(hide()));
    //    connect(patacrep, SIGNAL(error(QProcess::ProcessError)),
    //            this, SLOT(buildError(QProcess::ProcessError)));
    updateTitle(songbook()->filename());

    // make a clean cache directory for the application
    removeDirectoryRecursively(QDir(_cachePath));
    QDir().mkpath(_cachePath);

    readSettings(true);
}

MainWindow::~MainWindow()
{
    delete m_songbook;
    delete m_voidEditor;
}

void MainWindow::switchToolBar(QToolBar *toolBar)
{
    if (toolBar != m_currentToolBar) {
        addToolBar(toolBar);
        toolBar->setVisible(isToolBarDisplayed());
        m_currentToolBar->setVisible(false);
        removeToolBar(m_currentToolBar);
        m_currentToolBar = toolBar;
    }
}

void MainWindow::readSettings(bool firstLaunch)
{
    QSettings settings;
    settings.beginGroup("global");
    if (firstLaunch) {
        resize(settings.value("size", QSize(800, 600)).toSize());
        move(settings.value("pos", QPoint(200, 200)).toPoint());
        if (settings.value("maximized", isMaximized()).toBool())
            showMaximized();
    }
    settings.endGroup();

    settings.beginGroup("display");
    setStatusBarDisplayed(settings.value("statusBar", true).toBool());
    setToolBarDisplayed(settings.value("toolBar", true).toBool());
    log()->setVisible(settings.value("logs", false).toBool());
    settings.endGroup();

    view()->readSettings();
    library()->readSettings();
}

void MainWindow::writeSettings()
{
    QSettings settings;
    settings.beginGroup("global");
    settings.setValue("maximized", isMaximized());
    if (!isMaximized()) {
        settings.setValue("pos", pos());
        settings.setValue("size", size());
    }
    settings.endGroup();

    library()->writeSettings();
    view()->writeSettings();
}

void MainWindow::selectedSongsChanged(const QModelIndex &, const QModelIndex &)
{
    m_infoSelection->setText(tr("Selection: %1/%2")
                                 .arg(songbook()->selectedCount())
                                 .arg(songbook()->rowCount()));
}

void MainWindow::createActions()
{
    m_newSongAct = new QAction(tr("&New Song"), this);
    m_newSongAct->setIcon(QIcon::fromTheme(
        "list-add", QIcon(":/icons/tango/32x32/actions/list-add.png")));
    m_newSongAct->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_N));
    m_newSongAct->setStatusTip(tr("Write a new song"));
    m_newSongAct->setIconText(tr("Write a new song"));
    connect(m_newSongAct, SIGNAL(triggered()), this, SLOT(newSong()));

    m_importSongsAct = new QAction(tr("&Import Songs"), this);
    m_importSongsAct->setIcon(QIcon::fromTheme(
        "document-import",
        QIcon(":/icons/tango/32x32/actions/document-import.png")));
    m_importSongsAct->setShortcut(
        QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_I));
    m_importSongsAct->setStatusTip(tr("Import songs in the library"));
    m_importSongsAct->setIconText(tr("Import songs"));
    connect(m_importSongsAct, SIGNAL(triggered()), this,
            SLOT(importSongsDialog()));

    m_preferencesAct = new QAction(tr("&Preferences"), this);
    m_preferencesAct->setIcon(
        QIcon::fromTheme("document-properties",
                         QIcon(":/icons/tango/32x32/document-properties.png")));
    m_preferencesAct->setStatusTip(tr("Configure the application"));
    m_preferencesAct->setMenuRole(QAction::PreferencesRole);
    connect(m_preferencesAct, SIGNAL(triggered()), SLOT(preferences()));

    m_setupDatadirAct = new QAction(tr("&Choose Datadir"), this);
    m_setupDatadirAct->setIcon(QIcon::fromTheme(
        "folder-new", QIcon(":/icons/tango/32x32/actions/folder-new.png")));
    m_setupDatadirAct->setShortcut(
        QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_D));
    m_setupDatadirAct->setStatusTip(tr("Create a new datadir"));
    m_setupDatadirAct->setIconText(tr("Create datadir"));
    connect(m_setupDatadirAct, SIGNAL(triggered()), this,
            SLOT(setupDatadirDialog()));

    m_newAct = new QAction(tr("&New"), this);
    m_newAct->setIcon(QIcon::fromTheme(
        "folder-new", QIcon(":/icons/tango/32x32/actions/folder-new.png")));
    m_newAct->setShortcut(QKeySequence::New);
    m_newAct->setStatusTip(tr("Create a new songbook"));
    connect(m_newAct, SIGNAL(triggered()), this, SLOT(newSongbook()));

    m_openAct = new QAction(tr("&Open..."), this);
    m_openAct->setIcon(QIcon::fromTheme(
        "document-open",
        QIcon(":/icons/tango/32x32/actions/document-open.png")));
    m_openAct->setShortcut(QKeySequence::Open);
    m_openAct->setStatusTip(tr("Open a songbook"));
    connect(m_openAct, SIGNAL(triggered()), this, SLOT(open()));

    m_saveAct = new QAction(tr("&Save"), this);
    m_saveAct->setShortcut(QKeySequence::Save);
    m_saveAct->setIcon(QIcon::fromTheme(
        "document-save",
        QIcon(":/icons/tango/32x32/actions/document-save.png")));
    m_saveAct->setStatusTip(tr("Save the current songbook"));
    connect(m_saveAct, SIGNAL(triggered()), this, SLOT(save()));

    m_saveAsAct = new QAction(tr("Save &As..."), this);
    m_saveAsAct->setShortcut(QKeySequence::SaveAs);
    m_saveAsAct->setIcon(QIcon::fromTheme(
        "document-save-as",
        QIcon(":/icons/tango/32x32/actions/document-save-as.png")));
    m_saveAsAct->setStatusTip(
        tr("Save the current songbook with a different name"));
    connect(m_saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    m_documentationAct = new QAction(tr("Online &Documentation"), this);
    m_documentationAct->setShortcut(QKeySequence::HelpContents);
    m_documentationAct->setIcon(QIcon::fromTheme(
        "help-contents",
        QIcon(":/icons/tango/32x32/actions/help-contents.png")));
    m_documentationAct->setStatusTip(tr("Download documentation pdf file "));
    connect(m_documentationAct, SIGNAL(triggered()), this,
            SLOT(documentation()));

    m_bugsAct = new QAction(tr("&Report a bug"), this);
    m_bugsAct->setStatusTip(tr("Report a bug about this application"));
    connect(m_bugsAct, SIGNAL(triggered()), this, SLOT(reportBug()));

    m_aboutAct = new QAction(tr("&About"), this);
    m_aboutAct->setIcon(QIcon::fromTheme(
        "help-about", QIcon(":/icons/tango/32x32/actions/help-about.png")));
    m_aboutAct->setStatusTip(tr("About this application"));
    m_aboutAct->setMenuRole(QAction::AboutRole);
    connect(m_aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    m_exitAct = new QAction(tr("&Quit"), this);
    m_exitAct->setIcon(QIcon::fromTheme(
        "application-exit", QIcon(":/icons/tango/32x32/application-exit.png")));
    m_exitAct->setShortcut(QKeySequence::Quit);
    m_exitAct->setStatusTip(tr("Quit the program"));
    m_exitAct->setMenuRole(QAction::QuitRole);
    connect(m_exitAct, SIGNAL(triggered()), this, SLOT(close()));

    m_selectAllAct = new QAction(tr("&Check all"), this);
    m_selectAllAct->setIcon(QIcon::fromTheme(
        "select-all", QIcon(":/icons/songbook/32x32/select-all.png")));
    m_selectAllAct->setStatusTip(tr("Check all songs"));
    connect(m_selectAllAct, SIGNAL(triggered()), m_proxyModel,
            SLOT(checkAll()));

    m_unselectAllAct = new QAction(tr("&Uncheck all"), this);
    m_unselectAllAct->setIcon(QIcon::fromTheme(
        "select-none", QIcon(":/icons/songbook/32x32/select-none.png")));
    m_unselectAllAct->setStatusTip(tr("Uncheck all songs"));
    connect(m_unselectAllAct, SIGNAL(triggered()), m_proxyModel,
            SLOT(uncheckAll()));

    m_invertSelectionAct = new QAction(tr("&Toggle all"), this);
    m_invertSelectionAct->setIcon(QIcon::fromTheme(
        "select-invert", QIcon(":/icons/songbook/32x32/select-invert.png")));
    m_invertSelectionAct->setStatusTip(
        tr("Toggle the checked state of all songs"));
    connect(m_invertSelectionAct, SIGNAL(triggered()), m_proxyModel,
            SLOT(toggleAll()));

    m_libraryUpdateAct = new QAction(tr("&Update"), this);
    m_libraryUpdateAct->setStatusTip(
        tr("Update current song list from \".sg\" files"));
    m_libraryUpdateAct->setIcon(QIcon::fromTheme(
        "view-refresh", QIcon(":/icons/tango/32x32/actions/view-refresh.png")));
    m_libraryUpdateAct->setShortcut(QKeySequence::Refresh);
    connect(m_libraryUpdateAct, SIGNAL(triggered()), library(), SLOT(update()));

    m_buildAct = new QAction(tr("&Build PDF"), this);
    m_buildAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_B));
    m_buildAct->setIcon(QIcon::fromTheme(
        "document-export",
        QIcon(":/icons/tango/32x32/mimetypes/document-export.png")));
    m_buildAct->setStatusTip(tr("Generate pdf from selected songs"));
    connect(m_buildAct, SIGNAL(triggered()), this, SLOT(build()));

    m_cleanAct = new QAction(tr("&Clean"), this);
    m_cleanAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C));
    m_cleanAct->setIcon(QIcon::fromTheme(
        "edit-clear", QIcon(":/icons/tango/32x32/actions/edit-clear.png")));
    m_cleanAct->setStatusTip(tr("Clean LaTeX temporary files"));
    connect(m_cleanAct, SIGNAL(triggered()), this, SLOT(cleanDialog()));
}

void MainWindow::setToolBarDisplayed(bool value)
{
    if (m_isToolBarDisplayed != value && m_currentToolBar) {
        m_isToolBarDisplayed = value;
        m_currentToolBar->setVisible(value);
        m_mainToolBar->setVisible(value);
    }
}

bool MainWindow::isToolBarDisplayed()
{
    return m_isToolBarDisplayed;
}

void MainWindow::setStatusBarDisplayed(bool value)
{
    m_isStatusBarDisplayed = value;
    statusBar()->setVisible(value);
}

bool MainWindow::isStatusBarDisplayed()
{
    return m_isStatusBarDisplayed;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void MainWindow::createMenus()
{
    menuBar()->setContextMenuPolicy(Qt::PreventContextMenu);

    QMenu *fileMenu = menuBar()->addMenu(tr("&Songbook"));
    fileMenu->addAction(m_newAct);
    fileMenu->addAction(m_openAct);
    fileMenu->addAction(m_saveAct);
    fileMenu->addAction(m_saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(m_preferencesAct);
    fileMenu->addSeparator();
    fileMenu->addAction(m_buildAct);
    fileMenu->addAction(m_cleanAct);
    fileMenu->addSeparator();
    fileMenu->addAction(m_exitAct);

    QMenu *libraryMenu = menuBar()->addMenu(tr("&Library"));
    libraryMenu->addAction(m_newSongAct);
    libraryMenu->addAction(m_importSongsAct);
    libraryMenu->addAction(m_setupDatadirAct);
    libraryMenu->addSeparator();
    libraryMenu->addAction(m_selectAllAct);
    libraryMenu->addAction(m_unselectAllAct);
    libraryMenu->addAction(m_invertSelectionAct);
    libraryMenu->addSeparator();
    libraryMenu->addAction(m_libraryUpdateAct);

    m_editorMenu = menuBar()->addMenu(tr("&Editor"));

    m_voidEditor = new Editor(this);
    m_editorMenu->addActions(m_voidEditor->actionGroup()->actions());

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(m_documentationAct);
    helpMenu->addAction(m_bugsAct);
    helpMenu->addAction(m_aboutAct);
}

void MainWindow::createToolBar()
{
    m_mainToolBar = new QToolBar(tr("Song tools"), this);
    m_mainToolBar->setMovable(false);
    m_mainToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_mainToolBar->addAction(m_newSongAct);
    m_mainToolBar->addAction(m_buildAct);
    addToolBar(m_mainToolBar);

    QCompleter *completer = new QCompleter;
    completer->setModel(library()->completionModel());
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::PopupCompletion);

    m_filterLineEdit = new FilterLineEdit;
    m_filterLineEdit->setCompleter(completer);
    m_filterLineEdit->setFilterModel(
        qobject_cast<SongSortFilterProxyModel *>(m_proxyModel));

    QWidget *stretch = new QWidget;
    stretch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_libraryToolBar = new QToolBar(tr("Library tools"), this);
    m_libraryToolBar->setMovable(false);
    m_libraryToolBar->setContextMenuPolicy(Qt::PreventContextMenu);
    // selection actions
    m_libraryToolBar->addAction(m_selectAllAct);
    m_libraryToolBar->addAction(m_unselectAllAct);
    m_libraryToolBar->addAction(m_invertSelectionAct);
    // add toolbar spacing
    m_libraryToolBar->addWidget(stretch);
    // add toolbar filter
    m_libraryToolBar->addWidget(m_filterLineEdit);

    m_currentToolBar = m_libraryToolBar;
    addToolBar(m_libraryToolBar);
    setUnifiedTitleAndToolBarOnMac(true);
}

void MainWindow::preferences()
{
    ConfigDialog dialog(this);
    dialog.exec();
    readSettings();
}

void MainWindow::documentation()
{
    if (QLocale::system().language() == QLocale::French)
        QDesktopServices::openUrl(
            QUrl("http://www.patacrep.fr/data/documents/doc_fr.pdf"));
    else
        QDesktopServices::openUrl(
            QUrl("http://www.patacrep.fr/data/documents/doc_en.pdf"));
}

void MainWindow::reportBug()
{
    QDesktopServices::openUrl(
        QUrl("https://github.com/patacrep/patagui/issues"));
}

void MainWindow::about()
{
    QString title(tr("About Patagui"));
    QString version = QCoreApplication::applicationVersion();

    QString description(
        tr("This program allows one to build customized songbooks from "
           "<a href=\"http::www.patacrep.fr\">www.patacrep.fr</a>"));

    QStringList authorsList = QStringList() << "Crep (R. Goffe)"
                                            << "Lohrun (A. Dupas)"
                                            << "Carreau (M. Bussonnier)";
    QString authors = authorsList.join(", ");

    QMessageBox::about(this, title, tr("<p>%1</p>"
                                       "<p><b>Version:</b> %2</p>"
                                       "<p><b>Authors:</b> %3</p>")
                                        .arg(description)
                                        .arg(version)
                                        .arg(authors));
}

void MainWindow::build()
{
    if (!checkPdfLaTeX() || !checkPython())
        return;

    patacrep->testPython();

    songbook()->songsFromSelection();
    if (songbook()->songs().isEmpty()) {
        if (QMessageBox::question(
                this, windowTitle(),
                tr("You did not select any song. \n "
                   "Do you want to build the songbook with all songs?"),
                QMessageBox::Yes, QMessageBox::No,
                QMessageBox::NoButton) == QMessageBox::No)
            return;
        else
            songbook()->checkAll();
    }

    save();

    if (QFile(songbook()->filename()).exists()) {
        //        qobject_cast<QPlainTextEdit *>(log()->widget())->clear();
        make();
    } else {
        statusBar()->showMessage(
            tr("The songbook file %1 is invalid. Build aborted.")
                .arg(songbook()->filename()));
    }
}

void MainWindow::newSongbook()
{
    songbook()->reset();
    updateTitle(songbook()->filename());
}

void MainWindow::open(const QString &filename)
{
    songbook()->load(filename);
    updateTitle(songbook()->filename());
}

void MainWindow::open()
{
    QString filename = QFileDialog::getOpenFileName(
        this, tr("Open"), QString("%1/books").arg(workingPath()),
        tr("Songbook (*.sb)"));
    open(filename);
}

void MainWindow::save()
{
    if (songbook()->filename().isEmpty()) {
        return saveAs();
    }

    songbook()->save(songbook()->filename());
    updateTitle(songbook()->filename());
}

void MainWindow::saveAs()
{
    QString filename = QFileDialog::getSaveFileName(
        this, tr("Save as"), QDir::homePath(), tr("Songbook (*.sb)"));

    if (!filename.isEmpty()) {
        songbook()->setFilename(filename);
        save();
    }
}

void MainWindow::updateTitle(const QString &filename)
{
    QString text = filename.isEmpty() ? tr("New songbook") : filename;
    setWindowTitle(
        tr("%1 - %2[*]").arg(QCoreApplication::applicationName()).arg(text));
}

const QString MainWindow::workingPath()
{
    QSettings settings;
    settings.beginGroup("global");
    QString path = settings.value("songbookPath", QDir::homePath()).toString();
    settings.endGroup();
    return path;
}

const QString MainWindow::libraryPath()
{
    return library()->directory().canonicalPath();
}

void MainWindow::make()
{
    if (!future.isRunning()) {
        patacrep->setWorkingDirectory(libraryPath());
        patacrep->setSongbook(songbook());
        patacrep->addDatadir(songbook()->library()->directory().absolutePath());
        // To change properly, make access to datadir in songbook class

        future = QtConcurrent::run(patacrep, &Patacrep::buildSongbook);
    } else {
        // TODO: Choose behaviour: Wait for finished or error message?
        qDebug() << "Process already running";
    }
}

void MainWindow::cancelProcess()
{
    if (future.isRunning()) {
        patacrep->stopBuilding();
    }
}

ProgressBar *MainWindow::progressBar() const
{
    return m_progressBar;
}

Songbook *MainWindow::songbook() const
{
    return m_songbook;
}

LibraryView *MainWindow::view() const
{
    return m_view;
}

Library *MainWindow::library() const
{
    return Library::instance();
}

QItemSelectionModel *MainWindow::selectionModel()
{
    return view()->selectionModel();
}

void MainWindow::middleClicked(const QModelIndex &index)
{
    if (QApplication::mouseButtons() == Qt::MidButton) {
        songEditor(index);
        m_mainWidget->setCurrentIndex(0);
    }
}

void MainWindow::songEditor(const QModelIndex &index)
{
    Q_UNUSED(index);
    if (!selectionModel()->hasSelection()) {
        statusBar()->showMessage(tr("Please select a song to edit."));
        return;
    }

    QString path =
        view()
            ->model()
            ->data(selectionModel()->currentIndex(), Library::PathRole)
            .toString();

    songEditor(path);
}

void MainWindow::songEditor(const QString &path)
{
    // if an editor already corresponds to path, focus on it
    for (int i = 0; i < m_mainWidget->count(); ++i)
        if (SongEditor *editor =
                qobject_cast<SongEditor *>(m_mainWidget->widget(i)))
            if (editor->song().path == path) {
                m_mainWidget->setCurrentIndex(i);
                return;
            }

    // create a new editor
    SongEditor *editor = new SongEditor(this);

    if (!path.isEmpty()) {
        // if the song does not exist within the library, add it
        if (library()->getSongIndex(path) == -1)
            library()->addSongs(QStringList() << path);

        editor->setSong(library()->getSong(path));
    }

    // create the corresponding tab
    connect(editor, SIGNAL(labelChanged(const QString &)), m_mainWidget,
            SLOT(changeTabText(const QString &)));
    m_mainWidget->addTab(editor);
}

void MainWindow::newSong()
{
    songEditor(QString());
}

void MainWindow::importSongsDialog()
{
    ImportDialog *dialog = new ImportDialog(this);
    connect(dialog, SIGNAL(songsReadyToBeImported(const QStringList &)), this,
            SLOT(importSongs(const QStringList &)));
    dialog->exec();
}

void MainWindow::setupDatadirDialog()
{
    QString datadir = QFileDialog::getExistingDirectory(
        this, tr("Select Datadir"), QDir::homePath(),
        QFileDialog::ShowDirsOnly);
    if (datadir != "") {
        library()->setDirectory(datadir);
        if (library()->directory().mkdir("songs")) {
            // TODO Handle this. Message in Status Bar ?
            qDebug() << "Songs Directory created";
        }
    } else {
        if (QMessageBox::question(this, windowTitle(),
                                  tr("You did not select a datadir.\nDo you "
                                     "want to select one ?")) ==
            QMessageBox::Yes) {
            setupDatadirDialog();
        } else {
            QMessageBox::critical(
                this, tr("No datadir"),
                tr("You did not set a datadir.\nThis behaviour is unknown."));
        }
    }
}

void MainWindow::importSongs(const QStringList &songs)
{
    library()->importSongs(songs);
}

void MainWindow::deleteSong()
{
    if (!selectionModel()->hasSelection()) {
        statusBar()->showMessage(tr("Please select a song to remove."));
        return;
    }

    QString path =
        view()
            ->model()
            ->data(selectionModel()->currentIndex(), Library::PathRole)
            .toString();

    deleteSong(path);
}

void MainWindow::deleteSong(const QString &path)
{
    int ret = QMessageBox::warning(
        this, tr("Patagui"), tr("This file will be deleted:\n%1\n"
                                "Are you sure?").arg(path),
        QMessageBox::Cancel | QMessageBox::Ok, QMessageBox::Cancel);

    if (ret == QMessageBox::Ok)
        library()->deleteSong(path);
}

void MainWindow::closeTab(int index)
{
    if (SongEditor *editor =
            qobject_cast<SongEditor *>(m_mainWidget->widget(index)))
        if (editor->close()) {
            m_mainWidget->closeTab(index);
            delete editor;
        }
}

void MainWindow::changeTab(int index)
{
    m_editorMenu->clear();
    Editor *editor = qobject_cast<Editor *>(m_mainWidget->widget(index));
    if (editor != 0) {
        switchToolBar(editor->toolBar());
        m_saveAct->setShortcutContext(Qt::WidgetShortcut);

        // install highlighter
        if (!m_songHighlighter) {
            m_songHighlighter = new SongHighlighter;
        }
        editor->setHighlighter(m_songHighlighter);
    } else {
        editor = m_voidEditor;
        switchToolBar(m_libraryToolBar);
        m_saveAct->setShortcutContext(Qt::WindowShortcut);
    }
    m_editorMenu->addActions(editor->actionGroup()->actions());
}

QDockWidget *MainWindow::log() const
{
    return m_log;
}

void MainWindow::updateNotification(const QString &path)
{
    if (!m_updateAvailable) {
        m_updateAvailable = new Notification(this);
        m_updateAvailable->addAction(m_libraryUpdateAct);
    }

    m_updateAvailable->setMessage(
        tr("<strong>The following directory has been modified:</strong><br/>"
           "  %1 <br/>"
           "Do you want to update the library to reflect these changes?")
            .arg(path));
}

void MainWindow::noDataNotification(const QDir &directory)
{
    if (!m_noDataInfo) {
        m_noDataInfo = new Notification(this);
        m_noDataInfo->addAction(m_importSongsAct);
    }

    if (library()->rowCount() > 0) {
        m_noDataInfo->hide();
        m_buildAct->setEnabled(true);
    } else {
        m_noDataInfo->setMessage(
            tr("<strong>The directory <b>%1</b> does not contain any "
               "song.</strong><br/>"
               "Do you want to download the latest songs library?")
                .arg(directory.canonicalPath()));
        m_noDataInfo->show();
        m_buildAct->setEnabled(false);
    }
}

void MainWindow::noSongbookDirectoryNotification()
{
    if (!m_noDatadirSet) {
        m_noDatadirSet = new Notification(this);
        m_noDatadirSet->addAction(m_setupDatadirAct);
    }

    if (library()->rowCount() > 0) {
        m_noDatadirSet->hide();
        m_buildAct->setEnabled(true);
    } else {
        m_noDatadirSet->setMessage(tr(
            "There is no datadir at the moment. Do you want to set one up?"));
        m_noDatadirSet->show();
        m_buildAct->setEnabled(false);
    }
}

void MainWindow::cleanDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Clean"));

    QDialogButtonBox *buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), &dialog, SLOT(close()));

    if (!m_tempFilesmodel) {
        m_tempFilesmodel = new QFileSystemModel;
        m_tempFilesmodel->setRootPath(library()->directory().canonicalPath());
        m_tempFilesmodel->setNameFilters(QStringList() << "*.aux"
                                                       << "*.d"
                                                       << "*.toc"
                                                       << "*.out"
                                                       << "*.log"
                                                       << "*.nav"
                                                       << "*.snm"
                                                       << "*.sbx"
                                                       << "*.sxd");
        m_tempFilesmodel->setNameFilterDisables(false);
        m_tempFilesmodel->setFilter(QDir::Files);
    }

    QListView *view = new QListView;
    view->setModel(m_tempFilesmodel);
    view->setRootIndex(
        m_tempFilesmodel->index(library()->directory().canonicalPath()));

    QCheckBox *cleanAllButton =
        new QCheckBox(tr("Also remove pdf files"), this);
    updateTempFilesView(cleanAllButton->checkState());
    connect(cleanAllButton, SIGNAL(stateChanged(int)), this,
            SLOT(updateTempFilesView(int)));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(view);
    layout->addWidget(cleanAllButton);
    layout->addWidget(buttonBox);
    dialog.setLayout(layout);

    //    FIXME
    //    if (dialog.exec() == QDialog::Accepted) {
    //        if (cleanAllButton->isChecked())
    //            makeCleanall();
    //        else
    //            makeClean();
    //    }
}

void MainWindow::updateTempFilesView(int state)
{
    if (state == Qt::Checked) {
        m_tempFilesmodel->setNameFilters(m_tempFilesmodel->nameFilters()
                                         << "*.pdf");
    } else if (m_tempFilesmodel->nameFilters().contains("*.pdf")) {
        QStringList list = m_tempFilesmodel->nameFilters();
        list.removeLast();
        m_tempFilesmodel->setNameFilters(list);
    }
}
