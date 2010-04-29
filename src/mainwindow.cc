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
#include "mainwindow.hh"
#include "preferences.hh"
#include "library.hh"
#include "songbook.hh"
#include "header.hh"
#include "tools.hh"

#include <QtGui>
#include <QtSql>
#include <QtAlgorithms>
#include <iostream>
//******************************************************************************
CMainWindow::CMainWindow()
  : QMainWindow(),
    library(NULL),
    view(NULL),
    selectionModel(NULL),
    proxyModel(NULL),
    m_cover(NULL)
{
  setObjectName("songbook");
  setWindowTitle("Patacrep Songbook Client");
  setWindowIcon(QIcon(":/icons/patacrep.png"));
  readSettings();

  createActions();
  createMenus();

  QString message = tr("A context menu is available by right-clicking");
  statusBar()->showMessage(message);

  m_progressBar = new QProgressBar(statusBar());
  m_progressBar->setTextVisible(false);
  m_progressBar->setRange(0, 0);
  m_progressBar->hide();
  statusBar()->addPermanentWidget(m_progressBar);

  // Create the action button
  QDialogButtonBox * buttonBox = new QDialogButtonBox;

  QPushButton * buttonBuild = new QPushButton(tr("Build PDF"));
  buttonBuild->setDefault(true);
  connect(buttonBuild, SIGNAL(clicked()), this, SLOT(build()));

  QPushButton * buttonQuit = new QPushButton(QIcon(":/icons/application-exit.png"),tr("Quit"));
  connect(buttonQuit, SIGNAL(clicked()), this, SLOT(close()));

  buttonBox->addButton(buttonBuild, QDialogButtonBox::ActionRole);
  buttonBox->addButton(buttonQuit, QDialogButtonBox::DestructiveRole);


  //filtering
  proxyModel = new QSortFilterProxyModel;
  proxyModel->setDynamicSortFilter(true);
  
  m_filterPatternLineEdit = new QLineEdit;
  m_filterPatternLabel = new QLabel(tr("&Filter:"));
  m_filterPatternLabel->setBuddy(m_filterPatternLineEdit);
  
  m_filterSyntaxComboBox = new QComboBox;
  m_filterSyntaxComboBox->addItem(tr("All"), -1);
  m_filterSyntaxComboBox->addItem(tr("Artist"), 0);
  m_filterSyntaxComboBox->addItem(tr("Title"), 1);
  m_filterSyntaxComboBox->addItem(tr("Album"), 4);
  
  
  connect(m_filterPatternLineEdit, SIGNAL(textChanged(QString)),
	  this, SLOT(filterRegExpChanged()));
  connect(m_filterSyntaxComboBox, SIGNAL(currentIndexChanged(int)),
	  this, SLOT(filterRegExpChanged()));

  m_proxyGroupBox = new QGroupBox;
  QHBoxLayout *proxyLayout = new QHBoxLayout;
  proxyLayout->addWidget(m_filterPatternLabel);
  proxyLayout->addWidget(m_filterPatternLineEdit);
  proxyLayout->addWidget(m_filterSyntaxComboBox);
  m_proxyGroupBox->setLayout(proxyLayout);

  // Place the elements into the main window
  QWidget * main = new QWidget;

  view = new QTableView();
  QVBoxLayout * mainLayout = new QVBoxLayout;
  mainLayout->addWidget(view);
  mainLayout->addWidget(m_proxyGroupBox);
  mainLayout->addWidget(buttonBox);
  main->setLayout(mainLayout);

  setCentralWidget(main);

  //Connection to database
  connectDb();

}
//------------------------------------------------------------------------------
void CMainWindow::filterRegExpChanged()
{
  QRegExp regExp(m_filterPatternLineEdit->text(), Qt::CaseInsensitive, QRegExp::FixedString);
  proxyModel->setFilterRegExp(regExp);
  proxyModel->setFilterKeyColumn
    (m_filterSyntaxComboBox->itemData(m_filterSyntaxComboBox->currentIndex()).toInt());
}
//------------------------------------------------------------------------------
CMainWindow::~CMainWindow()
{}
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
  view->setColumnHidden(0,!m_displayColumnArtist);
  view->setColumnHidden(1,!m_displayColumnTitle);
  view->setColumnHidden(3,!m_displayColumnPath);
  view->setColumnHidden(4,!m_displayColumnAlbum);
  view->setColumnHidden(2,!m_displayColumnLilypond);
  view->setColumnHidden(5,!m_displayColumnCover);
  view->resizeColumnsToContents();
}
//------------------------------------------------------------------------------
void CMainWindow::setDisplaySongInfo(bool value)
{
  m_songInfo->setVisible(value);
}
//------------------------------------------------------------------------------
void CMainWindow::setDisplayBookInfo(bool value)
{
  m_bookInfo->setVisible(value);
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
  exitAct = new QAction(tr("Exit"), this);
  exitAct->setShortcut(tr("Ctrl+Q"));
  exitAct->setStatusTip(tr("Exit the application"));
  connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

  openAct = new QAction(QIcon(":/icons/document-load.png"), tr("Load Songs List"), this);
  openAct->setShortcut(tr("Ctrl+O"));
  openAct->setStatusTip(tr("Open a list of songs (.sgl) previously saved."));
  connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

  saveAct = new QAction(QIcon(":/icons/document-save.png"), tr("Save Songs List"), this);
  saveAct->setShortcut(tr("Ctrl+S"));
  saveAct->setStatusTip(tr("Save the list of currently selected songs."));
  connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

  buildAct = new QAction(tr("Build PDF"), this);
  buildAct->setShortcut(tr("Ctrl+B"));
  buildAct->setStatusTip(tr("Generate pdf from selected songs."));
  connect(buildAct, SIGNAL(triggered()), this, SLOT(build()));

  cleanAct = new QAction(QIcon(":/icons/edit-clear.png"), tr("Clean"), this);
  cleanAct->setStatusTip(tr("Clean"));
  connect(cleanAct, SIGNAL(triggered()), this, SLOT(clean()));

  preferencesAct = new QAction(tr("&Preferences"), this);
  preferencesAct->setStatusTip(tr("Select your preferences."));
  connect(preferencesAct, SIGNAL(triggered()), SLOT(preferences()));

  aboutAct = new QAction(tr("&About"), this);
  aboutAct->setStatusTip(tr("Show the application's About box"));
  connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

  selectAllAct = new QAction(tr("Select all"), this);
  selectAllAct->setStatusTip(tr("Select all displayed songs."));
  connect(selectAllAct, SIGNAL(triggered()), SLOT(selectAll()));

  unselectAllAct = new QAction(tr("Unselect all"), this);
  unselectAllAct->setStatusTip(tr("Unselect all displayed songs."));
  connect(unselectAllAct, SIGNAL(triggered()), SLOT(unselectAll()));

  invertSelectionAct = new QAction(tr("Invert Selection"), this);
  invertSelectionAct->setStatusTip(tr("Invert currently selected songs."));
  connect(invertSelectionAct, SIGNAL(triggered()), SLOT(invertSelection()));

  displaySongInfoAct = new QAction(tr("Display Song Info"), this);
  displaySongInfoAct->setStatusTip(tr("Display information about the last selected song."));
  displaySongInfoAct->setCheckable(true);
  displaySongInfoAct->setChecked(true);
  connect(displaySongInfoAct, SIGNAL(toggled(bool)), SLOT(setDisplaySongInfo(bool)));

  displayBookInfoAct = new QAction(tr("Display Book Info"), this);
  displayBookInfoAct->setStatusTip(tr("Display information about the songbook being processed."));
  displayBookInfoAct->setCheckable(true);
  displayBookInfoAct->setChecked(true);
  connect(displayBookInfoAct, SIGNAL(toggled(bool)), SLOT(setDisplayBookInfo(bool)));

  displayLogInfoAct = new QAction(tr("Display Log Info"), this);
  displayLogInfoAct->setStatusTip(tr("Output out the LaTeX compilation process."));
  displayLogInfoAct->setCheckable(true);
  displayLogInfoAct->setChecked(false);
  connect(displayLogInfoAct, SIGNAL(toggled(bool)), SLOT(setDisplayLogInfo(bool)));

  adjustColumnsAct = new QAction(tr("Auto Adjust Columns"), this);
  adjustColumnsAct->setStatusTip(tr("Adjust columns to contents."));
  connect(adjustColumnsAct, SIGNAL(triggered()), SLOT(applyDisplayColumn()));

  connectDbAct = new QAction(QIcon(":/icons/network-server.png"), 
			     tr("Connection to local database"), this);
  connectDbAct->setStatusTip(tr("Connection to local database."));
  connect(connectDbAct, SIGNAL(triggered()), SLOT(connectDb()));

  rebuildDbAct = new QAction(QIcon(":/icons/view-refresh.png"), 
			     tr("Synchronise"), this);
  rebuildDbAct->setStatusTip(tr("Rebuild database from local songs."));
  connect(rebuildDbAct, SIGNAL(triggered()), SLOT(synchroniseWithLocalSongs()));

  CTools* tools = new CTools(workingPath(), this);
  resizeCoversAct = new QAction( tr("Resize covers"), this);
  resizeCoversAct->setStatusTip(tr("Ensure that covers are correctly resized in songbook directory."));
  connect(resizeCoversAct, SIGNAL(triggered()), tools, SLOT(resizeCovers()));

  checkerAct = new QAction( tr("Global check"), this);
  checkerAct->setStatusTip(tr("Check for common mistakes in songs (e.g spelling, chords, LaTeX typo ...)."));
  connect(checkerAct, SIGNAL(triggered()), tools, SLOT(globalCheck()));


}
//------------------------------------------------------------------------------
void CMainWindow::connectDb()
{
  //Connect to database
  bool newdb = createDbConnection();

  // Initialize the song library
  library = new CLibrary();
  library->setPathToSongs(workingPath());

  // Display the song list
  proxyModel->setSourceModel(library);
  view->setModel(library);
  view->setShowGrid( false );
  view->setAlternatingRowColors(true);
  view->setSortingEnabled(true);
  view->setSelectionMode(QAbstractItemView::MultiSelection);
  view->setSelectionBehavior(QAbstractItemView::SelectRows);
  view->setEditTriggers(QAbstractItemView::NoEditTriggers);
  view->sortByColumn(1, Qt::AscendingOrder);
  view->sortByColumn(0, Qt::AscendingOrder);
  view->resizeColumnsToContents();
  view->setModel(proxyModel);
  view->show();
  selectionModel = view->selectionModel();
  
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
  library->setPathToSongs(workingPath());
  library->retrieveSongs();
  view->sortByColumn(1, Qt::AscendingOrder);
  view->sortByColumn(0, Qt::AscendingOrder);
  view->show();
  selectionModel = view->selectionModel();
  applyDisplayColumn();
}
//------------------------------------------------------------------------------
void CMainWindow::contextMenuEvent(QContextMenuEvent *event)
{
  QMenu menu(this);
  menu.addAction(selectAllAct);
  menu.addAction(unselectAllAct);
  menu.addAction(invertSelectionAct);
  menu.exec(event->globalPos());
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
  fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(openAct);
  fileMenu->addAction(saveAct);
  fileMenu->addSeparator();
  fileMenu->addAction(buildAct);
  fileMenu->addAction(rebuildDbAct);
  fileMenu->addAction(cleanAct);
  fileMenu->addSeparator();
  fileMenu->addAction(exitAct);

  editMenu = menuBar()->addMenu(tr("&Edit"));
  editMenu->addAction(selectAllAct);
  editMenu->addAction(unselectAllAct);
  editMenu->addAction(invertSelectionAct);
  editMenu->addSeparator();
  editMenu->addAction(preferencesAct);

  viewMenu = menuBar()->addMenu(tr("&View"));
  viewMenu->addAction(displaySongInfoAct);
  viewMenu->addAction(displayBookInfoAct);
  viewMenu->addAction(displayLogInfoAct);
  viewMenu->addAction(adjustColumnsAct);

  viewMenu = menuBar()->addMenu(tr("&Tools"));
  viewMenu->addAction(resizeCoversAct);
  viewMenu->addAction(checkerAct);

  helpMenu = menuBar()->addMenu(tr("&Help"));
  helpMenu->addAction(aboutAct);
}
//------------------------------------------------------------------------------
void CMainWindow::dockWidgets()
{

  //Book Info widget
  m_bookInfo = new QDockWidget( tr("My songbook"), this );
  m_bookInfo->setMinimumWidth(250);
  m_bookInfo->setMinimumHeight(300);
  m_bookInfo->setMaximumHeight(300);
  QWidget * bookInfoWidget = new QWidget();

  QLabel* ltitle     = new QLabel(tr("Title:"));
  QLabel* lsubtitle  = new QLabel(tr("Subtitle:"));
  QLabel* lauthor    = new QLabel(tr("Author:"));
  QLabel* lversion   = new QLabel(tr("Version:"));
  QLabel* lmail      = new QLabel(tr("Mail:"));
  QLabel* lpicture   = new QLabel(tr("Picture:"));
  QLabel* lcopyright = new QLabel(tr("Copyright:"));

  //Todo: use CHeader getters to initialize except for picture
  //since we want the path and not the basename that is in .tex file
  m_title = new QLineEdit("Patacrep Songbook");
  m_subtitle = new QLineEdit("Tome 1");
  m_author = new QLineEdit("Crep, Lohrun");
  m_version = new QLineEdit("0.1");
  m_mail = new QLineEdit("crep@team-on-fire.com");
  m_picture =new QLineEdit(QString("%1/img/feel-the-music.jpg").arg(workingPath()));
  m_copyright = new QLineEdit("©deviantart");
  m_picture->setReadOnly(true);

  QToolButton *browsePictureButton = new QToolButton;
  browsePictureButton->setIcon(QIcon(":/icons/document-load.png"));
  QPushButton *apply = new QPushButton(tr("Apply"));
  
  QGridLayout *bookInfoLayout = new QGridLayout();
  bookInfoLayout->addWidget(ltitle,0,0,1,1);
  bookInfoLayout->addWidget(m_title,0,1,1,3);
  bookInfoLayout->addWidget(lsubtitle,1,0,1,1);
  bookInfoLayout->addWidget(m_subtitle,1,1,1,3);
  bookInfoLayout->addWidget(lauthor,2,0,1,1);
  bookInfoLayout->addWidget(m_author,2,1,1,3);
  bookInfoLayout->addWidget(lversion,3,0,1,1);
  bookInfoLayout->addWidget(m_version,3,1,1,3);
  bookInfoLayout->addWidget(lmail,4,0,1,1);
  bookInfoLayout->addWidget(m_mail,4,1,1,3);
  bookInfoLayout->addWidget(lpicture,5,0,1,1);
  bookInfoLayout->addWidget(m_picture,5,1,1,2);
  bookInfoLayout->addWidget(browsePictureButton,5,3,1,1);
  bookInfoLayout->addWidget(lcopyright,6,0,1,1);
  bookInfoLayout->addWidget(m_copyright,6,1,1,3);
  bookInfoLayout->addWidget(apply,7,2,1,2);

  connect(apply, SIGNAL(clicked()),
	  this, SLOT(updateHeader()) );
  connect(browsePictureButton, SIGNAL(clicked()),
	  this, SLOT(browseHeaderPicture()) );

  bookInfoWidget->setLayout(bookInfoLayout);
  m_bookInfo->setWidget(bookInfoWidget);
  addDockWidget( Qt::LeftDockWidgetArea, m_bookInfo );

  // Song Info widget
  m_songInfo = new QDockWidget( tr("Current song"), this );
  m_songInfo->setMinimumWidth(200);
  m_songInfo->setMaximumHeight(250);
  QWidget * songInfoWidget = new QWidget();

  QLabel* lartist  = new QLabel(tr("Artist:"));
  QLabel* lsong    = new QLabel(tr("Song:"));
  QLabel* lalbum   = new QLabel(tr("Album:"));
  QLabel *artistLabel = new QLabel();
  QLabel *titleLabel = new QLabel();
  QLabel *albumLabel = new QLabel();

  QGridLayout *songInfoLayout = new QGridLayout();
  songInfoLayout->addWidget(&m_coverLabel,0,0,1,2);
  songInfoLayout->addWidget(lartist,1,0,1,1);
  songInfoLayout->addWidget(artistLabel,1,1,1,1);
  songInfoLayout->addWidget(lsong,2,0,1,1);
  songInfoLayout->addWidget(titleLabel,2,1,1,1);
  songInfoLayout->addWidget(lalbum,3,0,1,1);
  songInfoLayout->addWidget(albumLabel,3,1,1,1);

  songInfoWidget->setLayout(songInfoLayout);
  m_songInfo->setWidget(songInfoWidget);
  addDockWidget( Qt::LeftDockWidgetArea, m_songInfo );


  //Data mapper
  QDataWidgetMapper *mapper = new QDataWidgetMapper();
  mapper->setModel(proxyModel);
  mapper->addMapping(artistLabel, 0, QByteArray("text"));
  mapper->addMapping(titleLabel, 1, QByteArray("text"));
  mapper->addMapping(albumLabel, 4, QByteArray("text"));
  mapper->toFirst();

  connect(view, SIGNAL(clicked(const QModelIndex &)),
          mapper, SLOT(setCurrentModelIndex(const QModelIndex &)));
  connect(view, SIGNAL(clicked(const QModelIndex &)),
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
void CMainWindow::updateCover(const QModelIndex & index)
{
  if(m_cover) delete m_cover;
  m_cover = new QPixmap( library->record(proxyModel->mapToSource(index).row()).field("cover").value().toString() );
  m_coverLabel.setPixmap(*m_cover);
}
//------------------------------------------------------------------------------
void CMainWindow::updateHeader()
{
  CHeader header(workingPath());
  header.setTitle(m_title->text());
  header.setSubtitle(m_subtitle->text());
  header.setAuthor(m_author->text());
  header.setVersion(m_version->text());
  header.setMail(m_mail->text());
  header.setPicture(m_picture->text());
  header.setCopyright(m_copyright->text());
}
//------------------------------------------------------------------------------
void CMainWindow::browseHeaderPicture()
{
  QString filename = QFileDialog::getOpenFileName(this, tr("Open Image File"),
						  "/home",
						  tr("Images (*.png *.jpg)"));
  if (!filename.isEmpty())
    m_picture->setText(filename);
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
  view->selectAll();
}
//------------------------------------------------------------------------------
void CMainWindow::unselectAll()
{
  view->clearSelection();
}
//------------------------------------------------------------------------------
void CMainWindow::invertSelection()
{
  QModelIndexList indexes = selectionModel->selectedRows();
  QModelIndex index;

  view->selectAll();

  foreach(index, indexes) {
    selectionModel->select(index,QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
  }
}
//------------------------------------------------------------------------------
QStringList CMainWindow::getSelectedSongs()
{
  QStringList songsPath;
  QModelIndexList indexes = selectionModel->selectedRows();
  QModelIndex index;
  
  qSort(indexes.begin(), indexes.end());

  foreach(index, indexes)
    songsPath << library->record(proxyModel->mapToSource(index).row()).field("path").value().toString();

  return songsPath;
}
//------------------------------------------------------------------------------
void CMainWindow::build()
{
  clean(); //else songbook indexes are likely to be wrong
  QString filename = QString("%1/mybook.sgl").arg(workingPath());
  QStringList songlist = getSelectedSongs();
  if(songlist.isEmpty()) return;

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
  m_progressBar->show();
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
      std::cerr << "Mainwindow::applyBookType warning: unable to open file in read mode" << std::endl;
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
  QModelIndexList indexes = selectionModel->selectedRows();
  qSort(indexes.begin(), indexes.end());
  QModelIndex index;
  foreach(index, indexes)
    {
      
      if( library->record(proxyModel->mapToSource(index).row()).field("lilypond").value().toBool() )
	{

	  QProcess *lilyProcess;
	  QStringList sheets;
	  QString path = library->record(proxyModel->mapToSource(index).row()).field("path").value().toString(); 
	  QString artist = library->record(proxyModel->mapToSource(index).row()).field("artist").value().toString(); 
	  QString title = library->record(proxyModel->mapToSource(index).row()).field("title").value().toString(); 
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

  view->clearSelection();
  
  QList<QModelIndex> indexes;
  QString str;
  foreach(str, songlist)
    {
      indexes = library->match( proxyModel->index(0,3), Qt::MatchExactly, str );
      if(!indexes.isEmpty())
	selectionModel->select(indexes[0], QItemSelectionModel::Select | QItemSelectionModel::Rows);
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

  bool exist = (QFile::exists(dbpath))?true:false;
  
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
