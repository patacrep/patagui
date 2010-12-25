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
#include <QtGui>
#include <QtSql>
#include <QtAlgorithms>
#include <QDebug>

#include "label.hh"
#include "mainwindow.hh"
#include "preferences.hh"
#include "library.hh"
#include "songbook.hh"
#include "tools.hh"
#include "download.hh"
#include "song-editor.hh"
#include "dialog-new-song.hh"
#include "sort-filter-proxy-model.hh"
//******************************************************************************
CMainWindow::CMainWindow()
  : QMainWindow()
  , m_library()
  , m_proxyModel(new CSortFilterProxyModel)
  , m_songbook()
  , m_view(new QTableView)
  , m_progressBar(new QProgressBar)
  , m_cover(new QPixmap)
{
  setWindowTitle("Patacrep Songbook Client");
  setWindowIcon(QIcon(":/icons/patacrep.png"));

  m_isToolbarDisplayed = true;
  m_isStatusbarDisplayed = true;

  readSettings();

  // main document and title
  m_songbook = new CSongbook();
  m_songbook->setWorkingPath(workingPath());
  connect(m_songbook, SIGNAL(wasModified(bool)),
          this, SLOT(setWindowModified(bool)));
  connect(m_songbook, SIGNAL(wasModified(bool)),
          this, SLOT(updateSongbookLabels(bool)));
  connect(this, SIGNAL(workingPathChanged(QString)),
	  m_songbook, SLOT(setWorkingPath(QString)));
  updateTitle(m_songbook->filename());

  // compilation log
  m_log = new QTextEdit;
  m_log->setMaximumHeight(150);
  m_log->setReadOnly(true);

  // no data info widget
  m_noDataInfo = new QTextEdit;
  m_noDataInfo->setReadOnly(true);
  m_noDataInfo->setMaximumHeight(150);
  m_noDataInfo->
    setHtml(QString(tr("<table><tr><td valign=middle>  "
		       "<img src=\":/icons/attention.png\" />  </td><td>"
		       "<p>The directory <b>%1</b> does not contain any song file (\".sg\").<br/><br/> "
		       "You may :<ul><li>select a valid directory in the menu <i>Edit/Preferences</i></li>"
		       "<li>use the menu <i>Library/Download</i> to get the latest git snapshot</li>"
		       "<li>manually download the latest tarball on "
		       "<a href=\"http://www.patacrep.com/static1/downloads\">"
		       "patacrep.com</a></li></ul>"
		       "</p></td></tr></table>")).arg(workingPath()));
  m_noDataInfo->hide();

  // toolbar (for the build button)
  m_toolbar = new QToolBar;
  m_toolbar->setMovable(false);

  createActions();
  createMenus();

  m_toolbar->addAction(m_newAct);
  m_toolbar->addAction(m_openAct);
  m_toolbar->addAction(m_saveAct);
  m_toolbar->addAction(m_saveAsAct);
  m_toolbar->addSeparator();
  m_toolbar->addAction(m_buildAct);
  m_toolbar->addSeparator();
  m_toolbar->addAction(m_selectAllAct);
  m_toolbar->addAction(m_unselectAllAct);
  m_toolbar->addAction(m_invertSelectionAct);
  m_toolbar->addSeparator();
  m_toolbar->addAction(m_selectEnglishAct);
  m_toolbar->addAction(m_selectFrenchAct);
  m_toolbar->addAction(m_selectSpanishAct);

  //Connection to database
  connectDb();

  // filtering related widgets
  QLineEdit *filterLineEdit = new QLineEdit;
  QLabel *filterLabel = new QLabel(tr("&Filter:"));
  filterLabel->setBuddy(filterLineEdit);

  connect(filterLineEdit, SIGNAL(textChanged(QString)),
	  this, SLOT(filterChanged()));
  
  QBoxLayout *filterLayout = new QHBoxLayout;
  filterLayout->addWidget(filterLabel);
  filterLayout->addWidget(filterLineEdit);
  
  // organize the toolbar and the filter into an horizontal layout
  QBoxLayout *horizontalLayout = new QHBoxLayout;
  horizontalLayout->addWidget(m_toolbar);
  horizontalLayout->addStretch();
  horizontalLayout->addLayout(filterLayout);

  connect(selectionModel(), SIGNAL(selectionChanged(const QItemSelection & , 
						    const QItemSelection & )),
	  this, SLOT(selectionChanged(const QItemSelection & , const QItemSelection & )));

  
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
  dataLayout->addWidget(m_view);
  dataLayout->addWidget(m_noDataInfo);
  centerLayout->addLayout(leftLayout);
  centerLayout->setStretch(0,1);
  centerLayout->addLayout(dataLayout);
  centerLayout->setStretch(1,2);
  mainLayout->addLayout(centerLayout);
  mainLayout->addWidget(m_log);

  QWidget* libraryTab = new QWidget;
  QBoxLayout *libraryLayout = new QVBoxLayout;
  libraryLayout->addLayout(horizontalLayout);
  libraryLayout->addLayout(mainLayout);
  libraryTab->setLayout(libraryLayout);

  // place elements into the main window
  m_mainWidget = new CTabWidget;
  m_mainWidget->setTabsClosable(true);
  m_mainWidget->setMovable(true);
  connect( m_mainWidget, SIGNAL(tabCloseRequested(int)),
	   this, SLOT(closeTab(int)) );
  connect( m_mainWidget, SIGNAL(currentChanged(int)),
	   this, SLOT(changeTab(int)) );
  m_mainWidget->addTab(libraryTab, tr("Library"));
  setCentralWidget(m_mainWidget);

  // status bar with an embedded progress bar on the right
  progressBar()->setTextVisible(false);
  progressBar()->setRange(0, 0);
  progressBar()->hide();
  statusBar()->addPermanentWidget(progressBar());

  applySettings();
  selectionChanged();
  m_songbook->panel();
  updateSongbookLabels(true);
}
//------------------------------------------------------------------------------
CMainWindow::~CMainWindow()
{
  delete m_library;
  delete m_songbook;

  {  // close db connection
    QSqlDatabase db = QSqlDatabase::database();
    db.close();
  }
  QSqlDatabase::removeDatabase(QString());
}
//------------------------------------------------------------------------------
void CMainWindow::readSettings()
{
  QSettings settings;

  resize(settings.value("mainWindow/size", QSize(800,600)).toSize());

  setWorkingPath( settings.value("workingPath", QString("%1/songbook").arg(QDir::home().path())).toString() );

  settings.beginGroup("display");
  m_displayColumnArtist = settings.value("artist", true).toBool();
  m_displayColumnTitle = settings.value("title", true).toBool();
  m_displayColumnPath = settings.value("path", false).toBool();
  m_displayColumnAlbum = settings.value("album", true).toBool();
  m_displayColumnLilypond = settings.value("lilypond", false).toBool();
  m_displayColumnCover = settings.value("cover", true).toBool();
  m_displayColumnLang = settings.value("lang", false).toBool();
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
  m_view->setColumnHidden(0,!m_displayColumnArtist);
  m_view->setColumnHidden(1,!m_displayColumnTitle);
  m_view->setColumnHidden(2,!m_displayColumnLilypond);
  m_view->setColumnHidden(3,!m_displayColumnPath);
  m_view->setColumnHidden(4,!m_displayColumnAlbum);
  m_view->setColumnHidden(5,!m_displayColumnCover);
  m_view->setColumnHidden(6,!m_displayColumnLang);
  m_view->setColumnWidth(0,200);
  m_view->setColumnWidth(1,300);
  m_view->setColumnWidth(4,200);
  m_log->setVisible(m_displayCompilationLog);
}
//------------------------------------------------------------------------------
void CMainWindow::templateSettings()
{
  QDialog *dialog = new QDialog;
  dialog->setWindowTitle(tr("Songbook settings"));
  QVBoxLayout *layout = new QVBoxLayout;

  QScrollArea *songbookScrollArea = new QScrollArea();
  songbookScrollArea->setMinimumWidth(400);
  songbookScrollArea->setWidget(m_songbook->panel());
  songbookScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  
  QDialogButtonBox *buttonBox = new QDialogButtonBox;
  
  QPushButton *button = new QPushButton(tr("Reset"));
  connect( button, SIGNAL(clicked()), m_songbook, SLOT(reset()) );
  buttonBox->addButton(button, QDialogButtonBox::ResetRole);
  
  button = new QPushButton(tr("Ok"));
  button->setDefault(true);
  connect( button, SIGNAL(clicked()), dialog, SLOT(accept()) );
  buttonBox->addButton(button, QDialogButtonBox::ActionRole);
  
  layout->addWidget(songbookScrollArea);
  layout->addWidget(buttonBox);
  dialog->setLayout(layout);
  dialog->show();  
}
//------------------------------------------------------------------------------
void CMainWindow::updateSongbookLabels(bool modified)
{
  m_sbInfoTitle->setText(m_songbook->title());
  m_sbInfoAuthors->setText(m_songbook->authors());
  m_sbInfoStyle->setText(m_songbook->style());
}
//------------------------------------------------------------------------------
void CMainWindow::filterChanged()
{
  QLineEdit *lineEdit = qobject_cast< QLineEdit* >(QObject::sender());
  QRegExp expression = QRegExp(lineEdit->text(), Qt::CaseInsensitive, QRegExp::FixedString);
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
  m_sbNbTotal = m_library->rowCount();
  m_sbInfoSelection->setText(QString(tr("%1/%2"))
			     .arg(m_sbNbSelected).arg(m_sbNbTotal) );
}
//------------------------------------------------------------------------------
void CMainWindow::createActions()
{
  m_newSongAct = new QAction(tr("New Song"), this);
#if QT_VERSION >= 0x040600
  m_newSongAct->setIcon(QIcon::fromTheme("document-new"));
#endif
  m_newSongAct->setStatusTip(tr("Write a new song"));
  connect(m_newSongAct, SIGNAL(triggered()), this, SLOT(newSong()));

  m_newAct = new QAction(tr("New"), this);
#if QT_VERSION >= 0x040600
  m_newAct->setIcon(QIcon::fromTheme("folder-new"));
#endif
  m_newAct->setShortcut(QKeySequence::New);
  m_newAct->setStatusTip(tr("Create a new songbook"));
  connect(m_newAct, SIGNAL(triggered()), this, SLOT(newSongbook()));

  m_openAct = new QAction(tr("Open..."), this);
#if QT_VERSION >= 0x040600
  m_openAct->setIcon(QIcon::fromTheme("document-open"));
#endif
  m_openAct->setShortcut(QKeySequence::Open);
  m_openAct->setStatusTip(tr("Open a songbook"));
  connect(m_openAct, SIGNAL(triggered()), this, SLOT(open()));

  m_saveAct = new QAction(tr("Save"), this);
  m_saveAct->setShortcut(QKeySequence::Save);
#if QT_VERSION >= 0x040600
  m_saveAct->setIcon(QIcon::fromTheme("document-save"));
#endif
  m_saveAct->setStatusTip(tr("Save the current songbook"));
  connect(m_saveAct, SIGNAL(triggered()), this, SLOT(save()));

  m_saveAsAct = new QAction(tr("Save As..."), this);
  m_saveAsAct->setShortcut(QKeySequence::SaveAs);
#if QT_VERSION >= 0x040600
  m_saveAsAct->setIcon(QIcon::fromTheme("document-save-as"));
#endif
  m_saveAsAct->setStatusTip(tr("Save the current songbook with a different name"));
  connect(m_saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

  m_documentationAct = new QAction(tr("Online documentation"), this);
  m_saveAsAct->setShortcut(QKeySequence::HelpContents);
  m_documentationAct->setIcon(QIcon::fromTheme("help-contents"));
  m_documentationAct->setStatusTip(tr("Download documentation pdf file "));
  connect(m_documentationAct, SIGNAL(triggered()), this, SLOT(documentation()));

  m_aboutAct = new QAction(tr("&About"), this);
#if QT_VERSION >= 0x040600
  m_aboutAct->setIcon(QIcon::fromTheme("help-about"));
#endif
  m_aboutAct->setStatusTip(tr("About this application"));
  connect(m_aboutAct, SIGNAL(triggered()), this, SLOT(about()));

  m_exitAct = new QAction(tr("Quit"), this);
  m_saveAsAct->setShortcut(QKeySequence::Close);
#if QT_VERSION >= 0x040600
  m_exitAct->setIcon(QIcon::fromTheme("application-exit"));
  m_exitAct->setShortcut(QKeySequence::Quit);
#endif
  m_exitAct->setStatusTip(tr("Quit the program"));
  connect(m_exitAct, SIGNAL(triggered()), this, SLOT(close()));

  m_buildAct = new QAction(tr("Build PDF"), this);
#if QT_VERSION >= 0x040600
  m_buildAct->setIcon(QIcon::fromTheme("document-export"));
#endif
  m_buildAct->setStatusTip(tr("Generate pdf from selected songs"));
  connect(m_buildAct, SIGNAL(triggered()), this, SLOT(build()));

  m_cleanAct = new QAction(tr("Clean"), this);
#if QT_VERSION >= 0x040600
  m_cleanAct->setIcon(QIcon::fromTheme("edit-clear"));
#endif
  m_cleanAct->setStatusTip(tr("Clean LaTeX temporary files"));
  connect(m_cleanAct, SIGNAL(triggered()), this, SLOT(clean()));

  m_preferencesAct = new QAction(tr("&Preferences"), this);
  m_preferencesAct->setStatusTip(tr("Configure the application"));
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
  m_selectEnglishAct->setIcon(QIcon(":/icons/en.png"));
  m_selectEnglishAct->setCheckable(true);
  connect(m_selectEnglishAct, SIGNAL(triggered(bool)), SLOT(selectLanguage(bool)));

  m_selectFrenchAct = new QAction(tr("french"), this);
  m_selectFrenchAct->setStatusTip(tr("Select/Unselect songs in french"));
  m_selectFrenchAct->setIcon(QIcon(":/icons/fr.png"));
  m_selectFrenchAct->setCheckable(true);
  connect(m_selectFrenchAct, SIGNAL(triggered(bool)), SLOT(selectLanguage(bool)));

  m_selectSpanishAct = new QAction(tr("spanish"), this);
  m_selectSpanishAct->setStatusTip(tr("Select/Unselect songs in spanish"));
  m_selectSpanishAct->setIcon(QIcon(":/icons/es.png"));
  m_selectSpanishAct->setCheckable(true);
  connect(m_selectSpanishAct, SIGNAL(triggered(bool)), SLOT(selectLanguage(bool)));

  m_adjustColumnsAct = new QAction(tr("Auto Adjust Columns"), this);
  m_adjustColumnsAct->setStatusTip(tr("Adjust columns to contents"));
  connect(m_adjustColumnsAct, SIGNAL(triggered()),
          m_view, SLOT(resizeColumnsToContents()));

  m_refreshLibraryAct = new QAction(tr("Update"), this);
  m_refreshLibraryAct->setStatusTip(tr("Update current song list from \".sg\" files"));
  connect(m_refreshLibraryAct, SIGNAL(triggered()), this, SLOT(refreshLibrary()));

  m_rebuildLibraryAct = new QAction(tr("Rebuild"), this);
  m_rebuildLibraryAct->setStatusTip(tr("Rebuild the current song list from \".sg\" files"));
  connect(m_rebuildLibraryAct, SIGNAL(triggered()), this, SLOT(rebuildLibrary()));

  m_downloadDbAct = new QAction(tr("Download"),this);
  m_downloadDbAct->setStatusTip(tr("Download songs from Patacrep"));
#if QT_VERSION >= 0x040600
  m_downloadDbAct->setIcon(QIcon::fromTheme("folder-remote"));
#endif
  connect(m_downloadDbAct, SIGNAL(triggered()), this, SLOT(downloadDialog()));

  m_toolbarViewAct = new QAction(tr("Toolbar"),this);
  m_toolbarViewAct->setStatusTip(tr("Show or hide the toolbar in the current window"));
  m_toolbarViewAct->setCheckable(true);
  m_toolbarViewAct->setChecked(m_isToolbarDisplayed);
  connect(m_toolbarViewAct, SIGNAL(toggled(bool)), this, SLOT(setToolbarDisplayed(bool)));

  m_statusbarViewAct = new QAction(tr("Statusbar"),this);
  m_statusbarViewAct->setStatusTip(tr("Show or hide the statusbar in the current window"));
  m_statusbarViewAct->setCheckable(true);
  m_statusbarViewAct->setChecked(m_isStatusbarDisplayed);
  connect(m_statusbarViewAct, SIGNAL(toggled(bool)), this, SLOT(setStatusbarDisplayed(bool)));

  CTools* tools = new CTools(this);
  m_resizeCoversAct = new QAction( tr("Resize covers"), this);
  m_resizeCoversAct->setStatusTip(tr("Ensure that covers are correctly resized"));
  connect(m_resizeCoversAct, SIGNAL(triggered()), tools, SLOT(resizeCoversDialog()));

  m_checkerAct = new QAction( tr("LaTeX Preprocessing"), this);
  m_checkerAct->setStatusTip(tr("Check for common mistakes in songs (e.g spelling, chords, LaTeX typo ...)"));
  connect(m_checkerAct, SIGNAL(triggered()), tools, SLOT(latexPreprocessingDialog()));
}
//------------------------------------------------------------------------------
void CMainWindow::setToolbarDisplayed( bool value )
{
  if( m_isToolbarDisplayed != value && m_toolbar )
    {
      m_isToolbarDisplayed = value;
      m_toolbar->setVisible(value);
    }
}
//------------------------------------------------------------------------------
bool CMainWindow::isToolbarDisplayed( )
{
  return m_isToolbarDisplayed;
}
//------------------------------------------------------------------------------
void CMainWindow::setStatusbarDisplayed( bool value )
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
void CMainWindow::connectDb()
{
  //Connect to database
  QString path = QString("%1/.cache/songbook-client").arg(QDir::home().path());
  QDir dbdir; dbdir.mkpath( path );
  QString dbpath = QString("%1/patacrep.db").arg(path);

  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
  db.setDatabaseName(dbpath);
  if (!db.open())
    {
      QMessageBox::critical(this, tr("Cannot open database"),
			    tr("Unable to establish a database connection.\n"
			       "This application needs SQLite support. "
			       "Click Cancel to exit."), QMessageBox::Cancel);
    }
  if (!QFile::exists(dbpath))
    {
      QSqlQuery query;
      query.exec("create table songs ( artist text, "
		 "title text, "
		 "lilypond bool, "
		 "path text, "
		 "album text, "
		 "cover text, "
		 "lang text)");
    }

  // Initialize the song library
  m_library = new CLibrary();
  m_library->setPathToSongs(workingPath());

  m_proxyModel->setSourceModel(m_library);
  m_proxyModel->setDynamicSortFilter(true);

  m_view->setModel(m_library);
  m_view->setShowGrid( false );
  m_view->setAlternatingRowColors(true);
  m_view->setSelectionMode(QAbstractItemView::MultiSelection);
  m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_view->setSortingEnabled(true);
  m_view->sortByColumn(0, Qt::AscendingOrder);
  m_view->setModel(m_proxyModel);
  m_view->show();
}
//------------------------------------------------------------------------------
void CMainWindow::refreshLibrary()
{
  m_noDataInfo->hide();

  // Retrieve all songs from .sg files in working dir
  m_library->setPathToSongs(workingPath());
  if(!m_library->retrieveSongs())
    m_noDataInfo->show();
  
  m_view->sortByColumn(1, Qt::AscendingOrder);
  m_view->sortByColumn(0, Qt::AscendingOrder);
  m_view->show();
}
//------------------------------------------------------------------------------
void CMainWindow::rebuildLibrary()
{
  //Drop table songs and recreate
  QSqlQuery query("delete from songs");
  refreshLibrary();
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
  m_dbMenu->addAction(m_downloadDbAct);
  m_dbMenu->addAction(m_refreshLibraryAct);
  m_dbMenu->addAction(m_rebuildLibraryAct);

  m_viewMenu = menuBar()->addMenu(tr("&View"));
  m_viewMenu->addAction(m_toolbarViewAct);
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

  connect(m_view, SIGNAL(clicked(const QModelIndex &)),
          m_mapper, SLOT(setCurrentModelIndex(const QModelIndex &)));
  connect(m_view, SIGNAL(clicked(const QModelIndex &)),
          SLOT(updateCover(const QModelIndex &)));

  return layout;
}
//------------------------------------------------------------------------------
QGridLayout * CMainWindow::songbookInfo()
{
  m_sbInfoTitle     = new QLabel;
  m_sbInfoAuthors   = new QLabel;
  m_sbInfoStyle     = new QLabel;
  m_sbInfoSelection = new QLabel;
  
  QPushButton* button = new QPushButton(tr("Settings"));
  connect(button, SIGNAL(clicked()), this, SLOT(templateSettings()));
  
  QGridLayout* layout = new QGridLayout;
  layout->addWidget(new QLabel("<i>Title:</i>"),0,0,1,1);
  layout->addWidget(m_sbInfoTitle,0,1,1,2);
  layout->addWidget(new QLabel("<i>Authors:</i>"),1,0,1,1);
  layout->addWidget(m_sbInfoAuthors,1,1,1,2);
  layout->addWidget(new QLabel("<i>Style:</i>"),2,0,1,1);
  layout->addWidget(m_sbInfoStyle,2,1,1,2);
  layout->addWidget(new QLabel("<i>Selection:</i>"),3,0,1,1);
  layout->addWidget(m_sbInfoSelection,3,1,1,2);
  layout->addWidget(button,4,2,1,1);
  
  return layout;
}
//------------------------------------------------------------------------------
void CMainWindow::updateCover(const QModelIndex & index)
{
  if (!selectionModel()->hasSelection())
    {
#if QT_VERSION >= 0x040600
      m_cover = new QPixmap(QIcon::fromTheme("image-missing").pixmap(128,128));
#else
      m_cover = new QPixmap;
#endif
      m_coverLabel.setPixmap(*m_cover);
      return;
    }

  // do not retrieve last clicked item but last selected item
  QModelIndex lastIndex = selectionModel()->selectedRows().last();
  selectionModel()->setCurrentIndex(lastIndex, QItemSelectionModel::NoUpdate);
  if (lastIndex != index)
    m_mapper->setCurrentModelIndex(lastIndex);

  QString coverpath = m_library->record(m_proxyModel->mapToSource(lastIndex).row()).field("cover").value().toString();
  if (QFile::exists(coverpath))
    m_cover->load(coverpath);
  else
#if QT_VERSION >= 0x040600
    m_cover = new QPixmap(QIcon::fromTheme("image-missing").pixmap(128,128));
#else
  m_cover = new QPixmap;
#endif

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
  QString version = tr("0.4 November 2010");
  QMessageBox::about(this, 
		     tr("About Patacrep Songbook Client"),
		     QString
		     (tr("<br>This program is a client for building and customizing the songbooks available on"
			 " <a href=\"http::www.patacrep.com\">www.patacrep.com</a> </br>"
			 "<br><b>Version:</b> %1 </br>"
			 "<br><b>Authors:</b> Crep (R.Goffe), Lohrun (A.Dupas) </br>")).arg(version));
}
//------------------------------------------------------------------------------
void CMainWindow::selectAll()
{
  m_view->selectAll();
  m_view->setFocus();
}
//------------------------------------------------------------------------------
void CMainWindow::unselectAll()
{
  m_view->clearSelection();
}
//------------------------------------------------------------------------------
void CMainWindow::invertSelection()
{
  QModelIndexList indexes = selectionModel()->selectedRows();
  QModelIndex index;

  m_view->selectAll();

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

  indexes = m_library->match(m_proxyModel->index(0,6), Qt::ToolTipRole, language, -1);

  QItemSelectionModel::SelectionFlags flag = (selection ? QItemSelectionModel::Select : QItemSelectionModel::Deselect) | QItemSelectionModel::Rows;

  foreach(index, indexes)
    {
      selectionModel()->select(index, flag);
    }  
  m_view->setFocus();
}
//------------------------------------------------------------------------------
QStringList CMainWindow::getSelectedSongs()
{
  //ensure the songs are correctly sorted by artist And title
  //todo: do not sort the view but define the < operator
  m_view->sortByColumn(1, Qt::AscendingOrder);
  m_view->sortByColumn(0, Qt::AscendingOrder);

  QStringList songsPath;
  QModelIndexList indexes = selectionModel()->selectedRows();
  QModelIndex index;

  qSort(indexes.begin(), indexes.end());

  foreach(index, indexes)
    {
      songsPath << m_library->record(m_proxyModel->mapToSource(index).row()).field("path").value().toString();
    }

  return songsPath;
}
//------------------------------------------------------------------------------
void CMainWindow::build()
{
  if( getSelectedSongs().isEmpty() )
    {
      if(QMessageBox::question(this, this->windowTitle(), 
			       QString(tr("You did not select any song. \n Do you want to build the songbook with all songs ?")), 
			       QMessageBox::Yes, 
			       QMessageBox::No, 
			       QMessageBox::NoButton) == QMessageBox::No)
	return;
      else
	selectAll();
    }
  
  save(true);

  if (!m_songbook->filename().startsWith(workingPath()))
    {
      statusBar()->showMessage(tr("The songbook is not in the working directory. Build aborted."));
      return;
    }

  if (!m_songbook->filename().endsWith(QString(".sb")))
    {
      statusBar()->showMessage(tr("Wrong filename: songbook does not have \".sb\" extension. Build aborted."));
      return;
    }

  QString target = QString("%1.pdf")
    .arg(QFileInfo(m_songbook->filename()).baseName());

  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  m_buildProcess = new QProcess(this);
  env.insert("LATEX_OPTIONS", "-halt-on-error");
  m_buildProcess->setProcessEnvironment(env);
  m_buildProcess->setWorkingDirectory(workingPath());
  connect(m_buildProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
	  this, SLOT(buildFinished(int,QProcess::ExitStatus)));
  connect(m_buildProcess, SIGNAL(error(QProcess::ProcessError)),
	  this, SLOT(buildError(QProcess::ProcessError)));
  connect(m_buildProcess, SIGNAL(readyReadStandardOutput()),
	  this, SLOT(readProcessOut()));
  connect(m_buildProcess, SIGNAL(readyReadStandardError()),
	  this, SLOT(readProcessOut()));
  m_log->clear();

  statusBar()->showMessage(tr("The songbook is building. Please wait."));
  progressBar()->show();
  m_buildProcess->start("make", QStringList() << target);
}
//------------------------------------------------------------------------------
void CMainWindow::readProcessOut()
{
  m_log->append(m_buildProcess->readAllStandardOutput().data());
}
//------------------------------------------------------------------------------
void CMainWindow::buildFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  if (exitStatus == QProcess::NormalExit && exitCode == 0)
    {
      progressBar()->hide();
      QString msg(tr("Songbook successfully generated."));
      statusBar()->showMessage(msg);

      QString target = QString("%1.pdf").arg(QFileInfo(m_songbook->filename()).baseName());

      QDesktopServices::openUrl(QUrl(QString("file:///%1/%2").arg(m_workingPath).arg(target)));
    }
  else
    {
      progressBar()->hide();

      QMessageBox msgBox;
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.setText(tr("An error occured during the songbook generation.\n You may check compilation logs for more information."));
      msgBox.setDetailedText(m_log->toPlainText());
      msgBox.setStandardButtons(QMessageBox::Cancel);
      msgBox.setDefaultButton(QMessageBox::Cancel);
      msgBox.exec();
    }
}
//------------------------------------------------------------------------------
void CMainWindow::buildError(QProcess::ProcessError error)
{
  progressBar()->hide();

  QMessageBox msgBox;
  msgBox.setIcon(QMessageBox::Critical);
  msgBox.setText(tr("Warning: an error occured during the songbook generation."));
  msgBox.setStandardButtons(QMessageBox::Cancel);
  msgBox.setDefaultButton(QMessageBox::Cancel);
  msgBox.exec();
}
//------------------------------------------------------------------------------
void CMainWindow::clean()
{
  QProcess clean;
  clean.setWorkingDirectory(workingPath());
  statusBar()->showMessage(tr("Cleaning ..."));
  clean.start("make", QStringList() << "clean");
  if (clean.waitForFinished())
    progressBar()->show();
  
  progressBar()->hide();
  statusBar()->showMessage(tr("Cleaning completed."));
}
//------------------------------------------------------------------------------
void CMainWindow::newSongbook()
{
  m_songbook->reset();
  updateTitle(m_songbook->filename());
}
//------------------------------------------------------------------------------
void CMainWindow::open()
{
  QString filename = QFileDialog::getOpenFileName(this,
                                                  tr("Open"),
                                                  workingPath(),
                                                  tr("Songbook (*.sb)"));
  m_songbook->load(filename);
  QStringList songlist = m_songbook->songs();
  QString path = QString("%1/songs/").arg(workingPath());
  songlist.replaceInStrings(QRegExp("^"),path);

  m_view->clearSelection();

  QList<QModelIndex> indexes;
  QString str;
  foreach(str, songlist)
    {
      indexes = m_library->match( m_proxyModel->index(0,3), Qt::MatchExactly, str );
      if (!indexes.isEmpty())
        selectionModel()->select(indexes[0], QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }

  updateTitle(m_songbook->filename());
}
//------------------------------------------------------------------------------
void CMainWindow::save(bool forced)
{
  if(forced)
    {
      if (m_songbook->filename().isEmpty() )
	{
	  m_songbook->setFilename(QString("%1/default.sb").arg(workingPath()));
	}
      else
	{
	  updateSongsList();
	  m_songbook->save(m_songbook->filename());
	  updateTitle(m_songbook->filename());
	}
    }
  else
    {
      if (m_songbook->filename().isEmpty() || 
	  QString::compare(m_songbook->filename(), QString("%1/default.sb").arg(workingPath()))==0 )
	{
	  saveAs();
	}
      else
	{
	  updateSongsList();
	  m_songbook->save(m_songbook->filename());
	  updateTitle(m_songbook->filename());
	}
    }
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
      m_songbook->setFilename(filename);
      save(true);
    }
}
//------------------------------------------------------------------------------
void CMainWindow::updateSongsList()
{
  QStringList songlist = getSelectedSongs();
  QString path = QString("%1/songs/").arg(workingPath());
  songlist.replaceInStrings(path, QString());
  m_songbook->setSongs(songlist);
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
  if (!QDir( m_workingPath ).exists())
    m_workingPath = QDir::currentPath();
  return m_workingPath;
 }
//------------------------------------------------------------------------------
void CMainWindow::setWorkingPath(QString dirname)
{
  if(dirname.endsWith("/"))
    dirname.remove(-1,1);
  bool first = m_workingPath.isEmpty();

  if ( dirname != m_workingPath)
    {
      m_workingPath = dirname;
      emit(workingPathChanged(dirname));

      if (!first && QMessageBox::question
       	  (this, this->windowTitle(),
       	   QString(tr("The songbook directory has been changed.\nWould you like to scan for available songs ?")),
       	   QMessageBox::Yes,
       	   QMessageBox::No,
       	   QMessageBox::NoButton) == QMessageBox::Yes)
	{
	  rebuildLibrary();
	}
    }
}
//------------------------------------------------------------------------------
void CMainWindow::downloadDialog()
{
  new CDownloadDialog(this);
}
//------------------------------------------------------------------------------
QProgressBar * CMainWindow::progressBar() const
{
  return m_progressBar;
}
//------------------------------------------------------------------------------
QItemSelectionModel * CMainWindow::selectionModel()
{
  while (m_library->canFetchMore())
    m_library->fetchMore();

  return m_view->selectionModel();
}
//------------------------------------------------------------------------------
void CMainWindow::songEditor()
{
  if (!selectionModel()->hasSelection())
    {
      QMessageBox msgBox;
      msgBox.setIcon(QMessageBox::Warning);
      msgBox.setText(tr("Please select a song to edit."));
      msgBox.setStandardButtons(QMessageBox::Cancel);
      msgBox.setDefaultButton(QMessageBox::Cancel);
      msgBox.exec();
      return;
    }

  int row = m_proxyModel->mapToSource(selectionModel()->currentIndex()).row();
  QSqlRecord record = m_library->record(row);
  QString title = record.field("title").value().toString();
  QString path = record.field("path").value().toString();

  CSongEditor* editor = new CSongEditor(path);
  if (!editor->isOk)
    {
      delete editor;
      return;
    }
  m_mainWidget->setCurrentIndex(m_mainWidget->addTab(editor, title));
  editor->setTabIndex(m_mainWidget->currentIndex());
  editor->setLabel(title);
  connect(editor, SIGNAL(labelChanged()), this, SLOT(changeTabLabel()));
}
//------------------------------------------------------------------------------
void CMainWindow::changeTabLabel()
{
  QObject *object = QObject::sender();
  if (CSongEditor *editor = qobject_cast< CSongEditor* >(object))
    {
      m_mainWidget->setTabText(editor->tabIndex(), editor->label());
    }
}
//------------------------------------------------------------------------------
void CMainWindow::newSong()
{
  //pop up new song dialog
  m_newSongDialog = new CDialogNewSong();
  connect( m_newSongDialog, SIGNAL(accepted()), this, SLOT(songTemplate()) );
}
//------------------------------------------------------------------------------
bool CMainWindow::checkNewSongRequiredFields()
{
  if(!m_newSongDialog) return false;

  //retrieve user input fields
  QString title = m_newSongDialog->title() ;
  QString artist = m_newSongDialog->artist() ;

  if (title.isEmpty() || artist.isEmpty())
    {
      QMessageBox msgBox;
      msgBox.setIcon(QMessageBox::Warning);
      msgBox.setText(tr("Please fill all required fields."));
      msgBox.setStandardButtons(QMessageBox::Ok);
      msgBox.setDefaultButton(QMessageBox::Ok);
      msgBox.exec();
      return false;
    }
  return true;
}
//------------------------------------------------------------------------------
void CMainWindow::songTemplate()
{
  if ( !checkNewSongRequiredFields() )
    {
      //make a new instance of dialog since it is automtaically destroyed ...
      newSong();
      return;
    }

  //retrieve user input fields
  QString title = m_newSongDialog->title() ;
  QString artist = m_newSongDialog->artist() ;
  uint nbColumns = (uint) m_newSongDialog->nbColumns();
  uint capo = (uint) m_newSongDialog->capo();
  QString album = m_newSongDialog->album() ;
  QString cover = m_newSongDialog->cover() ;
  QString lang = m_newSongDialog->lang() ;

  //remove dialog
  delete m_newSongDialog;

  //todo: better (do not close dialog+highlight missing fields)
  //check required fields
  if (title.isEmpty() || artist.isEmpty())
    {
      QMessageBox msgBox;
      msgBox.setIcon(QMessageBox::Warning);
      msgBox.setText(tr("Please fill all required fields."));
      msgBox.setStandardButtons(QMessageBox::Cancel);
      msgBox.setDefaultButton(QMessageBox::Cancel);
      msgBox.exec();
      return;
    }

  //make new dir
  QString dirpath = QString("%1/songs/%2").arg(workingPath()).arg(filenameConvention(artist,"_"));
  QString filepath = QString("%1/%2.sg").arg(dirpath).arg(filenameConvention(title,"_"));
  QDir dir(dirpath);

  if (!dir.exists())
    dir.mkpath(dirpath);

  //handle album and cover
  bool img = false;
  QFile coverFile(cover);
  if (coverFile.exists())
    {
      //copy in artist directory and resize
      QFileInfo fi(cover);
      QString target = QString("%1/songs/%2/%3").arg(workingPath()).arg(filenameConvention(artist,"_")).arg(fi.fileName());
      img = coverFile.copy(target);
      QFile copyCover(target);

      //if album is specified, rename cover accordingly
      if (!album.isEmpty()
	  && !copyCover.rename(QString("%1/songs/%2/%3.jpg")
			       .arg(workingPath())
			       .arg(filenameConvention(artist,"_"))
			       .arg(filenameConvention(album,"-"))))
	copyCover.remove(); //remove copy if file already exists
    }

  //make template
  QFile file(filepath);
  QString songTemplate;
  songTemplate.append(QString("\\selectlanguage{%1}\n").arg(lang));
  if (nbColumns > 0)
    songTemplate.append(QString("\\songcolumns{%1}\n").arg(nbColumns));
  
  songTemplate.append(QString("\\beginsong{%1}[by=%2").arg(title).arg(artist));
  
  if(!album.isEmpty())
    songTemplate.append(QString(",album=%1").arg(album));
  
  if(img)
    songTemplate.append(QString(",cov=%1").arg(filenameConvention(album,"-")));

  songTemplate.append(QString("]\n\n"));
  
  if(img)
    songTemplate.append(QString("\\cover\n"));

  if (capo>0)
    songTemplate.append(QString("\\capo{%1}\n").arg(capo));
  
  songTemplate.append(QString("\n\\endsong"));

  if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
      QTextStream stream (&file);
      stream << songTemplate;
      file.close();
    }
  else
    {
      qDebug() << " CMainWindow::newsong unable to open file " << filepath << " in write mode ";
    }

  //Insert the song in the library
  m_library->addSongFromFile(filepath);
  m_library->submitAll();
  m_view->sortByColumn(1, Qt::AscendingOrder);
  m_view->sortByColumn(0, Qt::AscendingOrder);

  //position index of new song in the library and launch song editor
  CSongEditor* editor = new CSongEditor(filepath);
  m_mainWidget->setCurrentIndex(m_mainWidget->addTab(editor, title));
  editor->setTabIndex(m_mainWidget->currentIndex());
  editor->setLabel(title);
  connect(editor, SIGNAL(labelChanged()), this, SLOT(changeTabLabel()));
}
//------------------------------------------------------------------------------
QString CMainWindow::filenameConvention(const QString & str, const QString & sep)
{
  QString result = str;
  QString tmp;
  QStringList list;
  list <<"é"<<"è"<<"ê"<<"ë";
  foreach(tmp, list)
    result.replace(tmp, QString("e"));

  list = QStringList();
  list <<" "<<"&"<<"'"<<"`"<<"("<<")"<<"["<<"]"<<"{"<<"}"<<"_"<<"~"<<","<<"?"<<"!"<<":"<<";"<<"."<<"%";
  foreach(tmp, list)
    result.replace(tmp, sep);

  result.replace(QString("à"), QString("a"));
  result.replace(QString("â"), QString("a"));
  result.replace(QString("ï"), QString("i"));
  result.replace(QString("î"), QString("i"));
  result.replace(QString("ô"), QString("o"));
  result.replace(QString("ù"), QString("u"));

  return result;
}
//------------------------------------------------------------------------------
void CMainWindow::deleteSong()
{
  if (!selectionModel()->hasSelection())
    {
      QMessageBox msgBox;
      msgBox.setIcon(QMessageBox::Warning);
      msgBox.setText(tr("Please select a song to remove."));
      msgBox.setStandardButtons(QMessageBox::Cancel);
      msgBox.setDefaultButton(QMessageBox::Cancel);
      msgBox.exec();
      return;
    }

  QString path = m_library->record(m_proxyModel->mapToSource(selectionModel()->currentIndex()).row()).field("path").value().toString();

  if (QMessageBox::question
     (this, this->windowTitle(),
      QString(tr("Are you sure you want to permanently remove the file %1 ?")).arg(path),
      QMessageBox::Yes,
      QMessageBox::No,
      QMessageBox::NoButton) == QMessageBox::Yes)
    {
      //remove entry in database
      QSqlQuery query;
      query.exec(QString("DELETE FROM songs WHERE path = '%1'").arg(path));

      m_library->setPathToSongs(workingPath());
      //update models and display the song list
      m_proxyModel->setSourceModel(m_library);
      m_view->setModel(m_library);
      m_view->sortByColumn(0, Qt::AscendingOrder);
      m_view->setModel(m_proxyModel);
      m_view->sortByColumn(1, Qt::AscendingOrder);
      m_view->sortByColumn(0, Qt::AscendingOrder);

      //removal on disk
      QFile file(path);
      QFileInfo fileinfo(file);
      QString tmp = fileinfo.canonicalPath();
      if (file.remove())
	{
	  QDir dir;
	  dir.rmdir(tmp); //remove dir if empty
	  clean();
	  //once deleted move selection in the model
	  updateCover(selectionModel()->currentIndex());
	  m_mapper->setCurrentModelIndex(selectionModel()->currentIndex());
	}
    }
}
//------------------------------------------------------------------------------
void CMainWindow::closeTab(int index)
{
  //forbid to close main tab
  if(index!=0)
    m_mainWidget->closeTab(index);
}
//------------------------------------------------------------------------------
void CMainWindow::changeTab(int index)
{
  //avoid shortcuts conflicts
  if(index!=0)
    m_saveAct->setShortcutContext(Qt::WidgetShortcut);
  else
    m_saveAct->setShortcutContext(Qt::WindowShortcut);
}

//******************************************************************************
//******************************************************************************
CTabWidget::CTabWidget():QTabWidget()
{
  tabBar()->hide();
  QAction* action = new QAction(tr("Next tab"), this);
  action->setShortcut(QKeySequence::NextChild);
  connect(action, SIGNAL(triggered()), this, SLOT(next()));
  action = new QAction(tr("Previous tab"), this);
  action->setShortcut(QKeySequence::PreviousChild);
  connect(action, SIGNAL(triggered()), this, SLOT(prev()));
}
//------------------------------------------------------------------------------
CTabWidget::~CTabWidget()
{}
//------------------------------------------------------------------------------
void CTabWidget::closeTab(int index)
{
  removeTab(index);
  if (count() == 1)
    tabBar()->hide();
}
//------------------------------------------------------------------------------
int CTabWidget::addTab(QWidget* widget, const QString & label)
{
  int res = QTabWidget::addTab(widget, label);
  tabBar()->show();
  if (count() == 1)
    tabBar()->hide();
  return res;
}
//------------------------------------------------------------------------------
void CTabWidget::next()
{
  setCurrentIndex(currentIndex()+1);
}
//------------------------------------------------------------------------------
void CTabWidget::prev()
{
  setCurrentIndex(currentIndex()-1);
}
//******************************************************************************
//******************************************************************************
