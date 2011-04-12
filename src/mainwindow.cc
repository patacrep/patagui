// Copyright (C) 2009-2011 Romain Goffe, Alexandre Dupas
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
#include <QtGui>
#include <QtSql>
#include <QtAlgorithms>
#include <QDebug>

#include <assert.h>
#include "utils/utils.hh"
#include "label.hh"
#include "mainwindow.hh"
#include "preferences.hh"
#include "library.hh"
#include "songbook.hh"
#include "build-engine/resize-covers.hh"
#include "build-engine/latex-preprocessing.hh"
#include "build-engine/make-songbook.hh"
#include "song-editor.hh"
#include "highlighter.hh"
#include "dialog-new-song.hh"
#include "filter-lineedit.hh"
#include "songSortFilterProxyModel.hh"
#include "tab-widget.hh"
#include "library-download.hh"

using namespace SbUtils;

//******************************************************************************
CMainWindow::CMainWindow()
  : QMainWindow()
  , m_library()
  , m_proxyModel()
  , m_songbook()
  , m_sbInfoSelection(new CLabel)
  , m_sbInfoTitle(new CLabel)
  , m_sbInfoAuthors(new CLabel)
  , m_sbInfoStyle(new CLabel)
  , m_view()
  , m_progressBar(new QProgressBar(this))
  , m_isToolBarDisplayed(true)
  , m_isStatusbarDisplayed(true)
  , m_cover(new QPixmap)
{
  setWindowTitle("Patacrep Songbook Client");
  setWindowIcon(QIcon(":/icons/patacrep.png"));

  // initialize the database connection
  connectDatabase();

  // create and load song library
  m_library = new CLibrary(this);

  m_proxyModel = new CSongSortFilterProxyModel;
  m_proxyModel->setSourceModel(m_library);
  m_proxyModel->setSortLocaleAware(true);
  m_proxyModel->setDynamicSortFilter(true);
  m_proxyModel->setFilterKeyColumn(-1);

  m_view = new QTableView(this);
  m_view->setModel(m_proxyModel);
  m_view->setShowGrid(false);
  m_view->setAlternatingRowColors(true);
  m_view->setSelectionMode(QAbstractItemView::MultiSelection);
  m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_view->setSortingEnabled(true);
  m_view->verticalHeader()->setVisible(false);
  m_view->horizontalHeader()->setStretchLastSection(true);

  readSettings();

  connect(m_library, SIGNAL(wasModified()), SLOT(updateView()));
  connect(m_library, SIGNAL(wasModified()), SLOT(selectionChanged()));

  // songbook
  m_songbook = new CSongbook;
  m_songbook->setWorkingPath(workingPath());
  connect(m_songbook, SIGNAL(wasModified(bool)), SLOT(setWindowModified(bool)));
  connect(this, SIGNAL(workingPathChanged(const QString&)),
	  songbook(), SLOT(setWorkingPath(const QString&)));

  // compilation log
  m_log = new QTextEdit;
  m_log->setMaximumHeight(150);
  m_log->setReadOnly(true);
  new CHighlighter(m_log->document());

  // no data info widget
  m_noDataInfo = new QTextEdit;
  m_noDataInfo->setMaximumHeight(150);
  m_noDataInfo->setReadOnly(true);
  m_noDataInfo->
    setHtml(tr("<table><tr><td valign=middle>  "
	       "<img src=\":/icons/attention.png\" />  </td><td>"
	       "<p>The directory <b>%1</b> does not contain any song file (\".sg\").<br/><br/> "
	       "You may :<ul><li>select a valid directory in the menu <i>Edit/Preferences</i></li>"
	       "<li>use the menu <i>Library/Download</i> to get the latest git snapshot</li>"
	       "<li>manually download the latest tarball on "
	       "<a href=\"http://www.patacrep.com/static1/downloads\">"
	       "patacrep.com</a></li></ul>"
	       "</p></td></tr></table>").arg(workingPath()));
  m_noDataInfo->hide();

  createActions();
  createMenus();
  createToolBar();

  connect(selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
	  this, SLOT(selectionChanged(const QItemSelection&, const QItemSelection&)));

  //Layouts
  QBoxLayout *mainLayout = new QVBoxLayout;
  QBoxLayout *dataLayout = new QVBoxLayout;
  QBoxLayout *centerLayout = new QHBoxLayout;
  QBoxLayout *leftLayout = new QVBoxLayout;
  leftLayout->addWidget(new QLabel(tr("<b>Song</b>")));
  leftLayout->addLayout(songInfo());
  leftLayout->addWidget(new QLabel(tr("<b>Songbook</b>")));
  leftLayout->addLayout(songbookInfo());
  leftLayout->addStretch();
  dataLayout->addWidget(view());
  dataLayout->addWidget(m_noDataInfo);
  centerLayout->addLayout(leftLayout);
  centerLayout->addLayout(dataLayout);
  centerLayout->setStretch(1,2);
  mainLayout->addLayout(centerLayout);
  mainLayout->addWidget(log());

  QWidget* libraryTab = new QWidget;
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

  // status bar with an embedded progress bar on the right
  progressBar()->setTextVisible(false);
  progressBar()->setRange(0, 0);
  progressBar()->hide();
  statusBar()->addPermanentWidget(progressBar());

  applySettings();

  updateTitle(songbook()->filename());
  updateView();
  selectionChanged();
  songbook()->panel();
  updateSongbookLabels();
}
//------------------------------------------------------------------------------
CMainWindow::~CMainWindow()
{
  delete m_library;
  delete m_songbook;

  disconnectDatabase();
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
//------------------------------------------------------------------------------
void CMainWindow::readSettings()
{
  QSettings settings;

  resize(settings.value("mainWindow/size", QSize(800,600)).toSize());

  setWorkingPath(settings.value("workingPath", QDir::home().path()).toString());

  settings.beginGroup("display");
  m_displayColumnArtist = settings.value("artist", true).toBool();
  m_displayColumnTitle = settings.value("title", true).toBool();
  m_displayColumnPath = settings.value("path", false).toBool();
  m_displayColumnAlbum = settings.value("album", true).toBool();
  m_displayColumnLilypond = settings.value("lilypond", false).toBool();
  m_displayColumnCover = settings.value("cover", false).toBool();
  m_displayColumnLang = settings.value("lang", true).toBool();
  m_displayCompilationLog = settings.value("log", false).toBool();
  settings.endGroup();
}
//------------------------------------------------------------------------------
void CMainWindow::writeSettings()
{
  QSettings settings;
  settings.setValue("mainWindow/size", size());
}
//------------------------------------------------------------------------------
void CMainWindow::applySettings()
{
  view()->setColumnHidden(0,!m_displayColumnArtist);
  view()->setColumnHidden(1,!m_displayColumnTitle);
  view()->setColumnHidden(2,!m_displayColumnLilypond);
  view()->setColumnHidden(3,!m_displayColumnPath);
  view()->setColumnHidden(4,!m_displayColumnAlbum);
  view()->setColumnHidden(5,!m_displayColumnCover);
  view()->setColumnHidden(6,!m_displayColumnLang);
  view()->setColumnWidth(0,200);
  view()->setColumnWidth(1,200);
  view()->setColumnWidth(4,200);
  log()->setVisible(m_displayCompilationLog);
}
//------------------------------------------------------------------------------
void CMainWindow::templateSettings()
{
  QDialog *dialog = new QDialog;
  dialog->setWindowTitle(tr("Songbook settings"));
  QVBoxLayout *layout = new QVBoxLayout;

  QScrollArea *songbookScrollArea = new QScrollArea();
  songbookScrollArea->setMinimumWidth(400);
  songbookScrollArea->setWidget(songbook()->panel());
  songbookScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  QDialogButtonBox *buttonBox = new QDialogButtonBox;

  QPushButton *button = new QPushButton(tr("Reset"));
  connect( button, SIGNAL(clicked()), songbook(), SLOT(reset()) );
  buttonBox->addButton(button, QDialogButtonBox::ResetRole);

  button = new QPushButton(tr("Ok"));
  button->setDefault(true);
  connect( button, SIGNAL(clicked()), dialog, SLOT(accept()) );
  buttonBox->addButton(button, QDialogButtonBox::ActionRole);

  connect( dialog, SIGNAL(accepted()), this, SLOT(updateSongbookLabels()) );

  layout->addWidget(songbookScrollArea);
  layout->addWidget(buttonBox);
  dialog->setLayout(layout);
  dialog->show();
}
//------------------------------------------------------------------------------
void CMainWindow::updateSongbookLabels()
{
  m_sbInfoTitle->setText(songbook()->title());
  m_sbInfoAuthors->setText(songbook()->authors());
  m_sbInfoStyle->setText(songbook()->style());
}
//------------------------------------------------------------------------------
void CMainWindow::updateView()
{
  view()->sortByColumn(1, Qt::AscendingOrder);
  view()->sortByColumn(0, Qt::AscendingOrder);
}
//------------------------------------------------------------------------------
void CMainWindow::filterChanged(const QString &filter)
{
  QRegExp expression = QRegExp(filter, Qt::CaseInsensitive, QRegExp::FixedString);
  m_proxyModel->setFilterRegExp(expression);
}
//------------------------------------------------------------------------------
void CMainWindow::selectionChanged()
{
  QItemSelection invalid;
  selectionChanged(invalid, invalid);
}
//------------------------------------------------------------------------------
void CMainWindow::selectionChanged(const QItemSelection & , const QItemSelection & )
{
  m_sbNbSelected = selectionModel()->selectedRows().size();
  m_sbNbTotal = library()->rowCount();
  m_sbInfoSelection->setText(QString(tr("%1/%2"))
			     .arg(m_sbNbSelected).arg(m_sbNbTotal) );
  if(m_sbNbTotal==0)
    m_noDataInfo->show();
  else
    m_noDataInfo->hide();
}
//------------------------------------------------------------------------------
void CMainWindow::createActions()
{
  m_newSongAct = new QAction(tr("New Song"), this);
  m_newSongAct->setIcon(QIcon::fromTheme("document-new"));
  m_newSongAct->setStatusTip(tr("Write a new song"));
  connect(m_newSongAct, SIGNAL(triggered()), this, SLOT(newSong()));

  m_newAct = new QAction(tr("New"), this);
  m_newAct->setIcon(QIcon::fromTheme("folder-new"));
  m_newAct->setShortcut(QKeySequence::New);
  m_newAct->setStatusTip(tr("Create a new songbook"));
  connect(m_newAct, SIGNAL(triggered()), this, SLOT(newSongbook()));

  m_openAct = new QAction(tr("Open..."), this);
  m_openAct->setIcon(QIcon::fromTheme("document-open"));
  m_openAct->setShortcut(QKeySequence::Open);
  m_openAct->setStatusTip(tr("Open a songbook"));
  connect(m_openAct, SIGNAL(triggered()), this, SLOT(open()));

  m_saveAct = new QAction(tr("Save"), this);
  m_saveAct->setShortcut(QKeySequence::Save);
  m_saveAct->setIcon(QIcon::fromTheme("document-save"));
  m_saveAct->setStatusTip(tr("Save the current songbook"));
  connect(m_saveAct, SIGNAL(triggered()), this, SLOT(save()));

  m_saveAsAct = new QAction(tr("Save As..."), this);
  m_saveAsAct->setShortcut(QKeySequence::SaveAs);
  m_saveAsAct->setIcon(QIcon::fromTheme("document-save-as"));
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
  m_preferencesAct->setStatusTip(tr("Configure the application"));
  m_preferencesAct->setMenuRole(QAction::PreferencesRole);
  connect(m_preferencesAct, SIGNAL(triggered()), SLOT(preferences()));

  m_selectAllAct = new QAction(tr("Select all"), this);
  m_selectAllAct->setIcon(QIcon(":/icons/select-all.png"));
  m_selectAllAct->setStatusTip(tr("Select all songs in the library"));
  connect(m_selectAllAct, SIGNAL(triggered()), SLOT(selectAll()));

  m_unselectAllAct = new QAction(tr("Unselect all"), this);
  m_unselectAllAct->setIcon(QIcon(":/icons/unselect-all.png"));
  m_unselectAllAct->setStatusTip(tr("Unselect all songs in the library"));
  connect(m_unselectAllAct, SIGNAL(triggered()), SLOT(unselectAll()));

  m_invertSelectionAct = new QAction(tr("Invert Selection"), this);
  m_invertSelectionAct->setIcon(QIcon(":/icons/invert-selection.png"));
  m_invertSelectionAct->setStatusTip(tr("Invert currently selected songs in the library"));
  connect(m_invertSelectionAct, SIGNAL(triggered()), SLOT(invertSelection()));

  m_selectEnglishAct = new QAction(tr("english"), this);
  m_selectEnglishAct->setStatusTip(tr("Select/Unselect songs in english"));
  m_selectEnglishAct->setIcon(QIcon::fromTheme("flag-en",QIcon(":/icons/en.png")));
  m_selectEnglishAct->setCheckable(true);
  connect(m_selectEnglishAct, SIGNAL(triggered(bool)), SLOT(selectLanguage(bool)));

  m_selectFrenchAct = new QAction(tr("french"), this);
  m_selectFrenchAct->setStatusTip(tr("Select/Unselect songs in french"));
  m_selectFrenchAct->setIcon(QIcon::fromTheme("flag-fr",QIcon(":/icons/fr.png")));
  m_selectFrenchAct->setCheckable(true);
  connect(m_selectFrenchAct, SIGNAL(triggered(bool)), SLOT(selectLanguage(bool)));

  m_selectSpanishAct = new QAction(tr("spanish"), this);
  m_selectSpanishAct->setStatusTip(tr("Select/Unselect songs in spanish"));
  m_selectSpanishAct->setIcon(QIcon::fromTheme("flag-es",QIcon(":/icons/es.png")));
  m_selectSpanishAct->setCheckable(true);
  connect(m_selectSpanishAct, SIGNAL(triggered(bool)), SLOT(selectLanguage(bool)));

  m_adjustColumnsAct = new QAction(tr("Auto Adjust Columns"), this);
  m_adjustColumnsAct->setStatusTip(tr("Adjust columns to contents"));
  connect(m_adjustColumnsAct, SIGNAL(triggered()),
          view(), SLOT(resizeColumnsToContents()));

  m_libraryUpdateAct = new QAction(tr("Update"), this);
  m_libraryUpdateAct->setStatusTip(tr("Update current song list from \".sg\" files"));
  m_libraryUpdateAct->setIcon(QIcon::fromTheme("view-refresh"));
  m_libraryUpdateAct->setShortcut(QKeySequence::Refresh);
  connect(m_libraryUpdateAct, SIGNAL(triggered()), library(), SLOT(update()));

  CLibraryDownload *libraryDownload = new CLibraryDownload(this);
  m_libraryDownloadAct = new QAction(tr("Download"), this);
  m_libraryDownloadAct->setStatusTip(tr("Download songs from remote location"));
  m_libraryDownloadAct->setIcon(QIcon::fromTheme("folder-remote"));
  connect(m_libraryDownloadAct, SIGNAL(triggered()), libraryDownload, SLOT(exec()));

  m_toolBarViewAct = new QAction(tr("ToolBar"),this);
  m_toolBarViewAct->setStatusTip(tr("Show or hide the toolbar in the current window"));
  m_toolBarViewAct->setCheckable(true);
  m_toolBarViewAct->setChecked(m_isToolBarDisplayed);
  connect(m_toolBarViewAct, SIGNAL(toggled(bool)), this, SLOT(setToolBarDisplayed(bool)));

  m_statusbarViewAct = new QAction(tr("Statusbar"),this);
  m_statusbarViewAct->setStatusTip(tr("Show or hide the statusbar in the current window"));
  m_statusbarViewAct->setCheckable(true);
  m_statusbarViewAct->setChecked(m_isStatusbarDisplayed);
  connect(m_statusbarViewAct, SIGNAL(toggled(bool)), this, SLOT(setStatusbarDisplayed(bool)));

  m_builder = new CResizeCovers(this);
  m_resizeCoversAct = new QAction( tr("Resize covers"), this);
  m_resizeCoversAct->setStatusTip(tr("Ensure that covers are correctly resized"));
  connect(m_resizeCoversAct, SIGNAL(triggered()), m_builder, SLOT(dialog()));

  m_builder = new CLatexPreprocessing(this);
  m_checkerAct = new QAction( tr("LaTeX Preprocessing"), this);
  m_checkerAct->setStatusTip(tr("Check for common mistakes in songs (e.g spelling, chords, LaTeX typo ...)"));
  connect(m_checkerAct, SIGNAL(triggered()), m_builder, SLOT(dialog()));

  m_buildAct = new QAction(tr("Build PDF"), this);
  m_buildAct->setIcon(QIcon::fromTheme("document-export"));
  m_buildAct->setStatusTip(tr("Generate pdf from selected songs"));
  connect(m_buildAct, SIGNAL(triggered()), this, SLOT(build()));

  m_builder = new CMakeSongbook(this);
  m_builder->setProcessOptions(QStringList() << "clean");
#ifdef Q_WS_WIN
  m_builder->setProcessOptions(QStringList() << "/C" << "clean.bat");
#endif
  m_cleanAct = new QAction(tr("Clean"), this);
  m_cleanAct->setIcon(QIcon::fromTheme("edit-clear"));
  m_cleanAct->setStatusTip(tr("Clean LaTeX temporary files"));
  connect(m_cleanAct, SIGNAL(triggered()), m_builder, SLOT(action()));

}
//------------------------------------------------------------------------------
void CMainWindow::setToolBarDisplayed(bool value)
{
  if (m_isToolBarDisplayed != value && m_currentToolBar)
    {
      m_isToolBarDisplayed = value;
      m_currentToolBar->setVisible(value);
    }
}
//------------------------------------------------------------------------------
bool CMainWindow::isToolBarDisplayed( )
{
  return m_isToolBarDisplayed;
}
//------------------------------------------------------------------------------
void CMainWindow::setStatusbarDisplayed(bool value)
{
  m_isStatusbarDisplayed = value;
  statusBar()->setVisible(value);
}
//------------------------------------------------------------------------------
bool CMainWindow::isStatusbarDisplayed( )
{
  return m_isStatusbarDisplayed;
}
//------------------------------------------------------------------------------
void CMainWindow::closeEvent(QCloseEvent *event)
{
  writeSettings();
  event->accept();
}
//------------------------------------------------------------------------------
void CMainWindow::createMenus()
{
  menuBar()->setContextMenuPolicy(Qt::PreventContextMenu);

  m_fileMenu = menuBar()->addMenu(tr("&Songbook"));
  m_fileMenu->addAction(m_newAct);
  m_fileMenu->addAction(m_openAct);
  m_fileMenu->addAction(m_saveAct);
  m_fileMenu->addAction(m_saveAsAct);
  m_fileMenu->addSeparator();
  m_fileMenu->addAction(m_buildAct);
  m_fileMenu->addAction(m_cleanAct);
  m_fileMenu->addSeparator();
  m_fileMenu->addAction(m_exitAct);

  m_editMenu = menuBar()->addMenu(tr("&Edit"));
  m_editMenu->addAction(m_selectAllAct);
  m_editMenu->addAction(m_unselectAllAct);
  m_editMenu->addAction(m_invertSelectionAct);
  m_editMenu->addSeparator();
  m_editMenu->addAction(m_preferencesAct);

  m_dbMenu = menuBar()->addMenu(tr("&Library"));
  m_dbMenu->addAction(m_newSongAct);
  m_dbMenu->addSeparator();
  m_dbMenu->addAction(m_libraryDownloadAct);
  m_dbMenu->addAction(m_libraryUpdateAct);

  m_viewMenu = menuBar()->addMenu(tr("&View"));
  m_viewMenu->addAction(m_toolBarViewAct);
  m_viewMenu->addAction(m_statusbarViewAct);
  m_viewMenu->addAction(m_adjustColumnsAct);

  m_viewMenu = menuBar()->addMenu(tr("&Tools"));
  m_viewMenu->addAction(m_resizeCoversAct);
  m_viewMenu->addAction(m_checkerAct);

  m_helpMenu = menuBar()->addMenu(tr("&Help"));
  m_helpMenu->addAction(m_documentationAct);
  m_helpMenu->addAction(m_aboutAct);
}
//------------------------------------------------------------------------------
void CMainWindow::createToolBar()
{
  m_toolBar = new QToolBar(tr("Library tools"), this);
  m_toolBar->setMovable(false);
  m_toolBar->setContextMenuPolicy(Qt::PreventContextMenu);

  // filter related objects
  QSqlQueryModel *completionModel = new QSqlQueryModel;
  completionModel->setQuery("SELECT DISTINCT title FROM songs "
			    "UNION "
			    "SELECT DISTINCT artist FROM songs "
			    "UNION "
			    "SELECT DISTINCT album FROM songs ");

  QCompleter *completer = new QCompleter;
  completer->setModel(completionModel);
  completer->setCaseSensitivity(Qt::CaseInsensitive);
  completer->setCompletionMode(QCompleter::PopupCompletion);

  m_filterLineEdit = new CFilterLineEdit;
  m_filterLineEdit->setCompleter(completer);

  connect(m_filterLineEdit, SIGNAL(textChanged(const QString&)),
	  this, SLOT(filterChanged(const QString&)));

  QWidget* stretch = new QWidget;
  stretch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  // songbook actions
  m_toolBar->addAction(m_newAct);
  m_toolBar->addAction(m_openAct);
  m_toolBar->addAction(m_saveAct);
  m_toolBar->addAction(m_saveAsAct);
  m_toolBar->addSeparator();
  // library actions
  m_toolBar->addAction(m_buildAct);
  m_toolBar->addAction(m_newSongAct);
  m_toolBar->addSeparator();
  // selection actions
  m_toolBar->addAction(m_selectAllAct);
  m_toolBar->addAction(m_unselectAllAct);
  m_toolBar->addAction(m_invertSelectionAct);
  m_toolBar->addSeparator();
  // language selection actions
  m_toolBar->addAction(m_selectEnglishAct);
  m_toolBar->addAction(m_selectFrenchAct);
  m_toolBar->addAction(m_selectSpanishAct);
  // add toolbar spacing
  m_toolBar->addWidget(stretch);
  // add toolbar filter
  m_toolBar->addWidget(m_filterLineEdit);

  m_currentToolBar = m_toolBar;

  addToolBar(m_toolBar);
  setUnifiedTitleAndToolBarOnMac(true);
}
//------------------------------------------------------------------------------
QGridLayout * CMainWindow::songInfo()
{
  CLabel *artistLabel = new CLabel();
  artistLabel->setElideMode(Qt::ElideRight);
  artistLabel->setFixedWidth(175);
  CLabel *titleLabel = new CLabel();
  titleLabel->setElideMode(Qt::ElideRight);
  titleLabel->setFixedWidth(175);
  CLabel *albumLabel = new CLabel();
  albumLabel->setElideMode(Qt::ElideRight);
  albumLabel->setFixedWidth(175);

  QDialogButtonBox *buttonBox = new QDialogButtonBox;
  QPushButton *editButton = new QPushButton(tr("Edit"));
  QPushButton *deleteButton = new QPushButton(tr("Delete"));
  editButton->setDefault(true);
  buttonBox->addButton(editButton, QDialogButtonBox::ActionRole);
  buttonBox->addButton(deleteButton, QDialogButtonBox::NoRole);

  connect(editButton, SIGNAL(clicked()), SLOT(songEditor()));
  connect(deleteButton, SIGNAL(clicked()), SLOT(deleteSong()));

  QGridLayout *layout = new QGridLayout;
  m_coverLabel.setAlignment(Qt::AlignTop);
  layout->addWidget(&m_coverLabel,0,0,4,1);
  layout->addWidget(new QLabel(tr("<i>Title:</i>")),0,1,1,1,Qt::AlignLeft);
  layout->addWidget(titleLabel,0,2,1,1);
  layout->addWidget(new QLabel(tr("<i>Artist:</i>")),1,1,1,1,Qt::AlignLeft);
  layout->addWidget(artistLabel,1,2,1,1);
  layout->addWidget(new QLabel(tr("<i>Album:</i>")),2,1,1,1,Qt::AlignLeft);
  layout->addWidget(albumLabel,2,2,1,1);
  layout->addWidget(buttonBox,3,1,1,2);
  layout->setColumnStretch(2,1);

  //Data mapper
  m_mapper = new QDataWidgetMapper();
  m_mapper->setModel(m_proxyModel);
  m_mapper->addMapping(artistLabel, 0, QByteArray("text"));
  m_mapper->addMapping(titleLabel, 1, QByteArray("text"));
  m_mapper->addMapping(albumLabel, 4, QByteArray("text"));
  updateCover(QModelIndex());

  connect(view(), SIGNAL(clicked(const QModelIndex &)),
          m_mapper, SLOT(setCurrentModelIndex(const QModelIndex &)));
  connect(view(), SIGNAL(clicked(const QModelIndex &)),
          SLOT(updateCover(const QModelIndex &)));

  return layout;
}
//------------------------------------------------------------------------------
QGridLayout * CMainWindow::songbookInfo()
{
  QPushButton* button = new QPushButton(tr("Settings"));
  connect(button, SIGNAL(clicked()), this, SLOT(templateSettings()));
 
  QGridLayout* layout = new QGridLayout;
  layout->addWidget(new QLabel(tr("<i>Title:</i>")),0,0,1,1);
  layout->addWidget(m_sbInfoTitle,0,1,1,2);
  layout->addWidget(new QLabel(tr("<i>Authors:</i>")),1,0,1,1);
  layout->addWidget(m_sbInfoAuthors,1,1,1,2);
  layout->addWidget(new QLabel(tr("<i>Style:</i>")),2,0,1,1);
  layout->addWidget(m_sbInfoStyle,2,1,1,2);
  layout->addWidget(new QLabel(tr("<i>Selection:</i>")),3,0,1,1);
  layout->addWidget(m_sbInfoSelection,3,1,1,2);
  layout->addWidget(button,4,2,1,1);

  m_sbInfoTitle->setElideMode(Qt::ElideRight);
  m_sbInfoTitle->setFixedWidth(250);
  m_sbInfoAuthors->setElideMode(Qt::ElideRight);
  m_sbInfoAuthors->setFixedWidth(250);
  m_sbInfoStyle->setElideMode(Qt::ElideRight);
  m_sbInfoStyle->setFixedWidth(250);

  return layout;
}
//------------------------------------------------------------------------------
void CMainWindow::updateCover(const QModelIndex & index)
{
  if (!selectionModel()->hasSelection())
    {
      m_cover = new QPixmap(QIcon::fromTheme("image-missing").pixmap(128,128));
      m_coverLabel.setPixmap(m_cover->scaled(128, 128, Qt::IgnoreAspectRatio));
      return;
    }

  // do not retrieve last clicked item but last selected item
  QModelIndex lastIndex = selectionModel()->selectedRows().last();
  selectionModel()->setCurrentIndex(lastIndex, QItemSelectionModel::NoUpdate);
  if (lastIndex != index)
    m_mapper->setCurrentModelIndex(lastIndex);

  QString coverpath = library()->record(m_proxyModel->mapToSource(lastIndex).row()).field("cover").value().toString();
  if (QFile::exists(coverpath))
    m_cover->load(coverpath);
  else
    m_cover = new QPixmap(QIcon::fromTheme("image-missing").pixmap(128,128));

  m_coverLabel.setPixmap(m_cover->scaled(128, 128, Qt::IgnoreAspectRatio));
}
//------------------------------------------------------------------------------
void CMainWindow::preferences()
{
  ConfigDialog dialog;
  dialog.exec();
  readSettings();
  applySettings();
}
//------------------------------------------------------------------------------
void CMainWindow::documentation()
{
  QDesktopServices::openUrl(QUrl(QString("http://www.patacrep.com/data/documents/doc.pdf")));
}
//------------------------------------------------------------------------------
void CMainWindow::about()
{
  QString title = QString(tr("About Patacrep! Songbook Client"));
  QString version = QCoreApplication::applicationVersion();

  QString description = QString(tr("This program allows to build customized songbooks from "
				   "<a href=\"http::www.patacrep.com\">www.patacrep.com</a>"));

  QStringList authorsList = QStringList() << "Crep (R.Goffe)"
					  << "Lohrun (A.Dupas)"
					  << "Carreau (M.Bussonnier)";
  QString author, authors;
  foreach(author, authorsList)
    authors += author + ", ";
  authors.chop(2);

  QMessageBox::about(this, title, QString(tr("%1 <br/>"
					     "<b>Version:</b> %2 <br/>"
					     "<b>Authors:</b> %3 <br/>"))
		     .arg(description).arg(version).arg(authors));
}
//------------------------------------------------------------------------------
void CMainWindow::selectAll()
{
  view()->selectAll();
  view()->setFocus();
}
//------------------------------------------------------------------------------
void CMainWindow::unselectAll()
{
  view()->clearSelection();
}
//------------------------------------------------------------------------------
void CMainWindow::invertSelection()
{
  QModelIndexList indexes = selectionModel()->selectedRows();
  QModelIndex index;

  view()->selectAll();

  foreach(index, indexes)
    {
      selectionModel()->select(index, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
    }
}
//------------------------------------------------------------------------------
void CMainWindow::selectLanguage(bool selection)
{
  QString language = qobject_cast< QAction* >(QObject::sender())->text();
  QList<QModelIndex> indexes;
  QModelIndex index;

  indexes = m_proxyModel->match(m_proxyModel->index(0,6), Qt::ToolTipRole, language, -1);

  QItemSelectionModel::SelectionFlags flag = (selection ? QItemSelectionModel::Select : QItemSelectionModel::Deselect) | QItemSelectionModel::Rows;

  foreach(index, indexes)
    {
      selectionModel()->select(index, flag);
    }
  view()->setFocus();
}
//------------------------------------------------------------------------------
QStringList CMainWindow::getSelectedSongs()
{
  QStringList songsPath;
  QModelIndexList indexes = selectionModel()->selectedRows();
  QModelIndex index;

  qSort(indexes.begin(), indexes.end());

  foreach(index, indexes)
    {
      songsPath << library()->record(m_proxyModel->mapToSource(index).row()).field("path").value().toString();
    }

  return songsPath;
}
//------------------------------------------------------------------------------
void CMainWindow::build()
{
  if(getSelectedSongs().isEmpty())
    {
      if(QMessageBox::question(this, windowTitle(),
			       QString(tr("You did not select any song. \n "
					  "Do you want to build the songbook with all songs ?")),
			       QMessageBox::Yes,
			       QMessageBox::No,
			       QMessageBox::NoButton) == QMessageBox::No)
	return;
      else
	selectAll();
    }

  save(true);

  switch(songbook()->checkFilename())
    {
    case WrongDirectory:
      statusBar()->showMessage(tr("The songbook is not in the working directory. Build aborted."));
      return;
    case WrongExtension:
      statusBar()->showMessage(tr("Wrong filename: songbook does not have \".sb\" extension. Build aborted."));
      return;
    default:
      break;
    }

  QString basename = QFileInfo(songbook()->filename()).baseName();
  QString target = QString("%1.pdf").arg(basename);

  m_builder = new CMakeSongbook(this);

  //force a make clean
  m_builder->setProcessOptions(QStringList() << "clean");
#ifdef Q_WS_WIN
  m_builder->setProcessOptions(QStringList() << "/C" << "clean.bat");
#endif
  m_builder->action();
  m_builder->process()->waitForFinished();

  m_builder->setProcessOptions(QStringList() << target);
#ifdef Q_WS_WIN
  m_builder->setProcessOptions(QStringList() << "/C" << "make.bat" << basename);
#endif
  m_builder->action();
}
//------------------------------------------------------------------------------
void CMainWindow::newSongbook()
{
  songbook()->reset();
  updateTitle(songbook()->filename());
}
//------------------------------------------------------------------------------
void CMainWindow::open()
{
  QString filename = QFileDialog::getOpenFileName(this,
                                                  tr("Open"),
                                                  workingPath(),
                                                  tr("Songbook (*.sb)"));
  songbook()->load(filename);
  QStringList songlist = songbook()->songs();
  QString path = QString("%1/songs/").arg(workingPath());
  songlist.replaceInStrings(QRegExp("^"),path);

  m_filterLineEdit->clear();
  view()->clearSelection();

  QList<QModelIndex> indexes;
  QString str;
  foreach(str, songlist)
    {
      indexes = m_proxyModel->match( m_proxyModel->index(0,3), Qt::MatchExactly, str );
      if (!indexes.isEmpty())
        selectionModel()->select(indexes[0], QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }

  updateTitle(songbook()->filename());
}
//------------------------------------------------------------------------------
void CMainWindow::save(bool forced)
{
  if(songbook()->filename().isEmpty() || songbook()->filename().endsWith("default.sb"))
    {
      if(forced)
	songbook()->setFilename(QString("%1/default.sb").arg(workingPath()));
      else if(!songbook()->filename().isEmpty())
	saveAs();
    }

  updateSongsList();
  songbook()->save(songbook()->filename());
  updateTitle(songbook()->filename());
}
//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
void CMainWindow::updateSongsList()
{
  QStringList songlist = getSelectedSongs();
  QString path = QString("%1%2songs%2").arg(workingPath()).arg(QDir::separator());
  songlist.replaceInStrings(path, QString());
#ifdef Q_WS_WIN
  songlist.replaceInStrings("\\", "/");
#endif
  songbook()->setSongs(songlist);
}
//------------------------------------------------------------------------------
void CMainWindow::updateTitle(const QString &filename)
{
  QString text = filename.isEmpty() ? tr("New songbook") : filename;
  setWindowTitle(tr("%1 - %2[*]")
                 .arg(QCoreApplication::applicationName())
                 .arg(text));
}
//------------------------------------------------------------------------------
const QString CMainWindow::workingPath()
{
  return library()->directory().canonicalPath();
}
//------------------------------------------------------------------------------
void CMainWindow::setWorkingPath(const QString &path)
{
  if (path != workingPath())
    {
      library()->setDirectory(path);
      emit(workingPathChanged(workingPath()));

      // update the corresponding setting
      QSettings settings;
      settings.setValue("workingPath", workingPath());
    }
}
//------------------------------------------------------------------------------
QProgressBar * CMainWindow::progressBar() const
{
  return m_progressBar;
}
//------------------------------------------------------------------------------
CSongbook * CMainWindow::songbook() const
{
  return m_songbook;
}
//------------------------------------------------------------------------------
QTableView * CMainWindow::view() const
{
  return m_view;
}
//------------------------------------------------------------------------------
CLibrary * CMainWindow::library() const
{
  return m_library;
}
//------------------------------------------------------------------------------
QItemSelectionModel * CMainWindow::selectionModel()
{
  while (library()->canFetchMore())
    library()->fetchMore();

  return view()->selectionModel();
}
//------------------------------------------------------------------------------
void CMainWindow::songEditor()
{
  if (!selectionModel()->hasSelection())
    {
      statusBar()->showMessage(tr("Please select a song to edit."));
      return;
    }

  int row = m_proxyModel->mapToSource(selectionModel()->currentIndex()).row();
  QSqlRecord record = library()->record(row);
  QString path = record.field("path").value().toString();
  QString title = record.field("title").value().toString();

  songEditor(path, title);
}
//------------------------------------------------------------------------------
void CMainWindow::songEditor(const QString &path, const QString &title)
{
  if (m_editors.contains(path))
    {
      m_mainWidget->setCurrentWidget(m_editors[path]);
      return;
    }

  CSongEditor* editor = new CSongEditor();
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
//------------------------------------------------------------------------------
void CMainWindow::newSong()
{
  CDialogNewSong *dialog = new CDialogNewSong(this);
  if (dialog->exec() == QDialog::Accepted)
    {
      songEditor(dialog->path(), dialog->title());
    }
  delete dialog;
}
//------------------------------------------------------------------------------
void CMainWindow::deleteSong()
{
  if (!selectionModel()->hasSelection())
    {
      statusBar()->showMessage(tr("Please select a song to remove."));
      return;
    }

  QString path = library()->record(m_proxyModel->mapToSource(selectionModel()->currentIndex()).row()).field("path").value().toString();

  deleteSong(path);
}
//------------------------------------------------------------------------------
void CMainWindow::deleteSong(const QString &path)
{
  QString qs(tr("You are about to remov a song from the library.\n"
                "Yes : The song will only be deleted from the library"
                      "and can be retrieved by rebuilding the library\n"
                "No  : Nothing will be deleted\n"
                "Delete file : You will also delete %1 from your hard drive\n"
                "If you are unsure what to do, click No.").arg(path));
  QMessageBox msgBox;
  msgBox.setIcon(QMessageBox::Question);
  msgBox.setText(tr("Removing song from Library."));
  msgBox.setInformativeText(tr("Are you sure ?"));
  msgBox.addButton(QMessageBox::No);
  QPushButton* yesb = msgBox.addButton(QMessageBox::Yes);
  QPushButton* delb = msgBox.addButton(tr("Delete file"),QMessageBox::DestructiveRole);
  msgBox.setDefaultButton(QMessageBox::No);
  msgBox.setDetailedText(qs);
  msgBox.exec();

  if (msgBox.clickedButton() == yesb || msgBox.clickedButton() == delb)
  {
      //remove entry in database in 2 case
      library()->removeSong(path);
      //once deleted move selection in the model
      updateCover(selectionModel()->currentIndex());
      m_mapper->setCurrentModelIndex(selectionModel()->currentIndex());
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
//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
void CMainWindow::changeTab(int index)
{
  CSongEditor *editor = qobject_cast< CSongEditor* >(m_mainWidget->widget(index));

  if (editor)
    {
      switchToolBar(editor->toolBar());
      m_saveAct->setShortcutContext(Qt::WidgetShortcut);
    }
  else
    {
      switchToolBar(m_toolBar);
      m_saveAct->setShortcutContext(Qt::WindowShortcut);
    }
}
//------------------------------------------------------------------------------
QTextEdit* CMainWindow::log() const
{
  return m_log;
}

void CMainWindow::connectDatabase()
{
  if (!QSqlDatabase::isDriverAvailable("QSQLITE"))
    {
      QMessageBox::critical(this,
			    tr("Cannot open database"),
			    tr("Unable to establish a database connection.\n"
			       "This application needs SQLite support."),
			    QMessageBox::Abort);

    }
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");

  QDir::home().mkpath(".cache/songbook-client");
  QString databasePath
    = QDir::home().filePath(".cache/songbook-client/patacrep.db");

  db.setDatabaseName(databasePath);
  db.open();
}

void CMainWindow::disconnectDatabase()
{
  QSqlDatabase db = QSqlDatabase::database();
  db.close();
  QSqlDatabase::removeDatabase(QString());
}
