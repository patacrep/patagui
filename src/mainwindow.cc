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

#include "mainwindow.hh"
#include "preferences.hh"
#include "library.hh"
#include "songbook.hh"
#include "tools.hh"
#include "download.hh"
#include "song-editor.hh"

#include <QDebug>
//******************************************************************************
CMainWindow::CMainWindow()
  : QMainWindow()
  , m_library()
  , m_proxyModel(new QSortFilterProxyModel)
  , m_view(new QTableView)
  , m_progressBar(new QProgressBar)
  , m_cover(new QPixmap)
{
  setWindowTitle("Patacrep Songbook Client");
  setWindowIcon(QIcon(":/icons/patacrep.png"));

  readSettings();

  createActions();
  createMenus();

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
  toolbar->addAction(m_buildAct);

  // organize the toolbar and the filter into an horizontal layout
  QBoxLayout *horizontalLayout = new QHBoxLayout;
  horizontalLayout->addWidget(toolbar);
  horizontalLayout->addStretch();
  horizontalLayout->addLayout(filterLayout);

  // place elements into the main window
  m_mainWidget = new QTabWidget;
  m_mainWidget->setTabsClosable(true);
  m_mainWidget->setMovable(true);
  connect( m_mainWidget, SIGNAL(tabCloseRequested(int)),
	   this, SLOT(closeTab(int)) );
  setCentralWidget(m_mainWidget);

  QWidget* libraryTab = new QWidget;
  QBoxLayout *libraryLayout = new QVBoxLayout;
  libraryLayout->addLayout(horizontalLayout);
  libraryLayout->addWidget(m_view);
  libraryTab->setLayout(libraryLayout);
  m_mainWidget->addTab(libraryTab, tr("Library"));


  //Connection to database
  connectDb();

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

  settings.beginGroup("options");
  m_bookTypeChordbook = settings.value("chordbook", true).toBool();
  m_bookTypeLyricbook = settings.value("lyricbook", true).toBool();
  m_optionChordDiagram = settings.value("chordDiagram", true).toBool();
  m_optionLilypond = settings.value("lilypond", true).toBool();
  m_optionTablature = settings.value("tablature", true).toBool();
  settings.endGroup();

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
QString CMainWindow::packageOptions()
{
  QString options;
  if( m_bookTypeChordbook )
    {
      options = QString("chorded");
      if( m_optionChordDiagram )
        options.append(""); // not supported yet by the latex crepbook class
      if( m_optionLilypond )
        options.append(",lilypond");
      if( m_optionTablature )
        options.append(",tabs");
    }
  else if( m_bookTypeLyricbook )
    {
      options = QString("lyric");
    }
  return options;
}
//------------------------------------------------------------------------------
void CMainWindow::createActions()
{
  m_exitAct = new QAction(tr("Exit"), this);
  m_exitAct->setShortcut(tr("Ctrl+Q"));
  m_exitAct->setStatusTip(tr("Exit the application"));
  connect(m_exitAct, SIGNAL(triggered()), this, SLOT(close()));

  m_openAct = new QAction(QIcon(":/icons/document-load.png"), tr("Load Songs List"), this);
  m_openAct->setShortcut(tr("Ctrl+O"));
  m_openAct->setStatusTip(tr("Open a list of songs (.sgl) previously saved."));
  connect(m_openAct, SIGNAL(triggered()), this, SLOT(open()));

  m_saveAct = new QAction(QIcon(":/icons/document-save.png"), tr("Save Songs List"), this);
  m_saveAct->setShortcut(tr("Ctrl+S"));
  m_saveAct->setStatusTip(tr("Save the list of currently selected songs."));
  connect(m_saveAct, SIGNAL(triggered()), this, SLOT(save()));

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

  m_aboutAct = new QAction(tr("&About"), this);
  m_aboutAct->setStatusTip(tr("Show the application's About box"));
  connect(m_aboutAct, SIGNAL(triggered()), this, SLOT(about()));

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
void CMainWindow::connectDb()
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
  m_view->resizeColumnsToContents();
  m_view->setModel(m_proxyModel);
  m_view->show();
  
  dockWidgets();
  applyDisplayColumn();
  if(newdb)
    synchroniseWithLocalSongs();
}
//------------------------------------------------------------------------------
void CMainWindow::synchroniseWithLocalSongs()
{
  //Drop table songs and recreate
  QSqlQuery query;
  query.exec("delete from songs");
    
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
  m_fileMenu->addAction(m_openAct);
  m_fileMenu->addAction(m_saveAct);
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
void CMainWindow::dockWidgets()
{
  // Song Info widget
  m_songInfo = new QDockWidget( tr("Current song"), this );
  m_songInfo->setMaximumSize(300, 300);
  QWidget * songInfoWidget = new QWidget();

  m_artistLabel = new QLabel();
  m_titleLabel = new QLabel();
  m_albumLabel = new QLabel();

  QGroupBox* currentSongTagsBox = new QGroupBox;
  QGridLayout *songInfoLayout = new QGridLayout();
  songInfoLayout->addWidget(new QLabel(tr("<b>Song:</b>")),0,0,1,1,Qt::AlignLeft);
  songInfoLayout->addWidget(m_titleLabel,0,1,1,1);
  songInfoLayout->addWidget(new QLabel(tr("<b>Artist:</b>")),1,0,1,1,Qt::AlignLeft);
  songInfoLayout->addWidget(m_artistLabel,1,1,1,1);
  songInfoLayout->addWidget(new QLabel(tr("<b>Album:</b>")),2,0,1,1,Qt::AlignLeft);
  songInfoLayout->addWidget(m_albumLabel,2,1,1,1);
  songInfoLayout->setColumnStretch(2,1);
  songInfoLayout->setRowStretch(3,10);
  currentSongTagsBox->setLayout(songInfoLayout);
  
  QPushButton* editButton = new QPushButton(tr("Edit"));

  m_currentSongWidgetLayout = new QBoxLayout(QBoxLayout::TopToBottom, songInfoWidget);
  m_coverLabel.setAlignment(Qt::AlignTop);
  m_currentSongWidgetLayout->addWidget(&m_coverLabel);
  m_currentSongWidgetLayout->addWidget(currentSongTagsBox);
  m_currentSongWidgetLayout->addWidget(editButton);
  m_currentSongWidgetLayout->addStretch(1);
  m_songInfo->setWidget(songInfoWidget);
  addDockWidget( Qt::LeftDockWidgetArea, m_songInfo );
     
  connect(editButton, SIGNAL(clicked()), SLOT(songEditor()));

  connect(m_songInfo, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
          SLOT(dockWidgetDirectionChanged(Qt::DockWidgetArea)));

  //Data mapper
  QDataWidgetMapper *mapper = new QDataWidgetMapper();
  mapper->setModel(m_proxyModel);
  mapper->addMapping(m_artistLabel, 0, QByteArray("text"));
  mapper->addMapping(m_titleLabel, 1, QByteArray("text"));
  mapper->addMapping(m_albumLabel, 4, QByteArray("text"));
  updateCover(QModelIndex());

  connect(m_view, SIGNAL(clicked(const QModelIndex &)),
          mapper, SLOT(setCurrentModelIndex(const QModelIndex &)));
  connect(m_view, SIGNAL(clicked(const QModelIndex &)),
          SLOT(updateCover(const QModelIndex &)));

  // Debugger Info widget
  m_logInfo = new QDockWidget( tr("Logs"), this );
  m_logInfo->setMinimumWidth(250);
  m_log = new QTextEdit;
  m_log->setReadOnly(true);
  m_logInfo->setWidget(m_log);
  addDockWidget( Qt::BottomDockWidgetArea, m_logInfo );
  m_logInfo->setVisible(false);
}
//------------------------------------------------------------------------------
void CMainWindow::dockWidgetDirectionChanged(Qt::DockWidgetArea area)
{
  if(area==Qt::LeftDockWidgetArea || area==Qt::RightDockWidgetArea)
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
  QString coverpath = m_library->record(m_proxyModel->mapToSource(index).row()).field("cover").value().toString();
  if (QFile::exists(coverpath))
    m_cover->load(coverpath);
  else
    m_cover->load(":/icons/unavailable-large");
  m_coverLabel.setPixmap(*m_cover);
  //truncate labels if too long
  QString string = m_titleLabel->text();
  if(string.size() > 30)
    {
      string.truncate(27);
      m_titleLabel->setText(string+"...");
    }
  string = m_artistLabel->text();
  if(string.size() > 30)
    {
      string.truncate(27);
      m_artistLabel->setText(string+"...");
    }
  string = m_albumLabel->text();
  if(string.size() > 30)
    {
      string.truncate(27);
      m_albumLabel->setText(string+"...");
    }

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

  foreach(index, indexes) {
    selectionModel()->select(index,QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
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
    songsPath << m_library->record(m_proxyModel->mapToSource(index).row()).field("path").value().toString();

  return songsPath;
}
//------------------------------------------------------------------------------
void CMainWindow::build()
{
  QStringList songlist = getSelectedSongs();
  if( songlist.isEmpty() )
    {
      if(QMessageBox::question(this, this->windowTitle(), 
			       QString(tr("You did not select any song. \n Do you want to build the songbook with all songs ?")), 
			       QMessageBox::Yes, 
			       QMessageBox::No, 
			       QMessageBox::NoButton) == QMessageBox::No)
	return;
      else
	{
	  selectAll();
	  songlist = getSelectedSongs();
	}
    }
  clean(); //else songbook indexes are likely to be wrong
  QString filename = QString("%1/mybook.sgl").arg(workingPath());
  
  QString path = QString("%1/").arg(workingPath());
  songlist.replaceInStrings(path, QString());
  CSongbook songbook;
  songbook.setSongs(songlist);
  songbook.save(filename);
  
  applyBookType();
  
  if( m_bookTypeChordbook && m_optionLilypond )
    makeLilypondSheets();
  
  m_buildProcess = new QProcess(this);
  m_buildProcess->setWorkingDirectory(workingPath());
  connect(m_buildProcess, SIGNAL(finished(int,QProcess::ExitStatus)), 
	  this, SLOT(buildFinished(int,QProcess::ExitStatus)));
  connect(m_buildProcess, SIGNAL(error(QProcess::ProcessError)), 
	  this, SLOT(buildError(QProcess::ProcessError)));
  connect(m_buildProcess, SIGNAL(readyReadStandardOutput()), 
	  this, SLOT(readProcessOut()));
  m_log->clear();
  
  QString msg(tr("The songbook generation is now in progress, please wait ..."));
  statusBar()->showMessage(msg);
  progressBar()->show();
  m_buildProcess->start("make", QStringList() << "mybook.pdf");
}
//------------------------------------------------------------------------------
void CMainWindow::readProcessOut()
{
  m_log->append(m_buildProcess->readAllStandardOutput().data());
}
//------------------------------------------------------------------------------
void CMainWindow::buildFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  if(exitStatus == QProcess::NormalExit && exitCode==0)
    {
      m_progressBar->hide();
      QString msg(tr("Songbook successfully generated."));
      statusBar()->showMessage(msg);

      // Visualize produced book: mybook.pdf
      QDesktopServices::openUrl(QUrl(QString("file:///%1/mybook.pdf").arg(m_workingPath)));
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
  if(exitStatus == QProcess::NormalExit && exitCode==0)
    {
      m_progressBar->hide();
      statusBar()->showMessage(tr("Success!"));
    }
}
//------------------------------------------------------------------------------
void CMainWindow::applyBookType()
{
  QString options = packageOptions();

  QFile file(QString("%1/mybook.tex").arg(m_workingPath));

  QString old, fileStr;
  if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {      
      QTextStream stream (&file);
      fileStr = stream.readAll();
      file.close();
      QRegExp rx("documentclass\\[([^\\]]+)");
      rx.indexIn(fileStr);
      old = rx.cap(1);
      fileStr.replace(old, options);
      if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
          QTextStream stream (&file);
          stream << fileStr;
          file.close();
        }
    }
  else
    {
      qWarning() << "Mainwindow::applyBookType warning: unable to open file in read mode";
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
void CMainWindow::makeLilypondSheets()
{
  QModelIndexList indexes = selectionModel()->selectedRows();
  qSort(indexes.begin(), indexes.end());
  QModelIndex index;
  foreach(index, indexes)
    {
      
      if( m_library->record(m_proxyModel->mapToSource(index).row()).field("lilypond").value().toBool() )
	{

	  QProcess *lilyProcess;
	  QStringList sheets;
	  QString path = m_library->record(m_proxyModel->mapToSource(index).row()).field("path").value().toString(); 
	  QString artist = m_library->record(m_proxyModel->mapToSource(index).row()).field("artist").value().toString(); 
	  QString title = m_library->record(m_proxyModel->mapToSource(index).row()).field("title").value().toString(); 
	  QCoreApplication::processEvents();
	  statusBar()->showMessage(QString(tr("Building lilypond for: %1 - %2").arg(artist).arg(title)));

	  QFile file(path);
	  if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	    {
	      QTextStream stream (&file);
	      path = stream.readAll();
	      file.close();
                  
	      QRegExp rx("\\\\lilypond\\{([^}]+)");
	      rx.indexIn(path);
          
	      // process all lilypond files included in the song
	      int pos = 0;
	      while ((pos = rx.indexIn(path, pos)) != -1)
		{
		  lilyProcess = new QProcess;
		  lilyProcess->setWorkingDirectory(QString("%1/lilypond/").arg(workingPath()));
		  lilyProcess->start("lilypond", QStringList() << QString("%1.ly").arg(rx.cap(1)));
                      
		  if (!lilyProcess->waitForFinished()) 
		    delete lilyProcess;
              
		  pos += rx.matchedLength();
		}
	    }
	}
      else
        {
          // str is not present in the current library
        }
    }
}
//------------------------------------------------------------------------------
void CMainWindow::open()
{
  QString filename = QFileDialog::getOpenFileName(this,
                                                  tr("Load a list of songs"),
                                                  workingPath(),
                                                  tr("Songbook File (*.sgl)"));
  CSongbook songbook;
  songbook.load( filename );
  QStringList songlist = songbook.getSongs();
  QString path = QString("%1/").arg(workingPath());
  songlist.replaceInStrings(QRegExp("^"),path);

  m_view->clearSelection();
  
  QList<QModelIndex> indexes;
  QString str;
  foreach(str, songlist)
    {
      indexes = m_library->match( m_proxyModel->index(0,3), Qt::MatchExactly, str );
      if(!indexes.isEmpty())
	selectionModel()->select(indexes[0], QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }
}
//------------------------------------------------------------------------------
void CMainWindow::save()
{
  QString filename = QFileDialog::getSaveFileName(this,
                                                  tr("Save the list of selected songs"),
                                                  workingPath(),
                                                  tr("Songbook File (*.sgl)"));
  QStringList songlist = getSelectedSongs();
  QString path = QString("%1/").arg(workingPath());
  songlist.replaceInStrings(path, QString());
  CSongbook songbook;
  songbook.setSongs(songlist);
  songbook.save(filename);
}
//------------------------------------------------------------------------------
const QString CMainWindow::workingPath()
{
  if( QDir( m_workingPath ).exists() )
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
  if(exist) return false;
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
  CDownloadDialog* dialog = new CDownloadDialog(this);
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
  QString path  = m_library->record(m_proxyModel->mapToSource(selectionModel()->currentIndex()).row()).field("path").value().toString();
  CSongEditor* editor = new CSongEditor(path);
  m_mainWidget->addTab(editor, m_titleLabel->text());
}
//------------------------------------------------------------------------------
void CMainWindow::closeTab(int index)
{
  m_mainWidget->removeTab(index);  
}
