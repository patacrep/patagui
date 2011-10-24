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

#include <QtGui>

#include "utils/utils.hh"
#include "label.hh"
#include "library.hh"
#include "library-view.hh"
#include "songbook.hh"
#include "song-editor.hh"
#include "highlighter.hh"
#include "dialog-new-song.hh"
#include "filter-lineedit.hh"
#include "song-sort-filter-proxy-model.hh"
#include "tab-widget.hh"
#include "notification.hh"
#include "song-item-delegate.hh"
#include "preferences.hh"

#include "config.hh"

#ifdef ENABLE_LIBRARY_DOWNLOAD
#include "library-download.hh"
#endif // ENABLE_LIBRARY_DOWNLOAD

#include "make-songbook-process.hh"

#include <QDebug>

using namespace SbUtils;

CMainWindow::CMainWindow()
  : QMainWindow()
  , m_library(0)
  , m_view(0)
  , m_songbook(0)
  , m_proxyModel(0)
  , m_progressBar(0)
  , m_noDataInfo(0)
  , m_updateAvailable(0)
  , m_infoSelection(0)
{
  setWindowTitle("Patacrep Songbook Client");
  setWindowIcon(QIcon(":/icons/songbook-client.png"));

  // song library
  m_library = new CLibrary(this);

  connect(m_library, SIGNAL(directoryChanged(const QDir &)),
	  SLOT(noDataNotification(const QDir &)));

  // songbook (title, authors, song list)
  m_songbook = new CSongbook(this);
  m_songbook->setLibrary(m_library);

  connect(m_songbook, SIGNAL(wasModified(bool)), SLOT(setWindowModified(bool)));
  connect(m_songbook, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
	  SLOT(selectedSongsChanged(const QModelIndex &, const QModelIndex &)));

  // proxy model (sorting & filtering)
  m_proxyModel = new CSongSortFilterProxyModel(this);
  m_proxyModel->setSourceModel(m_songbook);
  m_proxyModel->setSortLocaleAware(true);
  m_proxyModel->setDynamicSortFilter(true);
  m_proxyModel->setFilterKeyColumn(-1);

  // view
  m_view = new CLibraryView(this);
  m_view->setModel(m_proxyModel);
  m_view->setItemDelegate(new CSongItemDelegate);
  m_view->resizeColumns();
  connect(m_library, SIGNAL(wasModified()), m_view, SLOT(update()));
  
  // compilation log
  m_log = new QPlainTextEdit;
  m_log->setMinimumHeight(150);
  m_log->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
  m_log->setReadOnly(true);
  Q_UNUSED(new CHighlighter(m_log->document()));

  createActions();
  createMenus();
  createToolBar();

  //Layouts
  QBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->setContentsMargins(0,0,0,0);
  mainLayout->addWidget(m_view);
  mainLayout->addWidget(m_log);

  QWidget *libraryTab = new QWidget;
  libraryTab->setLayout(mainLayout);

  // place elements into the main window
  m_mainWidget = new CTabWidget;
  m_mainWidget->setTabsClosable(true);
  m_mainWidget->setMovable(true);
  m_mainWidget->setSelectionBehaviorOnAdd(CTabWidget::SelectNew);
  connect(m_mainWidget, SIGNAL(tabCloseRequested(int)), SLOT(closeTab(int)));
  connect(m_mainWidget, SIGNAL(currentChanged(int)), SLOT(changeTab(int)));
  m_mainWidget->addTab(libraryTab, tr("Library"));
  setCentralWidget(m_mainWidget);

  // status bar with an embedded label and progress bar
  m_infoSelection = new QLabel(this);
  statusBar()->addPermanentWidget(m_infoSelection);

  m_progressBar = new QProgressBar(this);
  m_progressBar->setTextVisible(false);
  m_progressBar->setRange(0, 0);
  m_progressBar->hide();
  statusBar()->addPermanentWidget(m_progressBar);

  updateTitle(songbook()->filename());

  readSettings();
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

void CMainWindow::readSettings()
{
  QSettings settings;
  settings.beginGroup("general");
  resize(settings.value("size", QSize(800,600)).toSize());
  setStatusbarDisplayed(settings.value("statusBar", true).toBool());
  setToolBarDisplayed(settings.value("toolBar", true).toBool());
  settings.endGroup();

  settings.beginGroup("display");
  log()->setVisible(settings.value("logs", false).toBool());
  settings.endGroup();

  setBuildCommand(settings.value("buildCommand", PLATFORM_BUILD_COMMAND).toString());
  setCleanCommand(settings.value("cleanCommand", PLATFORM_CLEAN_COMMAND).toString());
  setCleanallCommand(settings.value("cleanallCommand", PLATFORM_CLEAN_COMMAND).toString());

  settings.beginGroup("tools");
#ifdef Q_WS_WIN
  setBuildCommand(settings.value("buildCommand", "cmd.exe /C make.bat %basename").toString());
  setCleanCommand(settings.value("cleanCommand", "cmd.exe /C clean.bat").toString());
  setCleanallCommand(settings.value("cleanallCommand", "cmd.exe /C clean.bat").toString());
#else //Apple/Linux
  setBuildCommand(settings.value("buildCommand", "make %target").toString());
  setCleanCommand(settings.value("cleanCommand", "make clean").toString());
  setCleanallCommand(settings.value("cleanallCommand", "make cleanall").toString());
#endif
  settings.endGroup();

  library()->readSettings();
  view()->readSettings();
}

void CMainWindow::writeSettings()
{
  QSettings settings;
  settings.beginGroup("general");
  settings.setValue("size", size());
  settings.setValue("statusBar", isStatusbarDisplayed());
  settings.setValue("toolBar", isToolBarDisplayed());
  settings.endGroup();

  library()->writeSettings();
  view()->writeSettings();
}

void CMainWindow::selectedSongsChanged(const QModelIndex &, const QModelIndex &)
{
  m_infoSelection->setText(QString(tr("Selection: %1/%2"))
			   .arg(songbook()->selectedCount())
			   .arg(songbook()->rowCount()));
}

void CMainWindow::createActions()
{
  m_newSongAct = new QAction(tr("New Song"), this);
  m_newSongAct->setIcon(QIcon::fromTheme("list-add", QIcon(":/icons/tango/32x32/actions/document-new.png")));
  m_newSongAct->setStatusTip(tr("Write a new song"));
  m_newSongAct->setIconText(tr("Add"));
  connect(m_newSongAct, SIGNAL(triggered()), this, SLOT(newSong()));

  m_newAct = new QAction(tr("New"), this);
  m_newAct->setIcon(QIcon::fromTheme("folder-new", QIcon(":/icons/tango/32x32/actions/folder-new.png")));
  m_newAct->setShortcut(QKeySequence::New);
  m_newAct->setStatusTip(tr("Create a new songbook"));
  connect(m_newAct, SIGNAL(triggered()), this, SLOT(newSongbook()));

  m_openAct = new QAction(tr("Open..."), this);
  m_openAct->setIcon(QIcon::fromTheme("document-open", QIcon(":/icons/tango/32x32/actions/document-open.png")));
  m_openAct->setShortcut(QKeySequence::Open);
  m_openAct->setStatusTip(tr("Open a songbook"));
  connect(m_openAct, SIGNAL(triggered()), this, SLOT(open()));

  m_saveAct = new QAction(tr("Save"), this);
  m_saveAct->setShortcut(QKeySequence::Save);
  m_saveAct->setIcon(QIcon::fromTheme("document-save", QIcon(":/icons/tango/32x32/actions/document-save.png")));
  m_saveAct->setStatusTip(tr("Save the current songbook"));
  connect(m_saveAct, SIGNAL(triggered()), this, SLOT(save()));

  m_saveAsAct = new QAction(tr("Save As..."), this);
  m_saveAsAct->setShortcut(QKeySequence::SaveAs);
  m_saveAsAct->setIcon(QIcon::fromTheme("document-save-as", QIcon(":/icons/tango/32x32/actions/document-save-as.png")));
  m_saveAsAct->setStatusTip(tr("Save the current songbook with a different name"));
  connect(m_saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

  m_documentationAct = new QAction(tr("Online documentation"), this);
  m_documentationAct->setShortcut(QKeySequence::HelpContents);
  m_documentationAct->setIcon(QIcon::fromTheme("help-contents"));
  m_documentationAct->setStatusTip(tr("Download documentation pdf file "));
  connect(m_documentationAct, SIGNAL(triggered()), this, SLOT(documentation()));

  m_aboutAct = new QAction(tr("&About"), this);
  m_aboutAct->setIcon(QIcon::fromTheme("help-about"));
  m_aboutAct->setStatusTip(tr("About this application"));
  m_aboutAct->setMenuRole(QAction::AboutRole);
  connect(m_aboutAct, SIGNAL(triggered()), this, SLOT(about()));

  m_exitAct = new QAction(tr("Quit"), this);
  m_exitAct->setIcon(QIcon::fromTheme("application-exit"));
  m_exitAct->setShortcut(QKeySequence::Quit);
  m_exitAct->setStatusTip(tr("Quit the program"));
  m_exitAct->setMenuRole(QAction::QuitRole);
  connect(m_exitAct, SIGNAL(triggered()), this, SLOT(close()));

  m_preferencesAct = new QAction(tr("&Preferences"), this);
  m_preferencesAct->setIcon(QIcon::fromTheme("document-properties"));
  m_preferencesAct->setStatusTip(tr("Configure the application"));
  m_preferencesAct->setMenuRole(QAction::PreferencesRole);
  connect(m_preferencesAct, SIGNAL(triggered()), SLOT(preferences()));

  m_selectAllAct = new QAction(tr("Check all"), this);
  m_selectAllAct->setIcon(QIcon::fromTheme("select_all",QIcon(":/icons/tango/48x48/songbook/select_all.png")));
  m_selectAllAct->setStatusTip(tr("Check all songs"));
  connect(m_selectAllAct, SIGNAL(triggered()), m_proxyModel, SLOT(checkAll()));

  m_unselectAllAct = new QAction(tr("Uncheck all"), this);
  m_unselectAllAct->setIcon(QIcon::fromTheme("select_none",QIcon(":/icons/tango/48x48/songbook/select_none.png")));
  m_unselectAllAct->setStatusTip(tr("Uncheck all songs"));
  connect(m_unselectAllAct, SIGNAL(triggered()), m_proxyModel, SLOT(uncheckAll()));

  m_invertSelectionAct = new QAction(tr("Toggle all"), this);
  m_invertSelectionAct->setIcon(QIcon::fromTheme("select_invert",QIcon(":/icons/tango/48x48/songbook/select_invert.png")));
  m_invertSelectionAct->setStatusTip(tr("Toggle the checked state of all songs"));
  connect(m_invertSelectionAct, SIGNAL(triggered()), m_proxyModel, SLOT(toggleAll()));

  m_adjustColumnsAct = new QAction(tr("Auto Adjust Columns"), this);
  m_adjustColumnsAct->setStatusTip(tr("Adjust columns to contents"));
  connect(m_adjustColumnsAct, SIGNAL(triggered()),
          view(), SLOT(resizeColumnsToContents()));

  m_libraryUpdateAct = new QAction(tr("Update"), this);
  m_libraryUpdateAct->setStatusTip(tr("Update current song list from \".sg\" files"));
  m_libraryUpdateAct->setIcon(QIcon::fromTheme("view-refresh", QIcon(":/icons/tango/32x32/actions/view-refresh.png")));
  m_libraryUpdateAct->setShortcut(QKeySequence::Refresh);
  connect(m_libraryUpdateAct, SIGNAL(triggered()), library(), SLOT(update()));

  m_libraryDownloadAct = new QAction(tr("Download"), this);
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

  m_buildAct = new QAction(tr("Build PDF"), this);
  m_buildAct->setIcon(QIcon::fromTheme("document-export",QIcon(":/icons/tango/32x32/mimetypes/document-export.png")));
  m_buildAct->setStatusTip(tr("Generate pdf from selected songs"));
  connect(m_buildAct, SIGNAL(triggered()), this, SLOT(build()));

  m_cleanAct = new QAction(tr("Clean"), this);
  m_cleanAct->setIcon(QIcon::fromTheme("edit-clear", QIcon(":/icons/tango/32x32/actions/edit-clear")));
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

bool CMainWindow::isToolBarDisplayed( )
{
  return m_isToolBarDisplayed;
}

void CMainWindow::setStatusbarDisplayed(bool value)
{
  m_isStatusbarDisplayed = value;
  statusBar()->setVisible(value);
}

bool CMainWindow::isStatusbarDisplayed( )
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
  CSongEditor *editor = new CSongEditor();
  QAction *action;
  foreach (action, editor->actions())
    {
      action->setDisabled(true);
      m_editorMenu->addAction(action);
    }

  QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
  viewMenu->addAction(m_toolBarViewAct);
  viewMenu->addAction(m_statusbarViewAct);
  viewMenu->addAction(m_adjustColumnsAct);

  QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
  helpMenu->addAction(m_documentationAct);
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
  QDesktopServices::openUrl(QUrl("http://www.patacrep.com/data/documents/doc.pdf"));
}

void CMainWindow::about()
{
  QString title = QString(tr("About Patacrep! Songbook Client"));
  QString version = QCoreApplication::applicationVersion();

  QString description = QString(tr("This program allows to build customized songbooks from "
				   "<a href=\"http::www.patacrep.com\">www.patacrep.com</a>"));

  QStringList authorsList = QStringList() << "Crep (R. Goffe)"
                                          << "Lohrun (A. Dupas)"
                                          << "Carreau (M. Bussonnier)";
  QString authors = authorsList.join(", ");

  QMessageBox::about(this, title, QString(tr("<p>%1</p>"
					     "<p><b>Version:</b> %2</p>"
					     "<p><b>Authors:</b> %3</p>"))
		     .arg(description).arg(version).arg(authors));
}

void CMainWindow::build()
{
  songbook()->songsFromSelection();
  if (songbook()->songs().isEmpty())
    {
      if (QMessageBox::question(this, windowTitle(),
				QString(tr("You did not select any song. \n "
					   "Do you want to build the songbook with all songs?")),
				QMessageBox::Yes,
				QMessageBox::No,
				QMessageBox::NoButton) == QMessageBox::No)
	return;
      else
	songbook()->checkAll();
    }

  save(true);
  
  if (!QFile(songbook()->filename()).exists())
    statusBar()->showMessage(QString(tr("The songbook file %1 is invalid. Build aborted."))
			     .arg(songbook()->filename()));

  QString basename = QFileInfo(songbook()->filename()).baseName();
  QString target = QString("%1.pdf").arg(basename);

  CMakeSongbookProcess *builder = new CMakeSongbookProcess(this);
  builder->setWorkingDirectory(workingPath());

  connect(builder, SIGNAL(aboutToStart()),
          progressBar(), SLOT(show()));
  connect(builder, SIGNAL(aboutToStart()),
          statusBar(), SLOT(clear()));
  connect(builder, SIGNAL(message(const QString &, int)), statusBar(),
          SLOT(showMessage(const QString &, int)));
  connect(builder, SIGNAL(finished(int, QProcess::ExitStatus)),
          progressBar(), SLOT(hide()));
  connect(builder, SIGNAL(readOnStandardOutput(const QString &)),
          log(), SLOT(appendPlainText(const QString &)));
  connect(builder, SIGNAL(readOnStandardError(const QString &)),
          log(), SLOT(appendPlainText(const QString &)));

  builder->setCommand(cleanCommand());

  builder->setStartMessage(tr("Cleaning the build directory."));
  builder->setSuccessMessage(tr("Build directory cleaned."));
  builder->setErrorMessage(tr("Error during cleaning, please check the log."));

  builder->execute();
  builder->waitForFinished();

  QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
  environment.insert("LATEX_OPTIONS", "-halt-on-error");
  builder->setProcessEnvironment(environment);

  QString command = buildCommand();
  builder->setCommand(command.replace("%target", target).replace("%basename", basename));

  builder->setUrlToOpen(QUrl(QString("file:///%1/%2").arg(workingPath()).arg(target)));
  builder->setStartMessage(tr("Building %1.").arg(target));
  builder->setSuccessMessage(tr("%1 successfully built.").arg(target));
  builder->setErrorMessage(tr("Error during the building of %1, please check the log.").arg(target));

  builder->execute();
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
  if (songbook()->filename().isEmpty() || songbook()->filename().endsWith("default.sb"))
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
						  workingPath(),
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

QProgressBar * CMainWindow::progressBar() const
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
  if (!selectionModel()->hasSelection())
    {
      statusBar()->showMessage(tr("Please select a song to edit."));
      return;
    }

  QString path = view()->model()->data(selectionModel()->currentIndex(), CLibrary::PathRole).toString();
  QString title = view()->model()->data(selectionModel()->currentIndex(), CLibrary::TitleRole).toString();

  songEditor(path, title);
}

void CMainWindow::songEditor(const QString &path, const QString &title)
{
  if (m_editors.contains(path))
    {
      m_mainWidget->setCurrentWidget(m_editors[path]);
      return;
    }

  CSongEditor *editor = new CSongEditor();
  editor->setPath(path);
  if (title == QString())
    {
      QFileInfo fileInfo(path);
      editor->setWindowTitle(fileInfo.fileName());
    }
  else
    {
      editor->setWindowTitle(title);
    }

  connect(editor, SIGNAL(labelChanged(const QString&)),
	  m_mainWidget, SLOT(changeTabText(const QString&)));

  m_mainWidget->addTab(editor);
  m_editors.insert(path, editor);
}

void CMainWindow::newSong()
{
  CDialogNewSong *dialog = new CDialogNewSong(this);

  if (dialog->exec() == QDialog::Accepted)
    {
      library()->update();
      songEditor(dialog->path(), dialog->title());
    }
  delete dialog;
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
  QString qs(tr("You are about to remove a song from the library.\n"
                "Yes : The song will only be deleted from the library "
		"and can be retrieved by rebuilding the library\n"
                "No  : Nothing will be deleted\n"
                "Delete file : You will also delete %1 from your hard drive\n"
                "If you are unsure what to do, click No.").arg(path));
  QMessageBox msgBox;
  msgBox.setIcon(QMessageBox::Question);
  msgBox.setText(tr("Removing song from Library."));
  msgBox.setInformativeText(tr("Are you sure?"));
  msgBox.addButton(QMessageBox::No);
  QPushButton *yesb = msgBox.addButton(QMessageBox::Yes);
  QPushButton *delb = msgBox.addButton(tr("Delete file"),QMessageBox::DestructiveRole);
  msgBox.setDefaultButton(QMessageBox::No);
  msgBox.setDetailedText(qs);
  msgBox.exec();

  if (msgBox.clickedButton() == yesb || msgBox.clickedButton() == delb)
    {
      //remove entry in database in 2 case
      library()->removeSong(path);
    }
  //don't forget to remove the file if asked
  if (msgBox.clickedButton() == delb)
    {
      //removal on disk only if deletion
      QFile file(path);
      QFileInfo fileinfo(file);
      QString tmp = fileinfo.canonicalPath();
      if (file.remove())
	{
	  QDir dir;
	  dir.rmdir(tmp); //remove dir if empty
	}
    }
}

void CMainWindow::closeTab(int index)
{
  CSongEditor *editor = qobject_cast< CSongEditor* >(m_mainWidget->widget(index));
  if (editor)
    {
      if (editor->document()->isModified())
	{
	  QMessageBox::StandardButton answer = 
	    QMessageBox::question(this,
				  tr("Close"),
				  tr("There is unsaved modification in the current editor, do you really want to close it?"),
				  QMessageBox::Ok | QMessageBox::Cancel,
				  QMessageBox::Cancel);
	  if (answer != QMessageBox::Ok)
	    return;
	}
      m_editors.remove(editor->path());
      m_mainWidget->closeTab(index);
    }
}

void CMainWindow::changeTab(int index)
{
  CSongEditor *editor = qobject_cast< CSongEditor* >(m_mainWidget->widget(index));
  QAction *action;
  if (editor)
    {
      m_editorMenu->clear();
      foreach (action, editor->actions())
	{
	  m_editorMenu->addAction(action);
	  action->setEnabled(true);
	}

      switchToolBar(editor->toolBar());
      m_saveAct->setShortcutContext(Qt::WidgetShortcut);
    }
  else
    {
      foreach (action, m_editorMenu->actions())
	{
	  action->setEnabled(false);
	}

      switchToolBar(m_libraryToolBar);
      m_saveAct->setShortcutContext(Qt::WindowShortcut);
    }
}

QPlainTextEdit* CMainWindow::log() const
{
  return m_log;
}

void CMainWindow::updateNotification(const QString &path)
{
  if (!m_updateAvailable)
    {
      m_updateAvailable = new CNotification(this);
      m_updateAvailable->addAction(m_libraryUpdateAct);
    }

  m_updateAvailable->setMessage
    (QString(tr("<strong>The following directory has been modified:</strong><br/>"
		"  %1 <br/>"
		"Do you want to update the library to reflect these changes?")).arg(path));
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
    }
  else
    {
      m_noDataInfo->setMessage
	(QString(tr("<strong>The directory <b>%1</b> does not contain any song.</strong><br/>"
		    "Do you want to download the latest songs library?").arg(directory.canonicalPath())));
      m_noDataInfo->show();
    }
}

void CMainWindow::downloadDialog()
{
  CLibraryDownload libraryDownload(this);
  libraryDownload.exec();
}

void CMainWindow::cleanDialog()
{
  QDialog *dialog = new QDialog(this);
  dialog->setWindowTitle(tr("Remove temporary files"));

  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
						     QDialogButtonBox::Cancel);
  connect( buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()) );
  connect( buttonBox, SIGNAL(rejected()), dialog, SLOT(close()) );

  m_tempFilesmodel = new QFileSystemModel;
  m_tempFilesmodel->setRootPath(library()->directory().canonicalPath());
  m_tempFilesmodel->setNameFilters(QStringList()
			<< "*.aux" << "*.d" << "*.toc" << "*.out"
			<< "*.log" << "*.nav" << "*.snm" << "*.sbx" << "*.sxd");
  m_tempFilesmodel->setNameFilterDisables(false);
  m_tempFilesmodel->setFilter(QDir::Files);

  QListView* view = new QListView;
  view->setModel(m_tempFilesmodel);
  view->setRootIndex(m_tempFilesmodel->index(library()->directory().canonicalPath()));

  QCheckBox* cleanAllButton = new QCheckBox("Also remove pdf files", this);
  connect(cleanAllButton, SIGNAL(stateChanged(int)), this, SLOT(updateTempFilesView(int)));

  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(view);
  layout->addWidget(cleanAllButton);
  layout->addWidget(buttonBox);
  dialog->setLayout(layout);

  if (dialog->exec() == QDialog::Accepted)
    {
      CMakeSongbookProcess *builder = new CMakeSongbookProcess(this);
      builder->setWorkingDirectory(workingPath());

      connect(builder, SIGNAL(aboutToStart()),
              progressBar(), SLOT(show()));
      connect(builder, SIGNAL(aboutToStart()),
              statusBar(), SLOT(clear()));
      connect(builder, SIGNAL(message(const QString &, int)), statusBar(),
              SLOT(showMessage(const QString &, int)));
      connect(builder, SIGNAL(finished(int, QProcess::ExitStatus)),
              progressBar(), SLOT(hide()));
      connect(builder, SIGNAL(readOnStandardOutput(const QString &)),
              log(), SLOT(appendPlainText(const QString &)));
      connect(builder, SIGNAL(readOnStandardError(const QString &)),
              log(), SLOT(appendPlainText(const QString &)));

      if(cleanAllButton->isChecked())
	builder->setCommand(cleanallCommand());
      else
	builder->setCommand(cleanCommand());

      builder->setStartMessage(tr("Cleaning the build directory."));
      builder->setSuccessMessage(tr("Build directory cleaned."));
      builder->setErrorMessage(tr("Error during cleaning, please check the log."));

      builder->execute();
    }
  delete dialog;
  delete m_tempFilesmodel;
}

void CMainWindow::updateTempFilesView(int state)
{
  if(state == Qt::Checked)
    {
      m_tempFilesmodel->setNameFilters(m_tempFilesmodel->nameFilters() << "*.pdf");
    }
  else if(m_tempFilesmodel->nameFilters().contains("*.pdf"))
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
  return m_cleanCommand;
}

void CMainWindow::setCleanallCommand(const QString &command)
{
  m_cleanCommand = command;
}
