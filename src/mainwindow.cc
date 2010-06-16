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

#include "label.hh"

#include "mainwindow.hh"
#include "preferences.hh"
#include "library.hh"
#include "songbook.hh"
#include "tools.hh"
#include "download.hh"
#include "song-editor.hh"
#include "dialog-new-song.hh"

#include <QDebug>
//******************************************************************************
CMainWindow::CMainWindow()
  : QMainWindow()
  , m_library()
  , m_proxyModel(new QSortFilterProxyModel)
  , m_songbook()
  , m_view(new QTableView)
  , m_progressBar(new QProgressBar)
  , m_cover(new QPixmap)
{
  setWindowTitle("Patacrep Songbook Client");
  setWindowIcon(QIcon(":/icons/patacrep.png"));

  readSettings();

  // main document and title
  m_songbook = new CSongbook();
  connect(m_songbook, SIGNAL(wasModified(bool)),
          this, SLOT(setWindowModified(bool)));
  updateTitle(m_songbook->filename());

  createActions();
  createMenus();

  //Connection to database
  if (connectDb())
    synchroniseWithLocalSongs();
  else
    applyDisplayColumn();

  // initialize the filtering proxy
  m_proxyModel->setDynamicSortFilter(true);

  // filtering related widgets
  QLineEdit *filterLineEdit = new QLineEdit;
  QLabel *filterLabel = new QLabel(tr("&Filter:"));
  filterLabel->setBuddy(filterLineEdit);
  QComboBox *filterComboBox = new QComboBox;
  filterComboBox->addItem(tr("All"), -1);
  filterComboBox->addItem(tr("Artist"), 0);
  filterComboBox->addItem(tr("Title"), 1);
  filterComboBox->addItem(tr("Album"), 4);
  m_proxyModel->setFilterKeyColumn(-1);

  connect(filterLineEdit, SIGNAL(textChanged(QString)),
	  this, SLOT(filterChanged()));
  connect(filterComboBox, SIGNAL(currentIndexChanged(int)),
	  this, SLOT(filterChanged()));

  QBoxLayout *filterLayout = new QHBoxLayout;
  filterLayout->addWidget(filterLabel);
  filterLayout->addWidget(filterLineEdit);
  filterLayout->addWidget(filterComboBox);

  // toolbar (for the build button)
  QToolBar *toolbar = new QToolBar;
  toolbar->setMovable(false);
  toolbar->addAction(m_newAct);
  toolbar->addAction(m_openAct);
  toolbar->addAction(m_saveAct);
  toolbar->addAction(m_saveAsAct);
  toolbar->addSeparator();
  toolbar->addAction(m_buildAct);

  // organize the toolbar and the filter into an horizontal layout
  QBoxLayout *horizontalLayout = new QHBoxLayout;
  horizontalLayout->addWidget(toolbar);
  horizontalLayout->addStretch();
  horizontalLayout->addLayout(filterLayout);

  // Main widgets
  //  QWidget *mainWidget = new QWidget;
  QBoxLayout *mainLayout = new QHBoxLayout;
  QBoxLayout *leftLayout = new QVBoxLayout;
  leftLayout->addWidget(new QLabel(tr("<b>Songbook</b>")));
  leftLayout->addWidget(m_songbook->panel());
  leftLayout->addWidget(new QLabel(tr("<b>Song</b>")));
  leftLayout->addWidget(createSongInfoWidget());
  leftLayout->addStretch();
  mainLayout->addLayout(leftLayout);
  mainLayout->setStretch(0,1);
  mainLayout->addWidget(m_view);
  mainLayout->setStretch(1,2);
  //  mainWidget->setLayout(mainLayout);

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
	   m_mainWidget, SLOT(closeTab(int)) );
  m_mainWidget->addTab(libraryTab, tr("Library"));
  setCentralWidget(m_mainWidget);

  // auto adjust column display
  applyDisplayColumn();

  // Debugger Info DockWidget
  m_logInfo = new QDockWidget( tr("Logs"), this );
  m_logInfo->setMinimumWidth(250);
  m_log = new QTextEdit;
  m_log->setReadOnly(true);
  m_logInfo->setWidget(m_log);
  addDockWidget( Qt::BottomDockWidgetArea, m_logInfo );
  m_logInfo->setVisible(false);

  // status bar with an embedded progress bar on the right
  m_progressBar->setTextVisible(false);
  m_progressBar->setRange(0, 0);
  m_progressBar->hide();
  statusBar()->addPermanentWidget(m_progressBar);
  statusBar()->showMessage(tr("A context menu is available by right-clicking"));
}
//------------------------------------------------------------------------------
void CMainWindow::filterChanged()
{
  QObject *object = QObject::sender();

  if (QLineEdit *lineEdit = qobject_cast< QLineEdit* >(object))
    {
      QRegExp expression = QRegExp(lineEdit->text(), Qt::CaseInsensitive, QRegExp::FixedString);
      m_proxyModel->setFilterRegExp(expression);
    }
  else if (QComboBox *comboBox = qobject_cast< QComboBox* >(object))
    {
      int column = comboBox->itemData(comboBox->currentIndex()).toInt();
      m_proxyModel->setFilterKeyColumn(column);
    }
  else
    {
      qWarning() << "Unknown caller to filterChanged.";
    }
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

  setWorkingPath(settings.value("workingPath", QString("%1/").arg(QDir::currentPath())).toString());

  settings.beginGroup("display");
  m_displayColumnArtist = settings.value("artist", true).toBool();
  m_displayColumnTitle = settings.value("title", true).toBool();
  m_displayColumnPath = settings.value("path", false).toBool();
  m_displayColumnAlbum = settings.value("album", true).toBool();
  m_displayColumnLilypond = settings.value("lilypond", false).toBool();
  m_displayColumnCover = settings.value("cover", true).toBool();
  settings.endGroup();
}
//------------------------------------------------------------------------------
void CMainWindow::writeSettings()
{
  QSettings settings;

  settings.setValue("mainWindow/size", size());
}
//------------------------------------------------------------------------------
void CMainWindow::applyDisplayColumn()
{
  m_view->setColumnHidden(0,!m_displayColumnArtist);
  m_view->setColumnHidden(1,!m_displayColumnTitle);
  m_view->setColumnHidden(3,!m_displayColumnPath);
  m_view->setColumnHidden(4,!m_displayColumnAlbum);
  m_view->setColumnHidden(2,!m_displayColumnLilypond);
  m_view->setColumnHidden(5,!m_displayColumnCover);
  m_view->resizeColumnsToContents();
}
//------------------------------------------------------------------------------
void CMainWindow::setDisplaySongInfo(bool value)
{
  m_songInfo->setVisible(value);
}
//------------------------------------------------------------------------------
void CMainWindow::setDisplayLogInfo(bool value)
{
  m_logInfo->setVisible(value);
}
//------------------------------------------------------------------------------
void CMainWindow::createActions()
{
  m_newAct = new QAction(QIcon(":/icons/document-new.png"), tr("New"), this);
  m_newAct->setShortcut(tr("Ctrl+N"));
  m_newAct->setStatusTip(tr("New songbook"));
  connect(m_newAct, SIGNAL(triggered()), this, SLOT(newSongbook()));

  m_openAct = new QAction(QIcon(":/icons/document-load.png"), tr("Open"), this);
  m_openAct->setShortcut(tr("Ctrl+O"));
  m_openAct->setStatusTip(tr("Open a songbook"));
  connect(m_openAct, SIGNAL(triggered()), this, SLOT(open()));

  m_saveAct = new QAction(QIcon(":/icons/document-save.png"), tr("Save"), this);
  m_saveAct->setShortcut(tr("Ctrl+S"));
  m_saveAct->setStatusTip(tr("Save the current songbook"));
  connect(m_saveAct, SIGNAL(triggered()), this, SLOT(save()));

  m_saveAsAct = new QAction(QIcon(":/icons/document-save.png"),
                            tr("SaveAs"), this);
  m_saveAsAct->setShortcut(tr("Maj+Ctrl+S"));
  m_saveAsAct->setStatusTip(tr("Save the current songbook as"));
  connect(m_saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

  m_aboutAct = new QAction(tr("&About"), this);
  m_aboutAct->setStatusTip(tr("Show the application's About box"));
  connect(m_aboutAct, SIGNAL(triggered()), this, SLOT(about()));

  m_exitAct = new QAction(tr("Exit"), this);
  m_exitAct->setShortcut(tr("Ctrl+Q"));
  m_exitAct->setStatusTip(tr("Exit the application"));
  connect(m_exitAct, SIGNAL(triggered()), this, SLOT(close()));

  m_newSongAct = new QAction(QIcon(":/icons/document-load.png"), tr("New Song"), this);
  // m_newSongAct->setShortcut(tr("Ctrl+N"));
  m_newSongAct->setStatusTip(tr("Write a new song."));
  connect(m_newSongAct, SIGNAL(triggered()), this, SLOT(newSong()));

  m_buildAct = new QAction(tr("Build PDF"), this);
  m_buildAct->setShortcut(tr("Ctrl+B"));
  m_buildAct->setStatusTip(tr("Generate pdf from selected songs."));
  connect(m_buildAct, SIGNAL(triggered()), this, SLOT(build()));

  m_cleanAct = new QAction(QIcon(":/icons/edit-clear.png"), tr("Clean"), this);
  m_cleanAct->setStatusTip(tr("Clean"));
  connect(m_cleanAct, SIGNAL(triggered()), this, SLOT(clean()));

  m_preferencesAct = new QAction(tr("&Preferences"), this);
  m_preferencesAct->setStatusTip(tr("Select your preferences."));
  connect(m_preferencesAct, SIGNAL(triggered()), SLOT(preferences()));

  m_selectAllAct = new QAction(tr("Select all"), this);
  m_selectAllAct->setStatusTip(tr("Select all displayed songs."));
  connect(m_selectAllAct, SIGNAL(triggered()), SLOT(selectAll()));

  m_unselectAllAct = new QAction(tr("Unselect all"), this);
  m_unselectAllAct->setStatusTip(tr("Unselect all displayed songs."));
  connect(m_unselectAllAct, SIGNAL(triggered()), SLOT(unselectAll()));

  m_invertSelectionAct = new QAction(tr("Invert Selection"), this);
  m_invertSelectionAct->setStatusTip(tr("Invert currently selected songs."));
  connect(m_invertSelectionAct, SIGNAL(triggered()), SLOT(invertSelection()));

  m_displaySongInfoAct = new QAction(tr("Display Song Info"), this);
  m_displaySongInfoAct->setStatusTip(tr("Display information about the last selected song."));
  m_displaySongInfoAct->setCheckable(true);
  m_displaySongInfoAct->setChecked(true);
  connect(m_displaySongInfoAct, SIGNAL(toggled(bool)), SLOT(setDisplaySongInfo(bool)));

  m_displayLogInfoAct = new QAction(tr("Display Log Info"), this);
  m_displayLogInfoAct->setStatusTip(tr("Output out the LaTeX compilation process."));
  m_displayLogInfoAct->setCheckable(true);
  m_displayLogInfoAct->setChecked(false);
  connect(m_displayLogInfoAct, SIGNAL(toggled(bool)), SLOT(setDisplayLogInfo(bool)));

  m_adjustColumnsAct = new QAction(tr("Auto Adjust Columns"), this);
  m_adjustColumnsAct->setStatusTip(tr("Adjust columns to contents."));
  connect(m_adjustColumnsAct, SIGNAL(triggered()), SLOT(applyDisplayColumn()));

  m_connectDbAct = new QAction(QIcon(":/icons/network-server.png"),
			       tr("Connection to local database"), this);
  m_connectDbAct->setStatusTip(tr("Connection to local database."));
  connect(m_connectDbAct, SIGNAL(triggered()), SLOT(connectDb()));

  m_rebuildDbAct = new QAction(QIcon(":/icons/view-refresh.png"),
			       tr("Synchronise"), this);
  m_rebuildDbAct->setStatusTip(tr("Rebuild database from local songs."));
  connect(m_rebuildDbAct, SIGNAL(triggered()), SLOT(synchroniseWithLocalSongs()));

  m_downloadDbAct = new QAction("Download",this);
  m_downloadDbAct->setStatusTip(tr("Download songs from Patacrep!"));
  connect(m_downloadDbAct, SIGNAL(triggered()), this, SLOT(downloadDialog()));

  CTools* tools = new CTools(workingPath(), this);
  m_resizeCoversAct = new QAction( tr("Resize covers"), this);
  m_resizeCoversAct->setStatusTip(tr("Ensure that covers are correctly resized in songbook directory."));
  connect(m_resizeCoversAct, SIGNAL(triggered()), tools, SLOT(resizeCovers()));

  m_checkerAct = new QAction( tr("Global check"), this);
  m_checkerAct->setStatusTip(tr("Check for common mistakes in songs (e.g spelling, chords, LaTeX typo ...)."));
  connect(m_checkerAct, SIGNAL(triggered()), tools, SLOT(globalCheck()));
}
//------------------------------------------------------------------------------
bool CMainWindow::connectDb()
{
  //Connect to database
  bool newdb = createDbConnection();

  // Initialize the song library
  m_library = new CLibrary();
  m_library->setPathToSongs(workingPath());

  // Display the song list
  m_proxyModel->setSourceModel(m_library);
  m_view->setModel(m_library);
  m_view->setShowGrid( false );
  m_view->setAlternatingRowColors(true);
  m_view->setSortingEnabled(true);
  m_view->setSelectionMode(QAbstractItemView::MultiSelection);
  m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_view->sortByColumn(1, Qt::AscendingOrder);
  m_view->sortByColumn(0, Qt::AscendingOrder);
  m_view->setModel(m_proxyModel);
  m_view->show();

  return newdb;
}
//------------------------------------------------------------------------------
void CMainWindow::synchroniseWithLocalSongs()
{
  //Drop table songs and recreate
  QSqlQuery query("delete from songs");

  // Retrieve all songs from .sg files in working dir
  m_library->setPathToSongs(workingPath());
  m_library->retrieveSongs();
  m_view->sortByColumn(1, Qt::AscendingOrder);
  m_view->sortByColumn(0, Qt::AscendingOrder);
  m_view->show();
  applyDisplayColumn();
}
//------------------------------------------------------------------------------
void CMainWindow::contextMenuEvent(QContextMenuEvent *event)
{
  QMenu *menu = new QMenu();
  menu->addAction(m_selectAllAct);
  menu->addAction(m_unselectAllAct);
  menu->addAction(m_invertSelectionAct);
  menu->exec(event->globalPos());
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
  m_fileMenu = menuBar()->addMenu(tr("&File"));
  m_fileMenu->addAction(m_newAct);
  m_fileMenu->addAction(m_openAct);
  m_fileMenu->addAction(m_saveAct);
  m_fileMenu->addAction(m_saveAsAct);
  m_fileMenu->addSeparator();
  m_fileMenu->addAction(m_newSongAct);
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

  m_dbMenu = menuBar()->addMenu(tr("&Database"));
  m_dbMenu->addAction(m_downloadDbAct);
  m_dbMenu->addAction(m_rebuildDbAct);

  m_viewMenu = menuBar()->addMenu(tr("&View"));
  m_viewMenu->addAction(m_displaySongInfoAct);
  m_viewMenu->addAction(m_displayLogInfoAct);
  m_viewMenu->addAction(m_adjustColumnsAct);

  m_viewMenu = menuBar()->addMenu(tr("&Tools"));
  m_viewMenu->addAction(m_resizeCoversAct);
  m_viewMenu->addAction(m_checkerAct);

  m_helpMenu = menuBar()->addMenu(tr("&Help"));
  m_helpMenu->addAction(m_aboutAct);
}
//------------------------------------------------------------------------------
QWidget * CMainWindow::createSongInfoWidget()
{
  QWidget * songInfoWidget = new QWidget();

  CLabel *artistLabel = new CLabel();
  artistLabel->setElideMode(Qt::ElideRight);
  CLabel *titleLabel = new CLabel();
  titleLabel->setElideMode(Qt::ElideRight);
  CLabel *albumLabel = new CLabel();
  albumLabel->setElideMode(Qt::ElideRight);

  QGroupBox* currentSongTagsBox = new QGroupBox;
  QGridLayout *songInfoLayout = new QGridLayout();
  songInfoLayout->addWidget(new QLabel(tr("<b>Song:</b>")),0,0,1,1,Qt::AlignLeft);
  songInfoLayout->addWidget(titleLabel,0,1,1,1);
  songInfoLayout->addWidget(new QLabel(tr("<b>Artist:</b>")),1,0,1,1,Qt::AlignLeft);
  songInfoLayout->addWidget(artistLabel,1,1,1,1);
  songInfoLayout->addWidget(new QLabel(tr("<b>Album:</b>")),2,0,1,1,Qt::AlignLeft);
  songInfoLayout->addWidget(albumLabel,2,1,1,1);
  songInfoLayout->setColumnStretch(2,1);
  songInfoLayout->setRowStretch(3,10);
  currentSongTagsBox->setLayout(songInfoLayout);

  QDialogButtonBox * buttonBox = new QDialogButtonBox;
  QPushButton* editButton = new QPushButton(tr("Edit"));
  QPushButton * deleteButton = new QPushButton(tr("Delete"));
  editButton->setDefault(true);
  buttonBox->addButton(editButton, QDialogButtonBox::ActionRole);
  buttonBox->addButton(deleteButton, QDialogButtonBox::ActionRole);

  connect(editButton, SIGNAL(clicked()), SLOT(songEditor()));
  connect(deleteButton, SIGNAL(clicked()), SLOT(deleteSong()));

  m_currentSongWidgetLayout = new QBoxLayout(QBoxLayout::TopToBottom, songInfoWidget);
  m_coverLabel.setAlignment(Qt::AlignTop);
  m_currentSongWidgetLayout->addWidget(&m_coverLabel);
  m_currentSongWidgetLayout->addWidget(currentSongTagsBox);
  m_currentSongWidgetLayout->addWidget(buttonBox);
  m_currentSongWidgetLayout->addStretch(1);

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

  return songInfoWidget;
}
//------------------------------------------------------------------------------
void CMainWindow::dockWidgetDirectionChanged(Qt::DockWidgetArea area)
{
  if (area==Qt::LeftDockWidgetArea || area==Qt::RightDockWidgetArea)
    {
      m_currentSongWidgetLayout->setDirection(QBoxLayout::TopToBottom);
      m_songInfo->setMaximumSize(300, 300);
    }
  else
    {
      m_currentSongWidgetLayout->setDirection(QBoxLayout::LeftToRight);
      m_songInfo->setMaximumSize(450, 170);
    }
}
//------------------------------------------------------------------------------
void CMainWindow::updateCover(const QModelIndex & index)
{
  if (!selectionModel()->hasSelection())
    {
      m_cover->load(":/icons/unavailable-large");
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
    m_cover->load(":/icons/unavailable-large");
  m_coverLabel.setPixmap(*m_cover);
}
//------------------------------------------------------------------------------
void CMainWindow::preferences()
{
  ConfigDialog dialog;
  dialog.exec();
  readSettings();
  applyDisplayColumn();
}
//------------------------------------------------------------------------------
void CMainWindow::about()
{
  QMessageBox::about(this, tr("About Patacrep Songbook Client"),
		     tr("<br>This program is a client for building pdf songbooks with LaTeX. </br> "
			"<br>Songbooks may represent lyrics, guitar chords or sheets for the songs available on"
			" <a href=\"http::www.patacrep.com\">www.patacrep.com</a> </br>"
			"<br>You may clone the <a href=\"git://git.lohrun.net/songbook.git\">songbook repository</a> </br>"
			"<br><b>Version:</b> 0.1 April 25th, 2010 </br>"
			"<br><b>Authors:</b> Crep (R.Goffe), Lohrun (A.Dupas) </br>"));
}
//------------------------------------------------------------------------------
void CMainWindow::selectAll()
{
  m_view->selectAll();
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
QStringList CMainWindow::getSelectedSongs()
{
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
  save();

  if (!m_songbook->filename().startsWith(workingPath()))
    {
      statusBar()->showMessage(tr("The songbook is not in the working directory. Build aborted."));
      return;
    }

  if (!m_songbook->filename().endsWith(QString(".sb")))
    {
      statusBar()->showMessage(tr("Wrong filename. Build aborted."));
      return;
    }

  QString target = QString("%1.pdf")
    .arg(QFileInfo(m_songbook->filename()).baseName());

  m_buildProcess = new QProcess(this);
  m_buildProcess->setWorkingDirectory(workingPath());
  connect(m_buildProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
	  this, SLOT(buildFinished(int,QProcess::ExitStatus)));
  connect(m_buildProcess, SIGNAL(error(QProcess::ProcessError)),
	  this, SLOT(buildError(QProcess::ProcessError)));
  connect(m_buildProcess, SIGNAL(readyReadStandardOutput()),
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
      m_progressBar->hide();
      QString msg(tr("Songbook successfully generated."));
      statusBar()->showMessage(msg);

      QString target = QString("%1.pdf").arg(QFileInfo(m_songbook->filename()).baseName());

      QDesktopServices::openUrl(QUrl(QString("file:///%1/%2").arg(m_workingPath).arg(target)));
    }
}
//------------------------------------------------------------------------------
void CMainWindow::buildError(QProcess::ProcessError error)
{
  QMessageBox msgBox;
  msgBox.setIcon(QMessageBox::Critical);
  msgBox.setText(tr("Sorry: an error occured during the songbook generation."));
  msgBox.setStandardButtons(QMessageBox::Cancel);
  msgBox.setDefaultButton(QMessageBox::Cancel);
  msgBox.exec();
}
//------------------------------------------------------------------------------
void CMainWindow::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  if (exitStatus == QProcess::NormalExit && exitCode == 0)
    {
      m_progressBar->hide();
      statusBar()->showMessage(tr("Success!"));
    }
}
//------------------------------------------------------------------------------
void CMainWindow::clean()
{
  QProcess clean;
  clean.setWorkingDirectory(workingPath());
  statusBar()->showMessage(tr("Cleaning ..."));
  clean.start("make", QStringList() << "clean");
  if (!clean.waitForFinished()) return;
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
void CMainWindow::save()
{
  if (m_songbook->filename().isEmpty())
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
//------------------------------------------------------------------------------
void CMainWindow::saveAs()
{
  QString filename = QFileDialog::getSaveFileName(this,
                                                  tr("Save as"),
                                                  workingPath(),
                                                  tr("Songbook (*.sb)"));
  m_songbook->setFilename(filename);
  save();
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
  if (QDir( m_workingPath ).exists())
    return m_workingPath;
  else
    return QDir::currentPath();
 }
//------------------------------------------------------------------------------
void CMainWindow::setWorkingPath( QString dirname )
{
  m_workingPath = dirname;
}
//------------------------------------------------------------------------------
bool CMainWindow::createDbConnection()
{
  QString path = QString("%1/.cache/songbook-client").arg(QDir::home().path());
  QDir dbdir; dbdir.mkdir( path );
  QString dbpath = QString("%1/patacrep.db").arg(path);

  bool exist = QFile::exists(dbpath);

  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
  db.setDatabaseName(dbpath);
  if (!db.open())
    {
      QMessageBox::critical(this, tr("Cannot open database"),
			    tr("Unable to establish a database connection.\n"
			       "This application needs SQLite support. "
			       "Click Cancel to exit."), QMessageBox::Cancel);
      return false;
    }
  if (exist)
    return false;

  QSqlQuery query;
  query.exec("create table songs ( artist char(80), "
	     "title char(80), "
	     "lilypond bool, "
	     "path char(80), "
	     "album char(80), "
	     "cover char(80))");

  return true;
}
//------------------------------------------------------------------------------
void CMainWindow::downloadDialog()
{
  new CDownloadDialog(this);
}
//------------------------------------------------------------------------------
QProgressBar * CMainWindow::progressBar()
{
  return m_progressBar;
}
//------------------------------------------------------------------------------
QItemSelectionModel * CMainWindow::selectionModel()
{
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
  CDialogNewSong *dialog = new CDialogNewSong();
  connect(dialog, SIGNAL(accepted()), this, SLOT(songTemplate()));
}
//------------------------------------------------------------------------------
void CMainWindow::songTemplate()
{
  QObject *object = QObject::sender();
  if (CDialogNewSong *dialog = qobject_cast< CDialogNewSong* >(object))
    {
      //retrieve user input fields
      QString title = dialog->title() ;
      QString artist = dialog->artist() ;
      uint nbColumns = (uint) dialog->nbColumns();
      uint capo = (uint) dialog->capo();
      QString album = dialog->album() ;
      QString cover = dialog->cover() ;

      //remove dialog
      delete dialog;

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
	  qDebug() << "new file copy " << cover << "in "<< target;
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
      if (nbColumns > 0)
        songTemplate.append(QString("\\songcolumns{%1}\n").arg(nbColumns));
      if (!img)
	songTemplate.append(QString("\\beginsong{%1}[by=%2]\n\n").arg(title).arg(artist));
      else
	songTemplate.append(QString("\\beginsong{%1}[by=%2,cov=%3]\n\n\\cover\n").arg(title).arg(artist).arg(filenameConvention(album,"-")));

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
      QString(tr("This will remove the file %1 ?")).arg(path),
      QMessageBox::Yes,
      QMessageBox::No,
      QMessageBox::NoButton) == QMessageBox::Yes)
    {
      //todo: debug
      //remove entry in database
      //QModelIndexList list = m_library->match(m_proxyModel->index(0,3), Qt::MatchExactly, path);
      //QModelIndex index;
      //foreach(index, list)
      //	removeRow(index.row());
      //submitAll();
      //removal on disk
      QFile file(path);
      QFileInfo fileinfo(file);
      QString tmp = fileinfo.canonicalPath();
      if (file.remove())
	{
	  QDir dir;
	  dir.rmdir(tmp); //remove dir if empty
	  synchroniseWithLocalSongs(); //temporary hack
	  //once deleted move selection in the model
	  updateCover(selectionModel()->currentIndex());
	  m_mapper->setCurrentModelIndex(selectionModel()->currentIndex());
	}
    }
}
//******************************************************************************
//******************************************************************************
CTabWidget::CTabWidget():QTabWidget()
{
  tabBar()->hide();
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
  if (count() == 1)
    tabBar()->hide();
  else
    tabBar()->show();
  return res;
}
//******************************************************************************
//******************************************************************************
