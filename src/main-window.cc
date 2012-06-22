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

#include <QAction>
#include <QBoxLayout>
#include <QCheckBox>
#include <QCloseEvent>
#include <QCompleter>
#include <QCoreApplication>
#include <QDesktopServices>
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
#include <QTimer>
#include <QToolBar>

#include "label.hh"
#include "library.hh"
#include "library-view.hh"
#include "songbook.hh"
#include "song-editor.hh"
#include "logs-highlighter.hh"
#include "filter-lineedit.hh"
#include "song-sort-filter-proxy-model.hh"
#include "tab-widget.hh"
#include "notification.hh"
#include "song-item-delegate.hh"
#include "preferences.hh"
#include "progress-bar.hh"

#include "config.hh"

#ifdef ENABLE_LIBRARY_DOWNLOAD
#include "library-download.hh"
#endif // ENABLE_LIBRARY_DOWNLOAD

#include "make-songbook-process.hh"

#include <QDebug>

namespace // anonymous namespace
{
  bool checkPdfLaTeX()
  {
    QString message;
    QProcess process;
    process.start("pdflatex", QStringList() << "--version");
    if (!process.waitForFinished())
      {
	QString platformSpecificMessage;
#if defined(Q_OS_WIN32)
	platformSpecificMessage = QObject::tr("<ol><li>Download and install the <a href=\"http://miktex.org\"/>MikTeX</a> distribution for Windows.</li>"
					      "<li>Verify that your PATH variable is correctly set.</li></ol>");
#elif defined(Q_OS_MAC)
	platformSpecificMessage = QObject::tr("<p>Download and install the <a href=\"http://www.tug.org/mactex\">MacTeX</a> distribution for Mac OS.</p>");
#else //Unix/Linux
	platformSpecificMessage = QObject::tr("<p>Download and install the following packages:</p>"
					      "<ol><li>texlive-base</li>"
					      "<li>texlive-latex-extra</li></ol>");
#endif
	message = QObject::tr("<p>The following program cannot be found: <i>pdflatex</i>.</p>"
			      "<p>A <a href=\"www.latex-project.org/\">LaTeX</a> distribution supporting <i>pdflatex</i> is required "
			      "to produce the PDF document. Such a distribution is either "
			      "not installed or misconfigured.</p>"
			      "%1"
			      "You can find more information in the "
			      "<a href=\"http://www.patacrep.com/data/documents/doc_%2.pdf\">"
			      "documentation</a>.\n")
	  .arg(platformSpecificMessage)
	  .arg((QLocale::system().language() == QLocale::French)? "fr":"en");
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
    if (!process.waitForFinished())
      {
	QString platformSpecificMessage;
#if defined(Q_OS_WIN32)
	platformSpecificMessage = QObject::tr("<ol><li>Download and install <a href=\"http://www.python.org/download\"</a>Python 2.X</a> for Windows.</li>"
					      "<li>Verify that your PATH variable is correctly set.</li></ol>");
#elif defined(Q_OS_MAC)
	platformSpecificMessage = QObject::tr("<ol><li>Download and install <a href=\"http://www.python.org/download\"</a>Python 2.X</a> for Mac OS.</li>"
					      "<li>Verify that your PATH variable is correctly set.</li></ol>");
#else //Unix/Linux
	platformSpecificMessage = QObject::tr("<p>Download and install the following packages: <i>python</i></p>");
#endif
	message = QObject::tr("<p>The following program cannot be found: <i>python</i>.</p>"
			      "<p>A version of <a href=\"www.python.org/\">Python 2</a> is required "
			      "to produce the PDF document.</p>"
			      "%1"
			      "You can find more information in the "
			      "<a href=\"http://www.patacrep.com/data/documents/doc_%2.pdf\">"
			      "documentation</a>.\n")
	  .arg(platformSpecificMessage)
	  .arg((QLocale::system().language() == QLocale::French)? "fr":"en");
	QMessageBox::warning(0, QObject::tr("Missing program"), message);
	return false;
      }
    return true;
  }
}

CMainWindow::CMainWindow(QWidget *parent)
  : QMainWindow(parent)
  , m_library(new CLibrary(this))
  , m_view(new CLibraryView(this))
  , m_songbook(new CSongbook(this))
  , m_proxyModel(new CSongSortFilterProxyModel(this))
  , m_tempFilesmodel(0)
  , m_mainWidget(new CTabWidget(this))
  , m_progressBar(new CProgressBar(this))
  , m_noDataInfo(0)
  , m_updateAvailable(0)
  , m_infoSelection(new QLabel(this))
  , m_log(new QDockWidget(tr("LaTeX compilation logs")))
  , m_isToolBarDisplayed(true)
  , m_currentToolBar(0)
  , m_builder(new CMakeSongbookProcess(this))
{
  setWindowTitle("Patacrep Songbook Client");
  setWindowIcon(QIcon(":/icons/songbook/256x256/songbook-client.png"));

  connect(m_library, SIGNAL(directoryChanged(const QDir &)),
	  SLOT(noDataNotification(const QDir &)));

  // songbook (title, authors, song list)
  m_songbook->setLibrary(m_library);

  connect(m_songbook, SIGNAL(wasModified(bool)), SLOT(setWindowModified(bool)));
  connect(m_songbook, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
	  SLOT(selectedSongsChanged(const QModelIndex &, const QModelIndex &)));

  // proxy model (sorting & filtering)
  m_proxyModel->setSourceModel(m_songbook);
  m_proxyModel->setSortLocaleAware(true);
  m_proxyModel->setDynamicSortFilter(true);
  m_proxyModel->setFilterKeyColumn(-1);

  // view
  m_view->setModel(m_proxyModel);
  m_view->setItemDelegate(new CSongItemDelegate);
  m_view->resizeColumns();
  connect(m_library, SIGNAL(wasModified()), m_view, SLOT(update()));

  // compilation log
  QPlainTextEdit* logs = new QPlainTextEdit;
  logs->setReadOnly(true);
  Q_UNUSED(new CLogsHighlighter(logs->document()));
  m_log->setWidget(logs);
  addDockWidget(Qt::BottomDockWidgetArea, m_log);

  createActions();
  createMenus();
  createToolBar();

  // place elements into the main window
  m_mainWidget->setTabsClosable(true);
  m_mainWidget->setMovable(true);
  m_mainWidget->setSelectionBehaviorOnAdd(CTabWidget::SelectNew);
  connect(m_mainWidget, SIGNAL(tabCloseRequested(int)), SLOT(closeTab(int)));
  connect(m_mainWidget, SIGNAL(currentChanged(int)), SLOT(changeTab(int)));
  m_mainWidget->addTab(m_view, tr("Library"));
  setCentralWidget(m_mainWidget);

  m_progressBar->setTextVisible(false);
  m_progressBar->setRange(0, 0);
  m_progressBar->hide();
  connect(progressBar(), SIGNAL(canceled()),
          this, SLOT(cancelProcess()));

  // status bar with an embedded label and progress bar
  statusBar()->addPermanentWidget(m_infoSelection);
  statusBar()->addPermanentWidget(m_progressBar);

  // make/make clean/make cleanall process
  connect(m_builder, SIGNAL(aboutToStart()),
          progressBar(), SLOT(show()));
  connect(m_builder, SIGNAL(aboutToStart()),
          statusBar(), SLOT(clear()));
  connect(m_builder, SIGNAL(message(const QString &, int)), statusBar(),
          SLOT(showMessage(const QString &, int)));
  connect(m_builder, SIGNAL(finished(int, QProcess::ExitStatus)),
          progressBar(), SLOT(hide()));
  connect(m_builder, SIGNAL(readOnStandardOutput(const QString &)),
          log()->widget(), SLOT(appendPlainText(const QString &)));
  connect(m_builder, SIGNAL(readOnStandardError(const QString &)),
          log()->widget(), SLOT(appendPlainText(const QString &)));
  connect(m_builder, SIGNAL(error(QProcess::ProcessError)),
          this, SLOT(buildError(QProcess::ProcessError)));

  updateTitle(songbook()->filename());

  readSettings(true);
}

CMainWindow::~CMainWindow()
{
  delete m_library;
  delete m_songbook;
}

void CMainWindow::switchToolBar(QToolBar *toolBar)
{
  if (toolBar != m_currentToolBar)
    {
      addToolBar(toolBar);
      toolBar->setVisible(isToolBarDisplayed());
      m_currentToolBar->setVisible(false);
      removeToolBar(m_currentToolBar);
      m_currentToolBar = toolBar;
    }
}

void CMainWindow::readSettings(bool firstLaunch)
{
  QSettings settings;
  settings.beginGroup("general");
  setStatusbarDisplayed(settings.value("statusBar", true).toBool());
  setToolBarDisplayed(settings.value("toolBar", true).toBool());
  if (firstLaunch)
    {
      resize(settings.value("size", QSize(800,600)).toSize());
      move(settings.value("pos", QPoint(200, 200)).toPoint());
      if (settings.value("maximized", isMaximized()).toBool())
	showMaximized();
    }
  settings.endGroup();

  settings.beginGroup("display");
  log()->setVisible(settings.value("logs", false).toBool());
  settings.endGroup();

  settings.beginGroup("tools");
  setBuildCommand(settings.value("buildCommand", PLATFORM_BUILD_COMMAND).toString());
  setCleanCommand(settings.value("cleanCommand", PLATFORM_CLEAN_COMMAND).toString());
  setCleanallCommand(settings.value("cleanallCommand", PLATFORM_CLEAN_COMMAND).toString());
  settings.endGroup();

  view()->readSettings();
  QTimer::singleShot(100, library(), SLOT(readSettings()));
}

void CMainWindow::writeSettings()
{
  QSettings settings;
  settings.beginGroup("general");
  settings.setValue("statusBar", isStatusbarDisplayed());
  settings.setValue("toolBar", isToolBarDisplayed());
  settings.setValue( "maximized", isMaximized() );
  if (!isMaximized())
    {
      settings.setValue( "pos", pos() );
      settings.setValue( "size", size() );
    }
  settings.endGroup();

  library()->writeSettings();
  view()->writeSettings();
}

void CMainWindow::selectedSongsChanged(const QModelIndex &, const QModelIndex &)
{
  m_infoSelection->setText(tr("Selection: %1/%2")
			   .arg(songbook()->selectedCount())
			   .arg(songbook()->rowCount()));
}

void CMainWindow::createActions()
{
  m_newSongAct = new QAction(tr("&Add Song"), this);
  m_newSongAct->setIcon(QIcon::fromTheme("list-add", QIcon(":/icons/tango/32x32/actions/list-add.png")));
  m_newSongAct->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_N));
  m_newSongAct->setStatusTip(tr("Write a new song"));
  m_newSongAct->setIconText(tr("Add song"));
  connect(m_newSongAct, SIGNAL(triggered()), this, SLOT(newSong()));

  m_newAct = new QAction(tr("&New"), this);
  m_newAct->setIcon(QIcon::fromTheme("folder-new", QIcon(":/icons/tango/32x32/actions/folder-new.png")));
  m_newAct->setShortcut(QKeySequence::New);
  m_newAct->setStatusTip(tr("Create a new songbook"));
  connect(m_newAct, SIGNAL(triggered()), this, SLOT(newSongbook()));

  m_openAct = new QAction(tr("&Open..."), this);
  m_openAct->setIcon(QIcon::fromTheme("document-open", QIcon(":/icons/tango/32x32/actions/document-open.png")));
  m_openAct->setShortcut(QKeySequence::Open);
  m_openAct->setStatusTip(tr("Open a songbook"));
  connect(m_openAct, SIGNAL(triggered()), this, SLOT(open()));

  m_saveAct = new QAction(tr("&Save"), this);
  m_saveAct->setShortcut(QKeySequence::Save);
  m_saveAct->setIcon(QIcon::fromTheme("document-save", QIcon(":/icons/tango/32x32/actions/document-save.png")));
  m_saveAct->setStatusTip(tr("Save the current songbook"));
  connect(m_saveAct, SIGNAL(triggered()), this, SLOT(save()));

  m_saveAsAct = new QAction(tr("Save &As..."), this);
  m_saveAsAct->setShortcut(QKeySequence::SaveAs);
  m_saveAsAct->setIcon(QIcon::fromTheme("document-save-as", QIcon(":/icons/tango/32x32/actions/document-save-as.png")));
  m_saveAsAct->setStatusTip(tr("Save the current songbook with a different name"));
  connect(m_saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

  m_documentationAct = new QAction(tr("Online &Documentation"), this);
  m_documentationAct->setShortcut(QKeySequence::HelpContents);
  m_documentationAct->setIcon(QIcon::fromTheme("help-contents", QIcon(":/icons/tango/32x32/actions/help-contents.png")));
  m_documentationAct->setStatusTip(tr("Download documentation pdf file "));
  connect(m_documentationAct, SIGNAL(triggered()), this, SLOT(documentation()));

  m_bugsAct = new QAction(tr("&Report a bug"), this);
  m_bugsAct->setStatusTip(tr("Report a bug about this application"));
  connect(m_bugsAct, SIGNAL(triggered()), this, SLOT(reportBug()));

  m_aboutAct = new QAction(tr("&About"), this);
  m_aboutAct->setIcon(QIcon::fromTheme("help-about", QIcon(":/icons/tango/32x32/actions/help-about.png")));
  m_aboutAct->setStatusTip(tr("About this application"));
  m_aboutAct->setMenuRole(QAction::AboutRole);
  connect(m_aboutAct, SIGNAL(triggered()), this, SLOT(about()));

  m_exitAct = new QAction(tr("&Quit"), this);
  m_exitAct->setIcon(QIcon::fromTheme("application-exit",QIcon(":/icons/tango/32x32/application-exit.png")));
  m_exitAct->setShortcut(QKeySequence::Quit);
  m_exitAct->setStatusTip(tr("Quit the program"));
  m_exitAct->setMenuRole(QAction::QuitRole);
  connect(m_exitAct, SIGNAL(triggered()), this, SLOT(close()));

  m_preferencesAct = new QAction(tr("&Preferences"), this);
  m_preferencesAct->setIcon(QIcon::fromTheme("document-properties",QIcon(":/icons/tango/32x32/document-properties.png")));
  m_preferencesAct->setStatusTip(tr("Configure the application"));
  m_preferencesAct->setMenuRole(QAction::PreferencesRole);
  connect(m_preferencesAct, SIGNAL(triggered()), SLOT(preferences()));

  m_selectAllAct = new QAction(tr("&Check all"), this);
  m_selectAllAct->setIcon(QIcon::fromTheme("select-all",QIcon(":/icons/songbook/32x32/select-all.png")));
  m_selectAllAct->setStatusTip(tr("Check all songs"));
  connect(m_selectAllAct, SIGNAL(triggered()), m_proxyModel, SLOT(checkAll()));

  m_unselectAllAct = new QAction(tr("&Uncheck all"), this);
  m_unselectAllAct->setIcon(QIcon::fromTheme("select-none",QIcon(":/icons/songbook/32x32/select-none.png")));
  m_unselectAllAct->setStatusTip(tr("Uncheck all songs"));
  connect(m_unselectAllAct, SIGNAL(triggered()), m_proxyModel, SLOT(uncheckAll()));

  m_invertSelectionAct = new QAction(tr("&Toggle all"), this);
  m_invertSelectionAct->setIcon(QIcon::fromTheme("select-invert",QIcon(":/icons/songbook/32x32/select-invert.png")));
  m_invertSelectionAct->setStatusTip(tr("Toggle the checked state of all songs"));
  connect(m_invertSelectionAct, SIGNAL(triggered()), m_proxyModel, SLOT(toggleAll()));

  m_libraryUpdateAct = new QAction(tr("&Update"), this);
  m_libraryUpdateAct->setStatusTip(tr("Update current song list from \".sg\" files"));
  m_libraryUpdateAct->setIcon(QIcon::fromTheme("view-refresh", QIcon(":/icons/tango/32x32/actions/view-refresh.png")));
  m_libraryUpdateAct->setShortcut(QKeySequence::Refresh);
  connect(m_libraryUpdateAct, SIGNAL(triggered()), library(), SLOT(update()));

  m_libraryDownloadAct = new QAction(tr("&Download"), this);
  m_libraryDownloadAct->setStatusTip(tr("Download songs from remote location"));
  m_libraryDownloadAct->setIcon(QIcon::fromTheme("folder-remote", QIcon(":/icons/tango/32x32/places/folder-remote.png")));
#ifdef ENABLE_LIBRARY_DOWNLOAD
  connect(m_libraryDownloadAct, SIGNAL(triggered()), this, SLOT(downloadDialog()));
#else // ENABLE_LIBRARY_DOWNLOAD
  m_libraryDownloadAct->setEnabled(false);
#endif // ENABLE_LIBRARY_DOWNLOAD

  QSettings settings;
  settings.beginGroup("general");
  m_toolBarViewAct = new QAction(tr("ToolBar"),this);
  m_toolBarViewAct->setStatusTip(tr("Show or hide the toolbar in the current window"));
  m_toolBarViewAct->setCheckable(true);
  m_toolBarViewAct->setChecked(settings.value("toolBar", true).toBool());
  connect(m_toolBarViewAct, SIGNAL(toggled(bool)), this, SLOT(setToolBarDisplayed(bool)));

  m_statusbarViewAct = new QAction(tr("Statusbar"),this);
  m_statusbarViewAct->setStatusTip(tr("Show or hide the statusbar in the current window"));
  m_statusbarViewAct->setCheckable(true);
  m_statusbarViewAct->setChecked(settings.value("statusBar", true).toBool());
  connect(m_statusbarViewAct, SIGNAL(toggled(bool)), this, SLOT(setStatusbarDisplayed(bool)));
  settings.endGroup();

  m_buildAct = new QAction(tr("&Build PDF"), this);
  m_buildAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_B));
  m_buildAct->setIcon(QIcon::fromTheme("document-export",QIcon(":/icons/tango/32x32/mimetypes/document-export.png")));
  m_buildAct->setStatusTip(tr("Generate pdf from selected songs"));
  connect(m_buildAct, SIGNAL(triggered()), this, SLOT(build()));

  m_cleanAct = new QAction(tr("&Clean"), this);
  m_cleanAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C));
  m_cleanAct->setIcon(QIcon::fromTheme("edit-clear", QIcon(":/icons/tango/32x32/actions/edit-clear.png")));
  m_cleanAct->setStatusTip(tr("Clean LaTeX temporary files"));
  connect(m_cleanAct, SIGNAL(triggered()), this, SLOT(cleanDialog()));
}

void CMainWindow::setToolBarDisplayed(bool value)
{
  if (m_isToolBarDisplayed != value && m_currentToolBar)
    {
      m_isToolBarDisplayed = value;
      m_currentToolBar->setVisible(value);
      m_mainToolBar->setVisible(value);
    }
}

bool CMainWindow::isToolBarDisplayed()
{
  return m_isToolBarDisplayed;
}

void CMainWindow::setStatusbarDisplayed(bool value)
{
  m_isStatusbarDisplayed = value;
  statusBar()->setVisible(value);
}

bool CMainWindow::isStatusbarDisplayed()
{
  return m_isStatusbarDisplayed;
}

void CMainWindow::closeEvent(QCloseEvent *event)
{
  writeSettings();
  event->accept();
}

void CMainWindow::createMenus()
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
  libraryMenu->addSeparator();
  libraryMenu->addAction(m_selectAllAct);
  libraryMenu->addAction(m_unselectAllAct);
  libraryMenu->addAction(m_invertSelectionAct);
  libraryMenu->addSeparator();
  libraryMenu->addAction(m_libraryDownloadAct);
  libraryMenu->addAction(m_libraryUpdateAct);

  m_editorMenu = menuBar()->addMenu(tr("&Editor"));

  m_voidEditor = new CSongEditor(this);
  m_voidEditor->actionGroup()->setEnabled(false);
  m_editorMenu->addActions(m_voidEditor->actionGroup()->actions());

  QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
  viewMenu->addAction(m_toolBarViewAct);
  viewMenu->addAction(m_statusbarViewAct);

  QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
  helpMenu->addAction(m_documentationAct);
  helpMenu->addAction(m_bugsAct);
  helpMenu->addAction(m_aboutAct);
}

void CMainWindow::createToolBar()
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

  m_filterLineEdit = new CFilterLineEdit;
  m_filterLineEdit->setCompleter(completer);
  m_filterLineEdit->setFilterModel(qobject_cast< CSongSortFilterProxyModel* >(m_proxyModel));

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

void CMainWindow::preferences()
{
  ConfigDialog dialog(this);
  dialog.exec();
  readSettings();
}

void CMainWindow::documentation()
{
  if (QLocale::system().language() == QLocale::French)
    QDesktopServices::openUrl(QUrl("http://www.patacrep.com/data/documents/doc_fr.pdf"));
  else
    QDesktopServices::openUrl(QUrl("http://www.patacrep.com/data/documents/doc_en.pdf"));
}

void CMainWindow::reportBug()
{
  QDesktopServices::openUrl(QUrl("https://github.com/crep4ever/songbook-client/issues"));
}

void CMainWindow::about()
{
  QString title(tr("About Patacrep! Songbook Client"));
  QString version = QCoreApplication::applicationVersion();

  QString description(tr("This program allows one to build customized songbooks from "
			 "<a href=\"http::www.patacrep.com\">www.patacrep.com</a>"));

  QStringList authorsList = QStringList() << "Crep (R. Goffe)"
                                          << "Lohrun (A. Dupas)"
                                          << "Carreau (M. Bussonnier)";
  QString authors = authorsList.join(", ");

  QMessageBox::about(this, title, tr("<p>%1</p>"
				     "<p><b>Version:</b> %2</p>"
				     "<p><b>Authors:</b> %3</p>")
		     .arg(description).arg(version).arg(authors));
}

void CMainWindow::build()
{
  if (!checkPdfLaTeX() || !checkPython())
    return;

  songbook()->songsFromSelection();
  if (songbook()->songs().isEmpty())
    {
      if (QMessageBox::question(this, windowTitle(), tr("You did not select any song. \n "
							"Do you want to build the songbook with all songs?"),
				QMessageBox::Yes,
				QMessageBox::No,
				QMessageBox::NoButton) == QMessageBox::No)
	return;
      else
	songbook()->checkAll();
    }

  save(true);

  if (QFile(songbook()->filename()).exists())
    {
      qobject_cast<QPlainTextEdit *>(log()->widget())->clear();
      make();
    }
  else
    {
      statusBar()->showMessage(tr("The songbook file %1 is invalid. Build aborted.")
			       .arg(songbook()->filename()));
    }
}

void CMainWindow::newSongbook()
{
  songbook()->reset();
  updateTitle(songbook()->filename());
}

void CMainWindow::open()
{
  QString filename = QFileDialog::getOpenFileName(this,
                                                  tr("Open"),
                                                  QString("%1/books").arg(workingPath()),
                                                  tr("Songbook (*.sb)"));
  songbook()->load(filename);
  updateTitle(songbook()->filename());
}

void CMainWindow::save(bool forced)
{
  if (songbook()->filename().isEmpty() ||
      songbook()->filename().endsWith("default.sb") ||
      !songbook()->filename().compare(".sb"))
    {
      if (forced)
	songbook()->setFilename(QString("%1/books/default.sb").arg(workingPath()));
      else if (!songbook()->filename().isEmpty())
	saveAs();
    }

  songbook()->save(songbook()->filename());
  updateTitle(songbook()->filename());
}

void CMainWindow::saveAs()
{
  QString filename = QFileDialog::getSaveFileName(this,
						  tr("Save as"),
						  QString("%1/books").arg(workingPath()),
						  tr("Songbook (*.sb)"));

  if (!filename.isEmpty())
    {
      songbook()->setFilename(filename);
      save();
    }
}

void CMainWindow::updateTitle(const QString &filename)
{
  QString text = filename.isEmpty() ? tr("New songbook") : filename;
  setWindowTitle(tr("%1 - %2[*]")
                 .arg(QCoreApplication::applicationName())
                 .arg(text));
}

const QString CMainWindow::workingPath()
{
  return library()->directory().canonicalPath();
}

void CMainWindow::make()
{
  makeClean(); //fix problems, don't remove
  m_builder->setWorkingDirectory(workingPath());

  QString basename = QFileInfo(songbook()->filename()).baseName();
  QString target = QString("%1.pdf").arg(basename);

  QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
  environment.insert("LATEX_OPTIONS", "-halt-on-error");
  m_builder->setProcessEnvironment(environment);

  QString command = buildCommand();
  m_builder->setCommand(command.replace("%target", target).replace("%basename", basename));

  m_builder->setUrlToOpen(QUrl::fromLocalFile((QString("%1/%2").arg(workingPath()).arg(target))));
  m_builder->setStartMessage(tr("Building %1.").arg(target));
  m_builder->setSuccessMessage(tr("%1 successfully built.").arg(target));
  m_builder->setErrorMessage(tr("Error during the building of %1, please check the log.").arg(target));

  m_builder->execute();
}

void CMainWindow::makeClean()
{
  m_builder->setWorkingDirectory(workingPath());
  m_builder->setCommand(cleanCommand());
  m_builder->setProcessEnvironment(QProcessEnvironment::systemEnvironment());

  m_builder->setUrlToOpen(QUrl());
  m_builder->setStartMessage(tr("Cleaning the build directory."));
  m_builder->setSuccessMessage(tr("Build directory cleaned."));
  m_builder->setErrorMessage(tr("Error during cleaning, please check the log."));

  m_builder->execute();
  m_builder->waitForFinished();
}

void CMainWindow::makeCleanall()
{
  m_builder->setWorkingDirectory(workingPath());
  m_builder->setCommand(cleanallCommand());
  m_builder->setProcessEnvironment(QProcessEnvironment::systemEnvironment());

  m_builder->setUrlToOpen(QUrl());
  m_builder->setStartMessage(tr("Cleaning the build directory."));
  m_builder->setSuccessMessage(tr("Build directory cleaned."));
  m_builder->setErrorMessage(tr("Error during cleaning, please check the log."));

  m_builder->execute();
  m_builder->waitForFinished();
}

void CMainWindow::cancelProcess()
{
  if (m_builder->state() == QProcess::Running)
    {
      m_builder->close();
      if (m_builder->command() == buildCommand())
	makeClean();
    }
}

CProgressBar * CMainWindow::progressBar() const
{
  return m_progressBar;
}

CSongbook * CMainWindow::songbook() const
{
  return m_songbook;
}

CLibraryView * CMainWindow::view() const
{
  return m_view;
}

CLibrary * CMainWindow::library() const
{
  return m_library;
}

QItemSelectionModel * CMainWindow::selectionModel()
{
  return view()->selectionModel();
}

void CMainWindow::songEditor(const QModelIndex &index)
{
  Q_UNUSED(index);
  if (!selectionModel()->hasSelection())
    {
      statusBar()->showMessage(tr("Please select a song to edit."));
      return;
    }

  QString path = view()->model()->data(selectionModel()->currentIndex(), CLibrary::PathRole).toString();

  songEditor(path);
}

void CMainWindow::songEditor(const QString &path)
{
  CSongEditor *editor = new CSongEditor(this);
  editor->setLibrary(library());
  editor->installHighlighter();
  if (!path.isEmpty())
    editor->setSong(library()->getSong(path));

  connect(editor, SIGNAL(labelChanged(const QString&)),
	  m_mainWidget, SLOT(changeTabText(const QString&)));
  m_mainWidget->addTab(editor);
}

void CMainWindow::newSong()
{
  songEditor(QString());
}

void CMainWindow::deleteSong()
{
  if (!selectionModel()->hasSelection())
    {
      statusBar()->showMessage(tr("Please select a song to remove."));
      return;
    }

  QString path = view()->model()->data(selectionModel()->currentIndex(), CLibrary::PathRole).toString();

  deleteSong(path);
}

void CMainWindow::deleteSong(const QString &path)
{
  int ret = QMessageBox::warning(this, tr("Songbook-Client"),
				 tr("This file will be deleted:\n%1\n"
				    "Are you sure?").arg(path),
				 QMessageBox::Cancel | QMessageBox::Ok,
				 QMessageBox::Cancel);

  if (ret == QMessageBox::Ok)
    library()->deleteSong(path);
}

void CMainWindow::closeTab(int index)
{
  if (CSongEditor *editor = qobject_cast< CSongEditor* >(m_mainWidget->widget(index)))
    if (editor->close())
      {
	m_mainWidget->closeTab(index);
	delete editor;
      }
}

void CMainWindow::changeTab(int index)
{
  m_editorMenu->clear();
  CSongEditor *editor = qobject_cast< CSongEditor* >(m_mainWidget->widget(index));
  if (editor != 0)
    {
      editor->actionGroup()->setEnabled(true);
      editor->setSpellCheckAvailable(editor->isSpellCheckAvailable());
      switchToolBar(editor->toolBar());
      m_saveAct->setShortcutContext(Qt::WidgetShortcut);
    }
  else
    {
      editor = m_voidEditor;
      editor->actionGroup()->setEnabled(false);
      switchToolBar(m_libraryToolBar);
      m_saveAct->setShortcutContext(Qt::WindowShortcut);
    }
  m_editorMenu->addActions(editor->actionGroup()->actions());
}

QDockWidget* CMainWindow::log() const
{
  return m_log;
}

void CMainWindow::buildError(QProcess::ProcessError error)
{
  Q_UNUSED(error);
  log()->setVisible(true);
  statusBar()->showMessage
    (qobject_cast< CMakeSongbookProcess* >(QObject::sender())->errorMessage());
}

void CMainWindow::updateNotification(const QString &path)
{
  if (!m_updateAvailable)
    {
      m_updateAvailable = new CNotification(this);
      m_updateAvailable->addAction(m_libraryUpdateAct);
    }

  m_updateAvailable->setMessage(tr("<strong>The following directory has been modified:</strong><br/>"
                                   "  %1 <br/>"
                                   "Do you want to update the library to reflect these changes?").arg(path));
}

void CMainWindow::noDataNotification(const QDir &directory)
{
  if (!m_noDataInfo)
    {
      m_noDataInfo = new CNotification(this);
      m_noDataInfo->addAction(m_libraryDownloadAct);
    }

  if (library()->rowCount() > 0)
    {
      m_noDataInfo->hide();
      m_buildAct->setEnabled(true);
    }
  else
    {
      m_noDataInfo->setMessage(tr("<strong>The directory <b>%1</b> does not contain any song.</strong><br/>"
                                  "Do you want to download the latest songs library?").arg(directory.canonicalPath()));
      m_noDataInfo->show();
      m_buildAct->setEnabled(false);
    }
}

void CMainWindow::downloadDialog()
{
#ifdef ENABLE_LIBRARY_DOWNLOAD
  CLibraryDownload *libraryDownload = new CLibraryDownload(this);
  libraryDownload->exec();
#endif
}

void CMainWindow::cleanDialog()
{
  QDialog dialog(this);
  dialog.setWindowTitle(tr("Clean"));

  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
						     QDialogButtonBox::Cancel);
  connect(buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), &dialog, SLOT(close()));

  if (!m_tempFilesmodel)
    {
      m_tempFilesmodel = new QFileSystemModel;
      m_tempFilesmodel->setRootPath(library()->directory().canonicalPath());
      m_tempFilesmodel->setNameFilters(QStringList()
				       << "*.aux" << "*.d" << "*.toc" << "*.out"
				       << "*.log" << "*.nav" << "*.snm" << "*.sbx" << "*.sxd");
      m_tempFilesmodel->setNameFilterDisables(false);
      m_tempFilesmodel->setFilter(QDir::Files);
    }

  QListView *view = new QListView;
  view->setModel(m_tempFilesmodel);
  view->setRootIndex(m_tempFilesmodel->index(library()->directory().canonicalPath()));

  QCheckBox* cleanAllButton = new QCheckBox("Also remove pdf files", this);
  updateTempFilesView(cleanAllButton->checkState());
  connect(cleanAllButton, SIGNAL(stateChanged(int)), this, SLOT(updateTempFilesView(int)));

  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(view);
  layout->addWidget(cleanAllButton);
  layout->addWidget(buttonBox);
  dialog.setLayout(layout);

  if (dialog.exec() == QDialog::Accepted)
    {
      if (cleanAllButton->isChecked())
	makeCleanall();
      else
	makeClean();
    }
}

void CMainWindow::updateTempFilesView(int state)
{
  if (state == Qt::Checked)
    {
      m_tempFilesmodel->setNameFilters(m_tempFilesmodel->nameFilters() << "*.pdf");
    }
  else if (m_tempFilesmodel->nameFilters().contains("*.pdf"))
    {
      QStringList list  = m_tempFilesmodel->nameFilters();
      list.removeLast();
      m_tempFilesmodel->setNameFilters(list);
    }
}

const QString & CMainWindow::buildCommand() const
{
  return m_buildCommand;
}

void CMainWindow::setBuildCommand(const QString &command)
{
  m_buildCommand = command;
}

const QString & CMainWindow::cleanCommand() const
{
  return m_cleanCommand;
}

void CMainWindow::setCleanCommand(const QString &command)
{
  m_cleanCommand = command;
}

const QString & CMainWindow::cleanallCommand() const
{
  return m_cleanallCommand;
}

void CMainWindow::setCleanallCommand(const QString &command)
{
  m_cleanallCommand = command;
}
